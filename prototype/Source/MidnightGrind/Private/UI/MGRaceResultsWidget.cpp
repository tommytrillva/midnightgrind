// Copyright Midnight Grind. All Rights Reserved.
// Updated Stage 51: Race Flow Integration

#include "UI/MGRaceResultsWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#include "TimerManager.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Race/MGRaceHistorySubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGRaceResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateUIElements();

	SetVisibility(ESlateVisibility::Collapsed);
}

FReply UMGRaceResultsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	// Accept/Continue
	if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		HandleContinue();
		return FReply::Handled();
	}

	// Restart
	if (Key == EKeys::R || Key == EKeys::Gamepad_FaceButton_Left)
	{
		HandleRestart();
		return FReply::Handled();
	}

	// Quit
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		HandleQuit();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMGRaceResultsWidget::DisplayResults(const FMGRaceResults& Results)
{
	CachedResults = Results;
	ProcessResultsData(Results);

	// Update header based on player result
	if (HeaderText)
	{
		if (Results.bPlayerWon)
		{
			HeaderText->SetText(FText::FromString(TEXT("VICTORY!")));
			HeaderText->SetColorAndOpacity(WinnerColor);
		}
		else
		{
			int32 PlayerPos = 0;
			for (const FMGRacerData& Racer : Results.RacerResults)
			{
				if (!Racer.bIsAI)
				{
					PlayerPos = Racer.Position;
					break;
				}
			}
			HeaderText->SetText(FText::FromString(FString::Printf(TEXT("FINISHED P%d"), PlayerPos)));
			HeaderText->SetColorAndOpacity(PlayerHighlightColor);
		}
	}

	// Update sub header
	if (SubHeaderText)
	{
		FString RaceType;
		switch (Results.Config.RaceType)
		{
			case EMGRaceType::Circuit: RaceType = TEXT("CIRCUIT RACE"); break;
			case EMGRaceType::Sprint: RaceType = TEXT("SPRINT RACE"); break;
			case EMGRaceType::Drift: RaceType = TEXT("DRIFT BATTLE"); break;
			case EMGRaceType::Drag: RaceType = TEXT("DRAG RACE"); break;
			case EMGRaceType::TimeTrial: RaceType = TEXT("TIME TRIAL"); break;
			case EMGRaceType::PinkSlip: RaceType = TEXT("PINK SLIP RACE"); break;
			default: RaceType = TEXT("RACE COMPLETE"); break;
		}
		SubHeaderText->SetText(FText::FromString(RaceType));
	}

	// Update rewards
	if (CreditsText)
	{
		CreditsText->SetText(GetCreditsEarnedText());
	}

	if (ReputationText)
	{
		ReputationText->SetText(GetReputationEarnedText());
	}

	// Update best lap
	if (BestLapText && Results.BestLapTime > 0.0f)
	{
		FString BestLapStr = FString::Printf(TEXT("FASTEST LAP: %s"), *FormatTime(Results.BestLapTime).ToString());
		BestLapText->SetText(FText::FromString(BestLapStr));
	}

	// Clear and rebuild results list
	if (ResultsListBox)
	{
		ResultsListBox->ClearChildren();

		for (const FMGResultRowData& Row : ResultRows)
		{
			UWidget* RowWidget = CreateResultRow(Row);
			if (RowWidget)
			{
				ResultsListBox->AddChild(RowWidget);
			}
		}
	}

	// Update prompt
	if (PromptText)
	{
		PromptText->SetText(FText::FromString(TEXT("[ENTER] Continue    [R] Restart    [ESC] Quit")));
	}

	// Update history stats display
	float PlayerTime = 0.0f;
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			PlayerTime = Racer.TotalTime;
			break;
		}
	}
	UpdateHistoryStatsDisplay(Results.Config.TrackLayoutID, PlayerTime);

	OnResultsReady();

	// Play appropriate animation
	if (Results.bPlayerWon)
	{
		PlayVictoryAnimation();
	}
	else
	{
		PlayDefeatAnimation();
	}
}

