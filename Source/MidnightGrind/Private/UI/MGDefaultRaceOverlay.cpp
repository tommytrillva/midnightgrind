// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGDefaultRaceOverlay.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

UMGDefaultRaceOverlay::UMGDefaultRaceOverlay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGDefaultRaceOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	CreateUIElements();
}

void UMGDefaultRaceOverlay::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMGDefaultRaceOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateCountdownAnimation(InDeltaTime);
	UpdateNotificationAnimations(InDeltaTime);
	UpdateWrongWayAnimation(InDeltaTime);
	UpdateFinishAnimation(InDeltaTime);
}

FReply UMGDefaultRaceOverlay::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ==========================================
// UI CREATION
// ==========================================

void UMGDefaultRaceOverlay::CreateUIElements()
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

	CreateCountdownDisplay();
	CreateNotificationArea();
	CreateWrongWayDisplay();
	CreateFinishDisplay();
}

void UMGDefaultRaceOverlay::CreateCountdownDisplay()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Countdown panel - centered
	CountdownPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CountdownPanel"));
	if (CountdownPanel)
	{
		RootCanvas->AddChild(CountdownPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(CountdownPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.5f, 0.4f, 0.5f, 0.4f));
			Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			Slot->SetAutoSize(true);
		}
		CountdownPanel->SetVisibility(ESlateVisibility::Collapsed);

		// Main countdown number
		CountdownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownText"));
		if (CountdownText)
		{
			CountdownPanel->AddChild(CountdownText);
			if (UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(CountdownText->Slot))
			{
				TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				TextSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = CountdownText->GetFont();
			FontInfo.Size = CountdownFontSize;
			CountdownText->SetFont(FontInfo);
			CountdownText->SetColorAndOpacity(FSlateColor(CyanNeon));
			CountdownText->SetJustification(ETextJustify::Center);
			CountdownText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
			CountdownText->SetShadowOffset(FVector2D(4.0f, 4.0f));
		}

		// Sub text ("GET READY")
		CountdownSubText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownSubText"));
		if (CountdownSubText)
		{
			CountdownPanel->AddChild(CountdownSubText);
			if (UCanvasPanelSlot* SubSlot = Cast<UCanvasPanelSlot>(CountdownSubText->Slot))
			{
				SubSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				SubSlot->SetAlignment(FVector2D(0.5f, 0.0f));
				SubSlot->SetPosition(FVector2D(0.0f, 120.0f));
				SubSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = CountdownSubText->GetFont();
			FontInfo.Size = CountdownSubFontSize;
			CountdownSubText->SetFont(FontInfo);
			CountdownSubText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			CountdownSubText->SetJustification(ETextJustify::Center);
			CountdownSubText->SetText(FText::FromString(TEXT("GET READY")));
		}
	}
}

void UMGDefaultRaceOverlay::CreateNotificationArea()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Notification container - right side
	NotificationContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NotificationContainer"));
	if (NotificationContainer)
	{
		RootCanvas->AddChild(NotificationContainer);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(NotificationContainer->Slot))
		{
			Slot->SetAnchors(FAnchors(1.0f, 0.3f, 1.0f, 0.3f));
			Slot->SetAlignment(FVector2D(1.0f, 0.0f));
			Slot->SetPosition(FVector2D(-40.0f, 0.0f));
			Slot->SetAutoSize(true);
		}
	}
}