void UMGRaceResultsWidget::ShowResults()
{
	SetVisibility(ESlateVisibility::Visible);
	SetKeyboardFocus();

	// Start row reveal animation
	CurrentRevealRow = 0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RowRevealTimerHandle, this, &UMGRaceResultsWidget::RevealNextRow, 0.2f, true);
	}
}

void UMGRaceResultsWidget::HideResults()
{
	SetVisibility(ESlateVisibility::Collapsed);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RowRevealTimerHandle);
	}
}

FText UMGRaceResultsWidget::GetCreditsEarnedText() const
{
	if (CachedResults.CreditsEarned > 0)
	{
		return FText::FromString(FString::Printf(TEXT("+$%lld CREDITS"), CachedResults.CreditsEarned));
	}
	return FText::FromString(TEXT("$0 CREDITS"));
}

FText UMGRaceResultsWidget::GetReputationEarnedText() const
{
	if (CachedResults.ReputationEarned > 0)
	{
		return FText::FromString(FString::Printf(TEXT("+%d REP"), CachedResults.ReputationEarned));
	}
	return FText::FromString(TEXT("+0 REP"));
}

FText UMGRaceResultsWidget::GetXPEarnedText() const
{
	// XP calculated from position and performance
	int32 XP = 100 - (CachedResults.RacerResults.Num() > 0 ? (CachedResults.RacerResults[0].Position - 1) * 15 : 0);
	XP = FMath::Max(XP, 10);

	return FText::FromString(FString::Printf(TEXT("+%d XP"), XP));
}

void UMGRaceResultsWidget::ProcessResultsData(const FMGRaceResults& Results)
{
	ResultRows.Empty();

	float WinnerTime = 0.0f;

	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		FMGResultRowData Row;
		Row.Position = Racer.Position;
		Row.DriverName = Racer.DisplayName;
		Row.TotalTime = Racer.TotalTime;
		Row.BestLap = Racer.BestLapTime;
		Row.bIsPlayer = !Racer.bIsAI;
		Row.bIsDNF = Racer.bDNF;
		Row.bHasBestLap = (Racer.BestLapTime > 0.0f && FMath::Abs(Racer.BestLapTime - Results.BestLapTime) < 0.001f);

		if (Racer.Position == 1)
		{
			WinnerTime = Racer.TotalTime;
		}

		if (Racer.Position > 1 && WinnerTime > 0.0f)
		{
			Row.GapToWinner = Racer.TotalTime - WinnerTime;
		}

		// Get vehicle name from pawn if available
		if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
		{
			FMGVehicleData Config = Vehicle->GetVehicleConfiguration();
			Row.VehicleName = FText::FromString(Config.DisplayName);
		}
		else
		{
			Row.VehicleName = FText::FromString(TEXT("Unknown"));
		}

		ResultRows.Add(Row);
	}

	// Sort by position
	ResultRows.Sort([](const FMGResultRowData& A, const FMGResultRowData& B)
	{
		return A.Position < B.Position;
	});
}

void UMGRaceResultsWidget::CreateUIElements()
{
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		if (RootCanvas)
		{
			WidgetTree->RootWidget = RootCanvas;
		}
	}

	if (!RootCanvas) return;

	// Background overlay
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	if (Background)
	{
		Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
		RootCanvas->AddChild(Background);

		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Background->Slot))
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			Slot->SetOffsets(FMargin(0.0f));
		}
	}

	// Header
	if (!HeaderText)
	{
		HeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (HeaderText)
		{
			HeaderText->SetText(FText::FromString(TEXT("RACE COMPLETE")));
			HeaderText->SetColorAndOpacity(WinnerColor);
			HeaderText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = HeaderText->GetFont();
			FontInfo.Size = 72;
			FontInfo.OutlineSettings.OutlineSize = 3;
			FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
			HeaderText->SetFont(FontInfo);

			RootCanvas->AddChild(HeaderText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(HeaderText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				Slot->SetAlignment(FVector2D(0.5f, 0.0f));
				Slot->SetPosition(FVector2D(0.0f, 50.0f));
				Slot->SetSize(FVector2D(800.0f, 100.0f));
			}
		}
	}

	// Sub header
	if (!SubHeaderText)
	{
		SubHeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (SubHeaderText)
		{
			SubHeaderText->SetText(FText::FromString(TEXT("CIRCUIT RACE")));
			SubHeaderText->SetColorAndOpacity(PlayerHighlightColor);
			SubHeaderText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = SubHeaderText->GetFont();
			FontInfo.Size = 28;
			SubHeaderText->SetFont(FontInfo);

			RootCanvas->AddChild(SubHeaderText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(SubHeaderText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				Slot->SetAlignment(FVector2D(0.5f, 0.0f));
				Slot->SetPosition(FVector2D(0.0f, 140.0f));
				Slot->SetSize(FVector2D(600.0f, 40.0f));
			}
		}
	}

	// Results list box
	if (!ResultsListBox)
	{
		ResultsListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		if (ResultsListBox)
		{
			RootCanvas->AddChild(ResultsListBox);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(ResultsListBox->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				Slot->SetAlignment(FVector2D(0.5f, 0.0f));
				Slot->SetPosition(FVector2D(0.0f, 200.0f));
				Slot->SetSize(FVector2D(900.0f, 400.0f));
			}
		}
	}

	// Credits display
	if (!CreditsText)
	{
		CreditsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (CreditsText)
		{
			CreditsText->SetText(FText::FromString(TEXT("+$0 CREDITS")));
			CreditsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
			CreditsText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = CreditsText->GetFont();
			FontInfo.Size = 36;
			CreditsText->SetFont(FontInfo);

			RootCanvas->AddChild(CreditsText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(CreditsText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.25f, 1.0f, 0.25f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -150.0f));
				Slot->SetSize(FVector2D(300.0f, 50.0f));
			}
		}
	}

	// Reputation display
	if (!ReputationText)
	{
		ReputationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (ReputationText)
		{
			ReputationText->SetText(FText::FromString(TEXT("+0 REP")));
			ReputationText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.6f, 1.0f)));
			ReputationText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = ReputationText->GetFont();
			FontInfo.Size = 36;
			ReputationText->SetFont(FontInfo);

			RootCanvas->AddChild(ReputationText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(ReputationText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.75f, 1.0f, 0.75f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -150.0f));
				Slot->SetSize(FVector2D(300.0f, 50.0f));
			}
		}
	}

	// Best lap
	if (!BestLapText)
	{
		BestLapText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (BestLapText)
		{
			BestLapText->SetText(FText::FromString(TEXT("")));
			BestLapText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.843f, 0.0f, 1.0f)));
			BestLapText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = BestLapText->GetFont();
			FontInfo.Size = 24;
			BestLapText->SetFont(FontInfo);

			RootCanvas->AddChild(BestLapText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(BestLapText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -100.0f));
				Slot->SetSize(FVector2D(400.0f, 35.0f));
			}
		}
	}

	// Prompt text
	if (!PromptText)
	{
		PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (PromptText)
		{
			PromptText->SetText(FText::FromString(TEXT("[ENTER] Continue    [R] Restart    [ESC] Quit")));
			PromptText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
			PromptText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = PromptText->GetFont();
			FontInfo.Size = 18;
			PromptText->SetFont(FontInfo);

			RootCanvas->AddChild(PromptText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(PromptText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -30.0f));
				Slot->SetSize(FVector2D(600.0f, 30.0f));
			}
		}
	}

	// Create history stats UI elements
	CreateHistoryStatsUI();
}