void UMGDefaultRaceOverlay::CreateWrongWayDisplay()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Wrong way panel - top center
	WrongWayPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("WrongWayPanel"));
	if (WrongWayPanel)
	{
		RootCanvas->AddChild(WrongWayPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(WrongWayPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.5f, 0.15f, 0.5f, 0.15f));
			Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			Slot->SetAutoSize(true);
		}
		WrongWayPanel->SetVisibility(ESlateVisibility::Collapsed);

		// Border background
		WrongWayBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WrongWayBorder"));
		if (WrongWayBorder)
		{
			WrongWayPanel->AddChild(WrongWayBorder);
			if (UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(WrongWayBorder->Slot))
			{
				BorderSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				BorderSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				BorderSlot->SetAutoSize(true);
			}
			WrongWayBorder->SetBrushColor(FLinearColor(0.2f, 0.0f, 0.0f, 0.8f));
			WrongWayBorder->SetPadding(FMargin(40.0f, 20.0f));
		}

		// Wrong way text
		WrongWayText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WrongWayText"));
		if (WrongWayText && WrongWayBorder)
		{
			WrongWayBorder->AddChild(WrongWayText);

			FSlateFontInfo FontInfo = WrongWayText->GetFont();
			FontInfo.Size = WrongWayFontSize;
			WrongWayText->SetFont(FontInfo);
			WrongWayText->SetColorAndOpacity(FSlateColor(RedNeon));
			WrongWayText->SetJustification(ETextJustify::Center);
			WrongWayText->SetText(FText::FromString(TEXT("WRONG WAY")));
			WrongWayText->SetShadowColorAndOpacity(FLinearColor::Black);
			WrongWayText->SetShadowOffset(FVector2D(3.0f, 3.0f));
		}
	}
}