UWidget* UMGRaceResultsWidget::CreateResultRow(const FMGResultRowData& RowData)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (!Row) return nullptr;

	// Determine row color
	FSlateColor RowColor;
	if (RowData.bIsPlayer)
	{
		RowColor = PlayerHighlightColor;
	}
	else if (RowData.Position == 1)
	{
		RowColor = WinnerColor;
	}
	else if (RowData.bIsDNF)
	{
		RowColor = DNFColor;
	}
	else
	{
		RowColor = FSlateColor(FLinearColor::White);
	}

	// Position
	UTextBlock* PosText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (PosText)
	{
		if (RowData.bIsDNF)
		{
			PosText->SetText(FText::FromString(TEXT("DNF")));
		}
		else
		{
			PosText->SetText(FText::FromString(FString::Printf(TEXT("%d"), RowData.Position)));
		}
		PosText->SetColorAndOpacity(RowColor);

		FSlateFontInfo FontInfo = PosText->GetFont();
		FontInfo.Size = 24;
		PosText->SetFont(FontInfo);

		Row->AddChild(PosText);
		if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(PosText->Slot))
		{
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			Slot->SetPadding(FMargin(10.0f, 5.0f));
		}
	}

	// Driver name
	UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (NameText)
	{
		NameText->SetText(RowData.DriverName);
		NameText->SetColorAndOpacity(RowColor);

		FSlateFontInfo FontInfo = NameText->GetFont();
		FontInfo.Size = 20;
		NameText->SetFont(FontInfo);

		Row->AddChild(NameText);
		if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(NameText->Slot))
		{
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			Slot->SetPadding(FMargin(10.0f, 5.0f));
		}
	}

	// Vehicle name
	UTextBlock* VehicleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (VehicleText)
	{
		VehicleText->SetText(RowData.VehicleName);
		VehicleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));

		FSlateFontInfo FontInfo = VehicleText->GetFont();
		FontInfo.Size = 18;
		VehicleText->SetFont(FontInfo);

		Row->AddChild(VehicleText);
		if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(VehicleText->Slot))
		{
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			Slot->SetPadding(FMargin(10.0f, 5.0f));
		}
	}

	// Total time / Gap
	UTextBlock* TimeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (TimeText)
	{
		if (RowData.Position == 1)
		{
			TimeText->SetText(FormatTime(RowData.TotalTime));
		}
		else
		{
			TimeText->SetText(FormatGap(RowData.GapToWinner));
		}
		TimeText->SetColorAndOpacity(RowColor);

		FSlateFontInfo FontInfo = TimeText->GetFont();
		FontInfo.Size = 20;
		TimeText->SetFont(FontInfo);

		Row->AddChild(TimeText);
		if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(TimeText->Slot))
		{
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			Slot->SetPadding(FMargin(10.0f, 5.0f));
			Slot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}
	}

	// Best lap indicator
	if (RowData.bHasBestLap)
	{
		UTextBlock* BestText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (BestText)
		{
			BestText->SetText(FText::FromString(TEXT("FASTEST")));
			BestText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.843f, 0.0f, 1.0f)));

			FSlateFontInfo FontInfo = BestText->GetFont();
			FontInfo.Size = 14;
			BestText->SetFont(FontInfo);

			Row->AddChild(BestText);
			if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(BestText->Slot))
			{
				Slot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				Slot->SetPadding(FMargin(10.0f, 5.0f));
			}
		}
	}

	return Row;
}

FText UMGRaceResultsWidget::FormatTime(float Seconds) const
{
	if (Seconds <= 0.0f)
	{
		return FText::FromString(TEXT("--:--.---"));
	}

	int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	float RemainingSecs = FMath::Fmod(Seconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(RemainingSecs);
	int32 Milliseconds = FMath::FloorToInt((RemainingSecs - WholeSeconds) * 1000.0f);

	return FText::FromString(FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds));
}

FText UMGRaceResultsWidget::FormatGap(float Seconds) const
{
	if (Seconds <= 0.0f)
	{
		return FText::FromString(TEXT("+0.000"));
	}

	return FText::FromString(FString::Printf(TEXT("+%.3f"), Seconds));
}

void UMGRaceResultsWidget::HandleContinue()
{
	OnContinue.Broadcast();
}

void UMGRaceResultsWidget::HandleRestart()
{
	OnRestart.Broadcast();
}

void UMGRaceResultsWidget::HandleQuit()
{
	OnQuit.Broadcast();
}

void UMGRaceResultsWidget::RevealNextRow()
{
	if (CurrentRevealRow < ResultRows.Num())
	{
		PlayRowRevealAnimation(CurrentRevealRow);
		CurrentRevealRow++;
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RowRevealTimerHandle);
		}
	}
}

// ==========================================
// FLOW SUBSYSTEM INTEGRATION (Stage 51)
// ==========================================

void UMGRaceResultsWidget::DisplayFlowResults(const FMGRaceFlowResult& FlowResult)
{
	CachedFlowResult = FlowResult;

	// Convert flow result to display format
	ResultRows.Empty();

	// Create player row
	FMGResultRowData PlayerRow;
	PlayerRow.Position = FlowResult.PlayerPosition;
	PlayerRow.DriverName = FText::FromString(TEXT("You"));
	PlayerRow.VehicleName = FText::FromString(TEXT("Your Vehicle"));
	PlayerRow.TotalTime = FlowResult.PlayerTotalTime;
	PlayerRow.BestLap = FlowResult.PlayerBestLap;
	PlayerRow.bIsPlayer = true;
	PlayerRow.bIsDNF = !FlowResult.bPlayerFinished;
	ResultRows.Add(PlayerRow);

	// Add placeholder AI rows based on position
	for (int32 i = 1; i <= FlowResult.TotalRacers; ++i)
	{
		if (i == FlowResult.PlayerPosition)
		{
			continue; // Skip player position
		}

		FMGResultRowData AIRow;
		AIRow.Position = i;
		AIRow.DriverName = FText::FromString(FString::Printf(TEXT("Racer %d"), i));
		AIRow.VehicleName = FText::FromString(TEXT("Opponent"));
		AIRow.bIsPlayer = false;
		AIRow.bIsDNF = false;

		// Estimate times based on position
		if (i == 1)
		{
			AIRow.TotalTime = FlowResult.PlayerTotalTime - (FlowResult.PlayerPosition - 1) * 2.0f;
		}
		else
		{
			AIRow.TotalTime = FlowResult.PlayerTotalTime + (i - FlowResult.PlayerPosition) * 2.0f;
		}

		ResultRows.Add(AIRow);
	}

	// Sort by position
	ResultRows.Sort([](const FMGResultRowData& A, const FMGResultRowData& B)
	{
		return A.Position < B.Position;
	});

	// Calculate gaps
	float WinnerTime = ResultRows.Num() > 0 ? ResultRows[0].TotalTime : 0.0f;
	for (FMGResultRowData& Row : ResultRows)
	{
		if (Row.Position > 1)
		{
			Row.GapToWinner = Row.TotalTime - WinnerTime;
		}
	}

	// Update header
	if (HeaderText)
	{
		if (FlowResult.bPlayerWon)
		{
			HeaderText->SetText(FText::FromString(TEXT("VICTORY!")));
			HeaderText->SetColorAndOpacity(WinnerColor);
		}
		else
		{
			FString OrdinalSuffix;
			switch (FlowResult.PlayerPosition)
			{
				case 1: OrdinalSuffix = TEXT("st"); break;
				case 2: OrdinalSuffix = TEXT("nd"); break;
				case 3: OrdinalSuffix = TEXT("rd"); break;
				default: OrdinalSuffix = TEXT("th"); break;
			}
			HeaderText->SetText(FText::FromString(FString::Printf(TEXT("%d%s PLACE"), FlowResult.PlayerPosition, *OrdinalSuffix)));
			HeaderText->SetColorAndOpacity(PlayerHighlightColor);
		}
	}

	// Update rewards from flow result
	if (CreditsText)
	{
		CreditsText->SetText(FText::FromString(FString::Printf(TEXT("+$%lld"), FlowResult.CashEarned)));
	}

	if (ReputationText)
	{
		ReputationText->SetText(FText::FromString(FString::Printf(TEXT("+%d REP"), FlowResult.ReputationEarned)));
	}

	// Best lap
	if (BestLapText && FlowResult.PlayerBestLap > 0.0f)
	{
		BestLapText->SetText(FText::FromString(FString::Printf(TEXT("BEST LAP: %s"), *FormatTime(FlowResult.PlayerBestLap).ToString())));
	}

	// Rebuild results list
	if (ResultsListBox)
	{
		ResultsListBox->ClearChildren();
		for (const FMGResultRowData& Row : ResultRows)
		{
			UWidget* RowWidget = CreateResultRow(Row);
			if (RowWidget)
			{
				ResultsListBox->AddChild(RowWidget);
			}
		}
	}

	// Pink slip display
	if (WonPinkSlipVehicle() || LostPinkSlipVehicle())
	{
		if (SubHeaderText)
		{
			if (WonPinkSlipVehicle())
			{
				SubHeaderText->SetText(FText::FromString(FString::Printf(TEXT("WON: %s"), *FlowResult.PinkSlipWonVehicleID.ToString())));
				SubHeaderText->SetColorAndOpacity(WinnerColor);
			}
			else
			{
				SubHeaderText->SetText(FText::FromString(FString::Printf(TEXT("LOST: %s"), *FlowResult.PinkSlipLostVehicleID.ToString())));
				SubHeaderText->SetColorAndOpacity(DNFColor);
			}
		}
	}

	// Update history stats display (use cached results track if available, otherwise empty)
	FString TrackId = CachedResults.Config.TrackLayoutID.IsEmpty() ? TEXT("DefaultTrack") : CachedResults.Config.TrackLayoutID;
	UpdateHistoryStatsDisplay(TrackId, FlowResult.PlayerTotalTime);

	OnResultsReady();

	if (FlowResult.bPlayerWon)
	{
		PlayVictoryAnimation();
	}
	else
	{
		PlayDefeatAnimation();
	}
}