void UMGDefaultRaceOverlay::CreateFinishDisplay()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Finish panel - centered
	FinishPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("FinishPanel"));
	if (FinishPanel)
	{
		RootCanvas->AddChild(FinishPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FinishPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			Slot->SetAutoSize(true);
		}
		FinishPanel->SetVisibility(ESlateVisibility::Collapsed);

		// Background
		FinishBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FinishBackground"));
		if (FinishBackground)
		{
			FinishPanel->AddChild(FinishBackground);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(FinishBackground->Slot))
			{
				BgSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				BgSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				BgSlot->SetSize(FVector2D(600.0f, 350.0f));
			}
			FinishBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
			FinishBackground->SetPadding(FMargin(60.0f, 40.0f));
		}

		// "FINISH!" text
		FinishMainText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FinishMainText"));
		if (FinishMainText)
		{
			FinishPanel->AddChild(FinishMainText);
			if (UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(FinishMainText->Slot))
			{
				TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				TextSlot->SetPosition(FVector2D(0.0f, -100.0f));
				TextSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = FinishMainText->GetFont();
			FontInfo.Size = FinishMainFontSize;
			FinishMainText->SetFont(FontInfo);
			FinishMainText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			FinishMainText->SetJustification(ETextJustify::Center);
			FinishMainText->SetText(FText::FromString(TEXT("FINISH!")));
			FinishMainText->SetShadowColorAndOpacity(FLinearColor::Black);
			FinishMainText->SetShadowOffset(FVector2D(3.0f, 3.0f));
		}

		// Position text (1st, 2nd, etc)
		FinishPositionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FinishPositionText"));
		if (FinishPositionText)
		{
			FinishPanel->AddChild(FinishPositionText);
			if (UCanvasPanelSlot* PosSlot = Cast<UCanvasPanelSlot>(FinishPositionText->Slot))
			{
				PosSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				PosSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				PosSlot->SetPosition(FVector2D(0.0f, 0.0f));
				PosSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = FinishPositionText->GetFont();
			FontInfo.Size = FinishPositionFontSize;
			FinishPositionText->SetFont(FontInfo);
			FinishPositionText->SetColorAndOpacity(FSlateColor(GoldColor));
			FinishPositionText->SetJustification(ETextJustify::Center);
			FinishPositionText->SetShadowColorAndOpacity(FLinearColor::Black);
			FinishPositionText->SetShadowOffset(FVector2D(4.0f, 4.0f));
		}

		// Time text
		FinishTimeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FinishTimeText"));
		if (FinishTimeText)
		{
			FinishPanel->AddChild(FinishTimeText);
			if (UCanvasPanelSlot* TimeSlot = Cast<UCanvasPanelSlot>(FinishTimeText->Slot))
			{
				TimeSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				TimeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				TimeSlot->SetPosition(FVector2D(0.0f, 90.0f));
				TimeSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = FinishTimeText->GetFont();
			FontInfo.Size = FinishTimeFontSize;
			FinishTimeText->SetFont(FontInfo);
			FinishTimeText->SetColorAndOpacity(FSlateColor(CyanNeon));
			FinishTimeText->SetJustification(ETextJustify::Center);
		}

		// New record text
		FinishRecordText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FinishRecordText"));
		if (FinishRecordText)
		{
			FinishPanel->AddChild(FinishRecordText);
			if (UCanvasPanelSlot* RecordSlot = Cast<UCanvasPanelSlot>(FinishRecordText->Slot))
			{
				RecordSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				RecordSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				RecordSlot->SetPosition(FVector2D(0.0f, 140.0f));
				RecordSlot->SetAutoSize(true);
			}

			FSlateFontInfo FontInfo = FinishRecordText->GetFont();
			FontInfo.Size = 32.0f;
			FinishRecordText->SetFont(FontInfo);
			FinishRecordText->SetColorAndOpacity(FSlateColor(PinkNeon));
			FinishRecordText->SetJustification(ETextJustify::Center);
			FinishRecordText->SetText(FText::FromString(TEXT("NEW RECORD!")));
			FinishRecordText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FMGNotificationDisplayEntry UMGDefaultRaceOverlay::CreateNotificationEntry(const FMGNotificationData& Data)
{
	FMGNotificationDisplayEntry Entry;
	Entry.NotificationID = Data.NotificationID;
	Entry.SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Entry.Duration = Data.Duration;

	if (!NotificationContainer || !WidgetTree)
	{
		return Entry;
	}

	// Create panel for this notification
	Entry.Panel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	if (Entry.Panel)
	{
		NotificationContainer->AddChild(Entry.Panel);
		if (UVerticalBoxSlot* Slot = Cast<UVerticalBoxSlot>(Entry.Panel->Slot))
		{
			Slot->SetHorizontalAlignment(HAlign_Right);
			Slot->SetPadding(FMargin(0.0f, 5.0f));
		}

		// Background border
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (Background)
		{
			Entry.Panel->AddChild(Background);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Background->Slot))
			{
				BgSlot->SetAutoSize(true);
			}
			Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
			Background->SetPadding(FMargin(20.0f, 10.0f));

			// Create vertical box for text
			UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
			if (TextBox)
			{
				Background->AddChild(TextBox);

				// Main text
				Entry.MainText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
				if (Entry.MainText)
				{
					TextBox->AddChild(Entry.MainText);
					if (UVerticalBoxSlot* TextSlot = Cast<UVerticalBoxSlot>(Entry.MainText->Slot))
					{
						TextSlot->SetHorizontalAlignment(HAlign_Right);
					}

					FSlateFontInfo FontInfo = Entry.MainText->GetFont();
					FontInfo.Size = NotificationFontSize;
					Entry.MainText->SetFont(FontInfo);
					Entry.MainText->SetColorAndOpacity(FSlateColor(Data.Color));
					Entry.MainText->SetJustification(ETextJustify::Right);
					Entry.MainText->SetText(Data.MainText);
					Entry.MainText->SetShadowColorAndOpacity(FLinearColor::Black);
					Entry.MainText->SetShadowOffset(FVector2D(2.0f, 2.0f));
				}

				// Sub text (if any)
				if (!Data.SubText.IsEmpty())
				{
					Entry.SubText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
					if (Entry.SubText)
					{
						TextBox->AddChild(Entry.SubText);
						if (UVerticalBoxSlot* SubSlot = Cast<UVerticalBoxSlot>(Entry.SubText->Slot))
						{
							SubSlot->SetHorizontalAlignment(HAlign_Right);
						}

						FSlateFontInfo SubFontInfo = Entry.SubText->GetFont();
						SubFontInfo.Size = NotificationSubFontSize;
						Entry.SubText->SetFont(SubFontInfo);
						Entry.SubText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
						Entry.SubText->SetJustification(ETextJustify::Right);
						Entry.SubText->SetText(Data.SubText);
					}
				}
			}
		}
	}

	return Entry;
}

// ==========================================
// ANIMATION UPDATES
// ==========================================

void UMGDefaultRaceOverlay::UpdateCountdownAnimation(float MGDeltaTime)
{
	if (!bCountdownAnimating || !CountdownText)
	{
		return;
	}

	CountdownAnimTime += DeltaTime;

	// Scale animation - pop in and settle
	float AnimProgress = FMath::Clamp(CountdownAnimTime / 0.3f, 0.0f, 1.0f);
	float ScaleCurve = 1.0f + (CountdownScaleTarget - 1.0f) * FMath::Sin(AnimProgress * PI * 0.5f);

	// Apply scale via render transform
	CountdownText->SetRenderScale(FVector2D(ScaleCurve, ScaleCurve));

	// Pulse effect
	float PulseAlpha = FMath::Clamp((CountdownAnimTime - 0.3f) / 0.7f, 0.0f, 1.0f);
	if (PulseAlpha > 0.0f)
	{
		float Pulse = GetPulseScale(CountdownAnimTime, 3.0f, 0.05f);
		CountdownText->SetRenderScale(FVector2D(1.0f + Pulse, 1.0f + Pulse));
	}
}

void UMGDefaultRaceOverlay::UpdateNotificationAnimations(float MGDeltaTime)
{
	if (!GetWorld())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (FMGNotificationDisplayEntry& Entry : NotificationEntries)
	{
		if (!Entry.Panel)
		{
			continue;
		}

		float Age = CurrentTime - Entry.SpawnTime;
		float NormalizedAge = (Entry.Duration > 0.0f) ? (Age / Entry.Duration) : 1.0f;

		// Fade in/out
		float Alpha = 1.0f;
		const float FadeInDuration = 0.2f;
		const float FadeOutStart = 0.7f;

		if (Age < FadeInDuration)
		{
			Alpha = Age / FadeInDuration;
		}
		else if (NormalizedAge > FadeOutStart)
		{
			Alpha = 1.0f - ((NormalizedAge - FadeOutStart) / (1.0f - FadeOutStart));
		}

		Entry.Panel->SetRenderOpacity(Alpha);

		// Slide in from right
		if (Age < 0.3f)
		{
			float SlideProgress = Age / 0.3f;
			float EasedProgress = 1.0f - FMath::Pow(1.0f - SlideProgress, 3.0f); // Ease out cubic
			float OffsetX = FMath::Lerp(100.0f, 0.0f, EasedProgress);
			Entry.Panel->SetRenderTranslation(FVector2D(OffsetX, 0.0f));
		}
	}
}

void UMGDefaultRaceOverlay::UpdateWrongWayAnimation(float MGDeltaTime)
{
	if (!bWrongWayVisible || !WrongWayText)
	{
		return;
	}

	WrongWayAnimTime += DeltaTime;

	// Flash effect - 2Hz
	float FlashPeriod = 0.5f;
	bool bFlashOn = FMath::Fmod(WrongWayAnimTime, FlashPeriod) < (FlashPeriod * 0.5f);

	float Alpha = bFlashOn ? 1.0f : 0.6f;
	WrongWayText->SetRenderOpacity(Alpha);

	// Pulsing scale
	float Pulse = GetPulseScale(WrongWayAnimTime, 4.0f, 0.08f);
	WrongWayText->SetRenderScale(FVector2D(1.0f + Pulse, 1.0f + Pulse));
}

void UMGDefaultRaceOverlay::UpdateFinishAnimation(float MGDeltaTime)
{
	if (!bFinishVisible || !FinishPanel)
	{
		return;
	}

	FinishAnimTime += DeltaTime;

	// Reveal animation
	float RevealProgress = FMath::Clamp(FinishAnimTime / 0.5f, 0.0f, 1.0f);
	float EasedReveal = 1.0f - FMath::Pow(1.0f - RevealProgress, 3.0f);

	// Scale up from small
	float Scale = FMath::Lerp(0.5f, 1.0f, EasedReveal);
	FinishPanel->SetRenderScale(FVector2D(Scale, Scale));
	FinishPanel->SetRenderOpacity(EasedReveal);

	// Position text pulse for emphasis
	if (FinishPositionText && FinishAnimTime > 0.5f)
	{
		float Pulse = GetPulseScale(FinishAnimTime - 0.5f, 1.5f, 0.03f);
		FinishPositionText->SetRenderScale(FVector2D(1.0f + Pulse, 1.0f + Pulse));
	}

	// Record text flash if applicable
	if (bFinishNewRecord && FinishRecordText && FinishAnimTime > 1.0f)
	{
		float FlashTime = FinishAnimTime - 1.0f;
		float Flash = 0.7f + 0.3f * FMath::Sin(FlashTime * 6.0f);
		FinishRecordText->SetRenderOpacity(Flash);
	}
}

float UMGDefaultRaceOverlay::GetPulseScale(float Time, float Frequency, float Amplitude) const
{
	return FMath::Sin(Time * Frequency * 2.0f * PI) * Amplitude;
}

// ==========================================
// HELPERS
// ==========================================

void UMGDefaultRaceOverlay::SetCountdownTextWithAnimation(const FText& Text, FLinearColor Color)
{
	if (CountdownText)
	{
		CountdownText->SetText(Text);
		CountdownText->SetColorAndOpacity(FSlateColor(Color));
		CountdownText->SetRenderScale(FVector2D(1.5f, 1.5f)); // Start large
	}

	CountdownAnimTime = 0.0f;
	bCountdownAnimating = true;
	CountdownScaleTarget = 1.5f;
}

void UMGDefaultRaceOverlay::LayoutNotificationEntries()
{
	// Notifications are automatically laid out by VerticalBox
	// This could be used for custom positioning if needed
}

FSlateColor UMGDefaultRaceOverlay::GetPositionDisplayColor(int32 Position) const
{
	switch (Position)
	{
	case 1:
		return FSlateColor(GoldColor);
	case 2:
		return FSlateColor(SilverColor);
	case 3:
		return FSlateColor(BronzeColor);
	default:
		return FSlateColor(FLinearColor::White);
	}
}

FText UMGDefaultRaceOverlay::FormatPosition(int32 Position) const
{
	FText Suffix;
	if (Position >= 11 && Position <= 13)
	{
		Suffix = FText::FromString(TEXT("TH"));
	}
	else
	{
		switch (Position % 10)
		{
		case 1:
			Suffix = FText::FromString(TEXT("ST"));
			break;
		case 2:
			Suffix = FText::FromString(TEXT("ND"));
			break;
		case 3:
			Suffix = FText::FromString(TEXT("RD"));
			break;
		default:
			Suffix = FText::FromString(TEXT("TH"));
			break;
		}
	}

	return FText::Format(FText::FromString(TEXT("{0}{1}")), FText::AsNumber(Position), Suffix);
}

// ==========================================
// IMPLEMENTATION OVERRIDES
// ==========================================

void UMGDefaultRaceOverlay::OnCountdownValueChanged_Implementation(int32 NewValue)
{
	if (!CountdownPanel)
	{
		return;
	}

	CountdownPanel->SetVisibility(ESlateVisibility::HitTestInvisible);

	FText DisplayText = FText::AsNumber(NewValue);
	FLinearColor DisplayColor = CyanNeon;

	// Color progression: 3=cyan, 2=yellow, 1=pink
	if (NewValue == 3)
	{
		DisplayColor = CyanNeon;
	}
	else if (NewValue == 2)
	{
		DisplayColor = YellowNeon;
	}
	else if (NewValue == 1)
	{
		DisplayColor = PinkNeon;
	}

	SetCountdownTextWithAnimation(DisplayText, DisplayColor);

	if (CountdownSubText)
	{
		CountdownSubText->SetText(FText::FromString(TEXT("GET READY")));
	}
}

void UMGDefaultRaceOverlay::OnCountdownGo_Implementation()
{
	if (!CountdownPanel)
	{
		return;
	}

	SetCountdownTextWithAnimation(FText::FromString(TEXT("GO!")), GreenNeon);

	if (CountdownSubText)
	{
		CountdownSubText->SetText(FText::GetEmpty());
	}

	// Hide countdown after a delay
	if (UWorld* World = GetWorld())
	{
		FTimerHandle HideHandle;
		TWeakObjectPtr<UMGDefaultRaceOverlay> WeakThis(this);
		World->GetTimerManager().SetTimer(HideHandle, [WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			if (WeakThis->CountdownPanel)
			{
				WeakThis->CountdownPanel->SetVisibility(ESlateVisibility::Collapsed);
				WeakThis->bCountdownAnimating = false;
			}
		}, 1.0f, false);
	}
}

void UMGDefaultRaceOverlay::DisplayNotification_Implementation(const FMGNotificationData& Data)
{
	FMGNotificationDisplayEntry Entry = CreateNotificationEntry(Data);
	if (Entry.Panel)
	{
		NotificationEntries.Add(Entry);
	}
}

void UMGDefaultRaceOverlay::RemoveNotification_Implementation(int32 NotificationID)
{
	int32 Index = NotificationEntries.IndexOfByPredicate([NotificationID](const FMGNotificationDisplayEntry& Entry)
	{
		return Entry.NotificationID == NotificationID;
	});

	if (Index != INDEX_NONE)
	{
		if (NotificationEntries[Index].Panel)
		{
			NotificationEntries[Index].Panel->RemoveFromParent();
		}
		NotificationEntries.RemoveAt(Index);
	}
}

void UMGDefaultRaceOverlay::UpdateWrongWayDisplay_Implementation(bool bShow)
{
	bWrongWayVisible = bShow;
	WrongWayAnimTime = 0.0f;

	if (WrongWayPanel)
	{
		WrongWayPanel->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UMGDefaultRaceOverlay::DisplayRaceFinish_Implementation(int32 Position, float Time, bool bNewRecord)
{
	FinishPosition = Position;
	bFinishNewRecord = bNewRecord;
	bFinishVisible = true;
	FinishAnimTime = 0.0f;

	if (FinishPanel)
	{
		FinishPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// Update position text
	if (FinishPositionText)
	{
		FinishPositionText->SetText(FormatPosition(Position));
		FinishPositionText->SetColorAndOpacity(GetPositionDisplayColor(Position));
	}

	// Update time text
	if (FinishTimeText)
	{
		FinishTimeText->SetText(FormatTime(Time));
	}

	// Show/hide new record
	if (FinishRecordText)
	{
		FinishRecordText->SetVisibility(bNewRecord ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	// Update main text based on position
	if (FinishMainText)
	{
		if (Position == 1)
		{
			FinishMainText->SetText(FText::FromString(TEXT("VICTORY!")));
			FinishMainText->SetColorAndOpacity(FSlateColor(GoldColor));
		}
		else if (Position <= 3)
		{
			FinishMainText->SetText(FText::FromString(TEXT("PODIUM!")));
			FinishMainText->SetColorAndOpacity(FSlateColor(CyanNeon));
		}
		else
		{
			FinishMainText->SetText(FText::FromString(TEXT("FINISH!")));
			FinishMainText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
}