void UMGRaceResultsWidget::DisplayFromFlowSubsystem()
{
	// Get flow subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		RaceFlowSubsystem = GI->GetSubsystem<UMGRaceFlowSubsystem>();
		if (RaceFlowSubsystem.IsValid())
		{
			DisplayFlowResults(RaceFlowSubsystem->GetLastResult());
		}
	}
}

void UMGRaceResultsWidget::ContinueToGarage()
{
	if (!RaceFlowSubsystem.IsValid())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			RaceFlowSubsystem = GI->GetSubsystem<UMGRaceFlowSubsystem>();
		}
	}

	if (RaceFlowSubsystem.IsValid())
	{
		RaceFlowSubsystem->ContinueToGarage();
	}

	OnContinue.Broadcast();
}

void UMGRaceResultsWidget::RestartRace()
{
	if (!RaceFlowSubsystem.IsValid())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			RaceFlowSubsystem = GI->GetSubsystem<UMGRaceFlowSubsystem>();
		}
	}

	if (RaceFlowSubsystem.IsValid())
	{
		RaceFlowSubsystem->RestartRace();
	}

	OnRestart.Broadcast();
}

FText UMGRaceResultsWidget::GetPinkSlipVehicleText() const
{
	if (!CachedFlowResult.PinkSlipWonVehicleID.IsNone())
	{
		return FText::FromString(FString::Printf(TEXT("WON: %s"), *CachedFlowResult.PinkSlipWonVehicleID.ToString()));
	}
	else if (!CachedFlowResult.PinkSlipLostVehicleID.IsNone())
	{
		return FText::FromString(FString::Printf(TEXT("LOST: %s"), *CachedFlowResult.PinkSlipLostVehicleID.ToString()));
	}
	return FText::GetEmpty();
}

// ==========================================
// HISTORY STATS (Iteration 67)
// ==========================================

UMGRaceHistorySubsystem* UMGRaceResultsWidget::GetHistorySubsystem()
{
	if (!RaceHistorySubsystem.IsValid())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			RaceHistorySubsystem = GI->GetSubsystem<UMGRaceHistorySubsystem>();
		}
	}
	return RaceHistorySubsystem.Get();
}

void UMGRaceResultsWidget::UpdateHistoryStatsDisplay(const FString& TrackId, float PlayerTime)
{
	UMGRaceHistorySubsystem* HistorySub = GetHistorySubsystem();
	if (!HistorySub)
	{
		return;
	}

	// Get track stats
	CachedTrackStats = HistorySub->GetTrackStats(TrackId);
	CachedLifetimeStats = HistorySub->GetLifetimeStats();

	// Check for new personal best
	float PrevBest = HistorySub->GetPersonalBestTime(TrackId);
	bIsNewPB = (PrevBest <= 0.0f || PlayerTime < PrevBest) && PlayerTime > 0.0f;

	// Update personal best text
	if (PersonalBestText)
	{
		if (bIsNewPB)
		{
			if (PrevBest > 0.0f)
			{
				float Improvement = PrevBest - PlayerTime;
				PersonalBestText->SetText(FText::FromString(FString::Printf(TEXT("NEW PB! (-%0.3fs)"), Improvement)));
				PersonalBestText->SetColorAndOpacity(WinnerColor);
			}
			else
			{
				PersonalBestText->SetText(FText::FromString(TEXT("NEW PERSONAL BEST!")));
				PersonalBestText->SetColorAndOpacity(WinnerColor);
			}
		}
		else if (PrevBest > 0.0f)
		{
			float Diff = PlayerTime - PrevBest;
			PersonalBestText->SetText(FText::FromString(FString::Printf(TEXT("PB: %s (+%0.3fs)"), *FormatTime(PrevBest).ToString(), Diff)));
			PersonalBestText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
		}
		else
		{
			PersonalBestText->SetText(FText::GetEmpty());
		}
	}

	// Update win streak text
	if (WinStreakText)
	{
		if (CachedLifetimeStats.CurrentWinStreak > 1)
		{
			WinStreakText->SetText(FText::FromString(FString::Printf(TEXT("WIN STREAK: %d"), CachedLifetimeStats.CurrentWinStreak)));
			WinStreakText->SetColorAndOpacity(WinnerColor);
		}
		else if (CachedLifetimeStats.CurrentPodiumStreak > 2)
		{
			WinStreakText->SetText(FText::FromString(FString::Printf(TEXT("PODIUM STREAK: %d"), CachedLifetimeStats.CurrentPodiumStreak)));
			WinStreakText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.4f, 0.0f, 1.0f)));
		}
		else
		{
			WinStreakText->SetText(FText::GetEmpty());
		}
	}

	// Update career stats text
	if (CareerStatsText)
	{
		float WinRate = CachedLifetimeStats.GetWinRate() * 100.0f;
		CareerStatsText->SetText(FText::FromString(FString::Printf(TEXT("CAREER: %d WINS / %d RACES (%.0f%%)"),
			CachedLifetimeStats.TotalWins,
			CachedLifetimeStats.TotalRaces,
			WinRate)));
		CareerStatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
	}

	// Update track record text
	if (TrackRecordText)
	{
		if (CachedTrackStats.TotalRaces > 0)
		{
			TrackRecordText->SetText(FText::FromString(FString::Printf(TEXT("THIS TRACK: %d WINS / %d RACES"),
				CachedTrackStats.Wins,
				CachedTrackStats.TotalRaces)));
			TrackRecordText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f)));
		}
		else
		{
			TrackRecordText->SetText(FText::FromString(TEXT("FIRST TIME ON THIS TRACK!")));
			TrackRecordText->SetColorAndOpacity(PlayerHighlightColor);
		}
	}
}

void UMGRaceResultsWidget::CreateHistoryStatsUI()
{
	if (!RootCanvas) return;

	// Personal Best display
	if (!PersonalBestText)
	{
		PersonalBestText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (PersonalBestText)
		{
			PersonalBestText->SetText(FText::GetEmpty());
			PersonalBestText->SetColorAndOpacity(WinnerColor);
			PersonalBestText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = PersonalBestText->GetFont();
			FontInfo.Size = 28;
			PersonalBestText->SetFont(FontInfo);

			RootCanvas->AddChild(PersonalBestText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(PersonalBestText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				Slot->SetAlignment(FVector2D(0.5f, 0.0f));
				Slot->SetPosition(FVector2D(0.0f, 175.0f));
				Slot->SetSize(FVector2D(500.0f, 35.0f));
			}
		}
	}

	// Win streak display
	if (!WinStreakText)
	{
		WinStreakText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (WinStreakText)
		{
			WinStreakText->SetText(FText::GetEmpty());
			WinStreakText->SetColorAndOpacity(WinnerColor);
			WinStreakText->SetJustification(ETextJustify::Right);

			FSlateFontInfo FontInfo = WinStreakText->GetFont();
			FontInfo.Size = 20;
			WinStreakText->SetFont(FontInfo);

			RootCanvas->AddChild(WinStreakText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(WinStreakText->Slot))
			{
				Slot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
				Slot->SetAlignment(FVector2D(1.0f, 0.0f));
				Slot->SetPosition(FVector2D(-30.0f, 60.0f));
				Slot->SetSize(FVector2D(250.0f, 30.0f));
			}
		}
	}

	// Career stats display
	if (!CareerStatsText)
	{
		CareerStatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (CareerStatsText)
		{
			CareerStatsText->SetText(FText::GetEmpty());
			CareerStatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
			CareerStatsText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = CareerStatsText->GetFont();
			FontInfo.Size = 16;
			CareerStatsText->SetFont(FontInfo);

			RootCanvas->AddChild(CareerStatsText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(CareerStatsText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -200.0f));
				Slot->SetSize(FVector2D(500.0f, 25.0f));
			}
		}
	}

	// Track record display
	if (!TrackRecordText)
	{
		TrackRecordText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (TrackRecordText)
		{
			TrackRecordText->SetText(FText::GetEmpty());
			TrackRecordText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f)));
			TrackRecordText->SetJustification(ETextJustify::Center);

			FSlateFontInfo FontInfo = TrackRecordText->GetFont();
			FontInfo.Size = 14;
			TrackRecordText->SetFont(FontInfo);

			RootCanvas->AddChild(TrackRecordText);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(TrackRecordText->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
				Slot->SetAlignment(FVector2D(0.5f, 1.0f));
				Slot->SetPosition(FVector2D(0.0f, -175.0f));
				Slot->SetSize(FVector2D(400.0f, 22.0f));
			}
		}
	}
}

FText UMGRaceResultsWidget::GetWinStreakText() const
{
	if (CachedLifetimeStats.CurrentWinStreak > 1)
	{
		return FText::FromString(FString::Printf(TEXT("WIN STREAK: %d"), CachedLifetimeStats.CurrentWinStreak));
	}
	else if (CachedLifetimeStats.CurrentPodiumStreak > 2)
	{
		return FText::FromString(FString::Printf(TEXT("PODIUM STREAK: %d"), CachedLifetimeStats.CurrentPodiumStreak));
	}
	return FText::GetEmpty();
}

FText UMGRaceResultsWidget::GetPersonalBestText() const
{
	UMGRaceHistorySubsystem* HistorySub = const_cast<UMGRaceResultsWidget*>(this)->GetHistorySubsystem();
	if (!HistorySub)
	{
		return FText::GetEmpty();
	}

	FString TrackId = CachedResults.Config.TrackLayoutID;
	float PB = HistorySub->GetPersonalBestTime(TrackId);
	if (PB > 0.0f)
	{
		return FText::FromString(FString::Printf(TEXT("PERSONAL BEST: %s"), *FormatTime(PB).ToString()));
	}
	return FText::GetEmpty();
}

FText UMGRaceResultsWidget::GetCareerStatsText() const
{
	float WinRate = CachedLifetimeStats.GetWinRate() * 100.0f;
	return FText::FromString(FString::Printf(TEXT("CAREER: %d WINS / %d RACES (%.0f%%)"),
		CachedLifetimeStats.TotalWins,
		CachedLifetimeStats.TotalRaces,
		WinRate));
}

int32 UMGRaceResultsWidget::GetCurrentWinStreak() const
{
	return CachedLifetimeStats.CurrentWinStreak;
}

FText UMGRaceResultsWidget::GetTrackWinRateText() const
{
	if (CachedTrackStats.TotalRaces > 0)
	{
		float TrackWinRate = CachedTrackStats.TotalRaces > 0 ?
			(float)CachedTrackStats.Wins / CachedTrackStats.TotalRaces * 100.0f : 0.0f;
		return FText::FromString(FString::Printf(TEXT("THIS TRACK: %d/%d WINS (%.0f%%)"),
			CachedTrackStats.Wins,
			CachedTrackStats.TotalRaces,
			TrackWinRate));
	}
	return FText::FromString(TEXT("FIRST TIME ON THIS TRACK"));
}
