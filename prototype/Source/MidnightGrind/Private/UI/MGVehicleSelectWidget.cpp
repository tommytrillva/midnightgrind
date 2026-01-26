// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGVehicleSelectWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

UMGVehicleSelectWidget::UMGVehicleSelectWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGVehicleSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreateUIElements();
}

void UMGVehicleSelectWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMGVehicleSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateCarouselAnimation(InDeltaTime);
}

FReply UMGVehicleSelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Left || Key == EKeys::A || Key == EKeys::Gamepad_DPad_Left)
	{
		SelectPrevious();
		return FReply::Handled();
	}
	else if (Key == EKeys::Right || Key == EKeys::D || Key == EKeys::Gamepad_DPad_Right)
	{
		SelectNext();
		return FReply::Handled();
	}
	else if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		ConfirmSelection();
		return FReply::Handled();
	}
	else if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		Cancel();
		return FReply::Handled();
	}
	else if (Key == EKeys::Tab || Key == EKeys::Gamepad_FaceButton_Top)
	{
		ToggleFilterPanel();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ==========================================
// INITIALIZATION
// ==========================================

void UMGVehicleSelectWidget::Initialize(const TArray<FMGVehiclePreviewData>& AvailableVehicles)
{
	AllVehicles = AvailableVehicles;
	ApplyFilterAndSort();

	if (FilteredVehicles.Num() > 0)
	{
		CurrentIndex = 0;
		TargetIndex = 0;
		UpdateVehicleDisplay();
	}
}

void UMGVehicleSelectWidget::SetClassRestriction(const FString& AllowedClass)
{
	ClassRestriction = AllowedClass;
	ApplyFilterAndSort();
	UpdateVehicleDisplay();
}

void UMGVehicleSelectWidget::ClearClassRestriction()
{
	ClassRestriction.Empty();
	ApplyFilterAndSort();
	UpdateVehicleDisplay();
}

void UMGVehicleSelectWidget::SetInitialSelection(FName VehicleID)
{
	for (int32 i = 0; i < FilteredVehicles.Num(); ++i)
	{
		if (FilteredVehicles[i].VehicleID == VehicleID)
		{
			CurrentIndex = i;
			TargetIndex = i;
			UpdateVehicleDisplay();
			return;
		}
	}
}

// ==========================================
// NAVIGATION
// ==========================================

void UMGVehicleSelectWidget::SelectNext()
{
	if (FilteredVehicles.Num() == 0)
	{
		return;
	}

	TargetIndex = (CurrentIndex + 1) % FilteredVehicles.Num();
	bIsAnimating = true;
	CarouselAnimProgress = 0.0f;

	// Immediately update for responsiveness
	CurrentIndex = TargetIndex;
	UpdateVehicleDisplay();

	OnVehicleHighlighted.Broadcast(FilteredVehicles[CurrentIndex].VehicleID);
}

void UMGVehicleSelectWidget::SelectPrevious()
{
	if (FilteredVehicles.Num() == 0)
	{
		return;
	}

	TargetIndex = (CurrentIndex - 1 + FilteredVehicles.Num()) % FilteredVehicles.Num();
	bIsAnimating = true;
	CarouselAnimProgress = 0.0f;

	CurrentIndex = TargetIndex;
	UpdateVehicleDisplay();

	OnVehicleHighlighted.Broadcast(FilteredVehicles[CurrentIndex].VehicleID);
}

void UMGVehicleSelectWidget::SelectIndex(int32 Index)
{
	if (Index < 0 || Index >= FilteredVehicles.Num())
	{
		return;
	}

	TargetIndex = Index;
	bIsAnimating = true;
	CarouselAnimProgress = 0.0f;

	CurrentIndex = TargetIndex;
	UpdateVehicleDisplay();

	OnVehicleHighlighted.Broadcast(FilteredVehicles[CurrentIndex].VehicleID);
}

void UMGVehicleSelectWidget::ConfirmSelection()
{
	if (!CanSelectCurrentVehicle())
	{
		// Play error sound or show message
		return;
	}

	OnVehicleConfirmed.Broadcast(FilteredVehicles[CurrentIndex].VehicleID);
}

void UMGVehicleSelectWidget::Cancel()
{
	OnCancelled.Broadcast();
}

// ==========================================
// FILTERING/SORTING
// ==========================================

void UMGVehicleSelectWidget::SetFilter(EMGVehicleFilter Filter)
{
	CurrentFilter = Filter;
	ApplyFilterAndSort();
	UpdateVehicleDisplay();
}

void UMGVehicleSelectWidget::SetSortMode(EMGVehicleSort SortMode)
{
	CurrentSort = SortMode;
	ApplyFilterAndSort();
	UpdateVehicleDisplay();
}

void UMGVehicleSelectWidget::ToggleFilterPanel()
{
	bFilterPanelVisible = !bFilterPanelVisible;

	if (FilterPanel)
	{
		FilterPanel->SetVisibility(bFilterPanelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// ==========================================
// STATE
// ==========================================

FMGVehiclePreviewData UMGVehicleSelectWidget::GetCurrentVehicle() const
{
	if (CurrentIndex >= 0 && CurrentIndex < FilteredVehicles.Num())
	{
		return FilteredVehicles[CurrentIndex];
	}
	return FMGVehiclePreviewData();
}

bool UMGVehicleSelectWidget::CanSelectCurrentVehicle() const
{
	if (CurrentIndex < 0 || CurrentIndex >= FilteredVehicles.Num())
	{
		return false;
	}

	const FMGVehiclePreviewData& Vehicle = FilteredVehicles[CurrentIndex];

	// Must be owned
	if (!Vehicle.bIsOwned)
	{
		return false;
	}

	// Must not be locked
	if (Vehicle.bIsLocked)
	{
		return false;
	}

	// Must meet class restriction if set
	if (!ClassRestriction.IsEmpty())
	{
		if (!Vehicle.ClassTier.ToString().Equals(ClassRestriction, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	return true;
}

// ==========================================
// UI CREATION
// ==========================================

void UMGVehicleSelectWidget::CreateUIElements()
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

	CreateInfoPanel();
	CreateStatBars();
	CreateNavigationControls();
	CreateFilterPanel();
}

void UMGVehicleSelectWidget::CreateInfoPanel()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Info panel container - left side
	UCanvasPanel* InfoPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InfoPanel"));
	if (InfoPanel)
	{
		RootCanvas->AddChild(InfoPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(InfoPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.0f, 0.4f, 1.0f));
			Slot->SetOffsets(FMargin(40.0f, 100.0f, 40.0f, 150.0f));
		}

		// Background
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InfoBg"));
		if (Background)
		{
			InfoPanel->AddChild(Background);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Background->Slot))
			{
				BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				BgSlot->SetOffsets(FMargin(0.0f));
			}
			Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
			Background->SetPadding(FMargin(30.0f));

			// Vertical layout for info
			UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoVBox"));
			if (InfoVBox)
			{
				Background->AddChild(InfoVBox);

				// Manufacturer
				ManufacturerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Manufacturer"));
				if (ManufacturerText)
				{
					InfoVBox->AddChild(ManufacturerText);
					FSlateFontInfo Font = ManufacturerText->GetFont();
					Font.Size = 24;
					ManufacturerText->SetFont(Font);
					ManufacturerText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.7f));
				}

				// Vehicle name
				VehicleNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VehicleName"));
				if (VehicleNameText)
				{
					InfoVBox->AddChild(VehicleNameText);
					if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(VehicleNameText->Slot))
					{
						VSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
					}
					FSlateFontInfo Font = VehicleNameText->GetFont();
					Font.Size = 48;
					VehicleNameText->SetFont(Font);
					VehicleNameText->SetColorAndOpacity(FSlateColor(CyanNeon));
				}

				// Class and PI row
				UHorizontalBox* ClassRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ClassRow"));
				if (ClassRow)
				{
					InfoVBox->AddChild(ClassRow);
					if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(ClassRow->Slot))
					{
						VSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
					}

					ClassText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Class"));
					if (ClassText)
					{
						ClassRow->AddChild(ClassText);
						if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(ClassText->Slot))
						{
							HSlot->SetPadding(FMargin(0.0f, 0.0f, 20.0f, 0.0f));
						}
						FSlateFontInfo Font = ClassText->GetFont();
						Font.Size = 36;
						ClassText->SetFont(Font);
					}

					PIText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PI"));
					if (PIText)
					{
						ClassRow->AddChild(PIText);
						FSlateFontInfo Font = PIText->GetFont();
						Font.Size = 36;
						PIText->SetFont(Font);
						PIText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
					}
				}

				// Type and Drivetrain
				UHorizontalBox* TypeRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TypeRow"));
				if (TypeRow)
				{
					InfoVBox->AddChild(TypeRow);
					if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(TypeRow->Slot))
					{
						VSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 30.0f));
					}

					TypeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Type"));
					if (TypeText)
					{
						TypeRow->AddChild(TypeText);
						if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(TypeText->Slot))
						{
							HSlot->SetPadding(FMargin(0.0f, 0.0f, 30.0f, 0.0f));
						}
						FSlateFontInfo Font = TypeText->GetFont();
						Font.Size = 20;
						TypeText->SetFont(Font);
						TypeText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.6f));
					}

					DrivetrainText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Drivetrain"));
					if (DrivetrainText)
					{
						TypeRow->AddChild(DrivetrainText);
						FSlateFontInfo Font = DrivetrainText->GetFont();
						Font.Size = 20;
						DrivetrainText->SetFont(Font);
						DrivetrainText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.6f));
					}
				}

				// Description
				DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Description"));
				if (DescriptionText)
				{
					InfoVBox->AddChild(DescriptionText);
					if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(DescriptionText->Slot))
					{
						VSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 30.0f));
					}
					FSlateFontInfo Font = DescriptionText->GetFont();
					Font.Size = 18;
					DescriptionText->SetFont(Font);
					DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.8f));
					DescriptionText->SetAutoWrapText(true);
				}

				// Price/Lock text
				PriceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Price"));
				if (PriceText)
				{
					InfoVBox->AddChild(PriceText);
					FSlateFontInfo Font = PriceText->GetFont();
					Font.Size = 28;
					PriceText->SetFont(Font);
					PriceText->SetColorAndOpacity(FSlateColor(GoldColor));
				}

				LockReasonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LockReason"));
				if (LockReasonText)
				{
					InfoVBox->AddChild(LockReasonText);
					FSlateFontInfo Font = LockReasonText->GetFont();
					Font.Size = 20;
					LockReasonText->SetFont(Font);
					LockReasonText->SetColorAndOpacity(FSlateColor(RedNeon));
				}
			}
		}
	}
}

void UMGVehicleSelectWidget::CreateStatBars()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Stats panel - bottom right
	UCanvasPanel* StatsPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("StatsPanel"));
	if (StatsPanel)
	{
		RootCanvas->AddChild(StatsPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(StatsPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.6f, 0.5f, 1.0f, 0.85f));
			Slot->SetOffsets(FMargin(20.0f, 0.0f, 40.0f, 0.0f));
		}

		UBorder* StatsBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatsBg"));
		if (StatsBg)
		{
			StatsPanel->AddChild(StatsBg);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(StatsBg->Slot))
			{
				BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				BgSlot->SetOffsets(FMargin(0.0f));
			}
			StatsBg->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
			StatsBg->SetPadding(FMargin(25.0f));

			UVerticalBox* StatsVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsVBox"));
			if (StatsVBox)
			{
				StatsBg->AddChild(StatsVBox);

				// Create stat rows
				auto CreateStatRow = [this, StatsVBox](const FString& Label, UProgressBar*& OutBar, int32 Index)
				{
					UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
					if (Row)
					{
						StatsVBox->AddChild(Row);
						if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(Row->Slot))
						{
							VSlot->SetPadding(FMargin(0.0f, 8.0f));
						}

						// Label
						UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
						if (LabelText)
						{
							Row->AddChild(LabelText);
							if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(LabelText->Slot))
							{
								HSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
								HSlot->SetHorizontalAlignment(HAlign_Left);
							}
							FSlateFontInfo Font = LabelText->GetFont();
							Font.Size = 20;
							LabelText->SetFont(Font);
							LabelText->SetText(FText::FromString(Label));
							LabelText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.8f));
							StatLabels.Add(LabelText);
						}

						// Bar
						OutBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
						if (OutBar)
						{
							Row->AddChild(OutBar);
							if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(OutBar->Slot))
							{
								HSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
								HSlot->SetVerticalAlignment(VAlign_Center);
								HSlot->SetPadding(FMargin(20.0f, 0.0f));
							}
							OutBar->SetFillColorAndOpacity(CyanNeon);
						}

						// Value text
						UTextBlock* ValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
						if (ValueText)
						{
							Row->AddChild(ValueText);
							if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(ValueText->Slot))
							{
								HSlot->SetHorizontalAlignment(HAlign_Right);
							}
							FSlateFontInfo Font = ValueText->GetFont();
							Font.Size = 20;
							ValueText->SetFont(Font);
							ValueText->SetColorAndOpacity(FSlateColor(CyanNeon));
							StatValues.Add(ValueText);
						}
					}
				};

				CreateStatRow(TEXT("SPEED"), SpeedBar, 0);
				CreateStatRow(TEXT("ACCEL"), AccelBar, 1);
				CreateStatRow(TEXT("HANDLING"), HandlingBar, 2);
				CreateStatRow(TEXT("BRAKING"), BrakingBar, 3);
				CreateStatRow(TEXT("DRIFT"), DriftBar, 4);
			}
		}
	}
}

void UMGVehicleSelectWidget::CreateNavigationControls()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Navigation panel - bottom
	UCanvasPanel* NavPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NavPanel"));
	if (NavPanel)
	{
		RootCanvas->AddChild(NavPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(NavPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.9f, 1.0f, 1.0f));
			Slot->SetOffsets(FMargin(40.0f, 0.0f, 40.0f, 20.0f));
		}

		UHorizontalBox* NavHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NavHBox"));
		if (NavHBox)
		{
			NavPanel->AddChild(NavHBox);
			if (UCanvasPanelSlot* HSlot = Cast<UCanvasPanelSlot>(NavHBox->Slot))
			{
				HSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				HSlot->SetOffsets(FMargin(0.0f));
			}

			// Index indicator (e.g., "5 / 24")
			IndexText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Index"));
			if (IndexText)
			{
				NavHBox->AddChild(IndexText);
				if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(IndexText->Slot))
				{
					Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
					Slot->SetHorizontalAlignment(HAlign_Left);
					Slot->SetVerticalAlignment(VAlign_Center);
				}
				FSlateFontInfo Font = IndexText->GetFont();
				Font.Size = 24;
				IndexText->SetFont(Font);
				IndexText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.6f));
			}

			// Control hints
			UTextBlock* HintsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Hints"));
			if (HintsText)
			{
				NavHBox->AddChild(HintsText);
				if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(HintsText->Slot))
				{
					Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
					Slot->SetHorizontalAlignment(HAlign_Center);
					Slot->SetVerticalAlignment(VAlign_Center);
				}
				FSlateFontInfo Font = HintsText->GetFont();
				Font.Size = 18;
				HintsText->SetFont(Font);
				HintsText->SetText(FText::FromString(TEXT("[A/D] Navigate   [ENTER] Select   [TAB] Filter   [ESC] Back")));
				HintsText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.5f));
			}
		}
	}
}

void UMGVehicleSelectWidget::CreateFilterPanel()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Filter panel - overlay on right side
	FilterPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("FilterPanel"));
	if (FilterPanel)
	{
		RootCanvas->AddChild(FilterPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FilterPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.75f, 0.1f, 0.98f, 0.5f));
			Slot->SetOffsets(FMargin(0.0f));
		}
		FilterPanel->SetVisibility(ESlateVisibility::Collapsed);

		UBorder* FilterBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FilterBg"));
		if (FilterBg)
		{
			FilterPanel->AddChild(FilterBg);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(FilterBg->Slot))
			{
				BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				BgSlot->SetOffsets(FMargin(0.0f));
			}
			FilterBg->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.95f));
			FilterBg->SetPadding(FMargin(20.0f));

			FilterOptions = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FilterOptions"));
			if (FilterOptions)
			{
				FilterBg->AddChild(FilterOptions);

				// Title
				UTextBlock* FilterTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FilterTitle"));
				if (FilterTitle)
				{
					FilterOptions->AddChild(FilterTitle);
					if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(FilterTitle->Slot))
					{
						VSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 15.0f));
					}
					FSlateFontInfo Font = FilterTitle->GetFont();
					Font.Size = 24;
					FilterTitle->SetFont(Font);
					FilterTitle->SetText(FText::FromString(TEXT("FILTER")));
					FilterTitle->SetColorAndOpacity(FSlateColor(CyanNeon));
				}

				// Filter options would be added here as buttons
				// For brevity, showing placeholder
				UTextBlock* FilterPlaceholder = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
				if (FilterPlaceholder)
				{
					FilterOptions->AddChild(FilterPlaceholder);
					FSlateFontInfo Font = FilterPlaceholder->GetFont();
					Font.Size = 16;
					FilterPlaceholder->SetFont(Font);
					FilterPlaceholder->SetText(FText::FromString(TEXT("All Vehicles\nOwned Only\nClass S\nClass A\nClass B\nClass C\nClass D")));
					FilterPlaceholder->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.7f));
				}
			}
		}
	}
}

// ==========================================
// UI UPDATE
// ==========================================

void UMGVehicleSelectWidget::UpdateVehicleDisplay()
{
	if (CurrentIndex < 0 || CurrentIndex >= FilteredVehicles.Num())
	{
		return;
	}

	const FMGVehiclePreviewData& Vehicle = FilteredVehicles[CurrentIndex];

	// Update text fields
	if (VehicleNameText)
	{
		VehicleNameText->SetText(Vehicle.DisplayName);

		// Gray out if not selectable
		FLinearColor NameColor = CanSelectCurrentVehicle() ? CyanNeon : FLinearColor::White * 0.4f;
		VehicleNameText->SetColorAndOpacity(FSlateColor(NameColor));
	}

	if (ManufacturerText)
	{
		ManufacturerText->SetText(Vehicle.Manufacturer);
	}

	if (ClassText)
	{
		ClassText->SetText(Vehicle.ClassTier);
		ClassText->SetColorAndOpacity(GetClassColor(Vehicle.ClassTier));
	}

	if (PIText)
	{
		PIText->SetText(FText::Format(FText::FromString(TEXT("PI {0}")), FText::AsNumber(Vehicle.PerformanceIndex)));
	}

	if (TypeText)
	{
		TypeText->SetText(Vehicle.VehicleType);
	}

	if (DrivetrainText)
	{
		DrivetrainText->SetText(Vehicle.Drivetrain);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(Vehicle.Description);
	}

	// Price/ownership status
	if (PriceText)
	{
		if (Vehicle.bIsOwned)
		{
			PriceText->SetText(FText::FromString(TEXT("OWNED")));
			PriceText->SetColorAndOpacity(FSlateColor(GreenNeon));
		}
		else
		{
			PriceText->SetText(FormatPrice(Vehicle.Price));
			PriceText->SetColorAndOpacity(FSlateColor(GoldColor));
		}
	}

	if (LockReasonText)
	{
		if (Vehicle.bIsLocked)
		{
			LockReasonText->SetText(Vehicle.UnlockRequirement);
			LockReasonText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			LockReasonText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update stats
	UpdateStatBars(Vehicle);
	UpdateNavigationIndicators();
}

void UMGVehicleSelectWidget::UpdateStatBars(const FMGVehiclePreviewData& Vehicle)
{
	auto UpdateBar = [this](UProgressBar* Bar, float Value, int32 ValueIndex)
	{
		if (Bar)
		{
			Bar->SetPercent(Value);
			Bar->SetFillColorAndOpacity(GetStatBarColor(Value));
		}
		if (ValueIndex < StatValues.Num() && StatValues[ValueIndex])
		{
			int32 DisplayValue = FMath::RoundToInt(Value * 10.0f);
			StatValues[ValueIndex]->SetText(FText::AsNumber(DisplayValue));
			StatValues[ValueIndex]->SetColorAndOpacity(FSlateColor(GetStatBarColor(Value)));
		}
	};

	UpdateBar(SpeedBar, Vehicle.SpeedRating, 0);
	UpdateBar(AccelBar, Vehicle.AccelerationRating, 1);
	UpdateBar(HandlingBar, Vehicle.HandlingRating, 2);
	UpdateBar(BrakingBar, Vehicle.BrakingRating, 3);
	UpdateBar(DriftBar, Vehicle.DriftRating, 4);
}

void UMGVehicleSelectWidget::UpdateNavigationIndicators()
{
	if (IndexText)
	{
		FText IndexString = FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FText::AsNumber(CurrentIndex + 1),
			FText::AsNumber(FilteredVehicles.Num())
		);
		IndexText->SetText(IndexString);
	}
}

void UMGVehicleSelectWidget::UpdateCarouselAnimation(float DeltaTime)
{
	if (!bIsAnimating)
	{
		return;
	}

	CarouselAnimProgress += DeltaTime * 5.0f;

	if (CarouselAnimProgress >= 1.0f)
	{
		CarouselAnimProgress = 1.0f;
		bIsAnimating = false;
	}

	// Animation complete - the actual visual carousel would be updated here
	// For now, we update immediately in SelectNext/SelectPrevious
}

// ==========================================
// FILTERING
// ==========================================

void UMGVehicleSelectWidget::ApplyFilterAndSort()
{
	FilteredVehicles.Empty();

	// Apply filter
	for (const FMGVehiclePreviewData& Vehicle : AllVehicles)
	{
		if (PassesFilter(Vehicle))
		{
			FilteredVehicles.Add(Vehicle);
		}
	}

	// Sort
	SortVehicles();

	// Clamp current index
	if (FilteredVehicles.Num() > 0)
	{
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, FilteredVehicles.Num() - 1);
	}
	else
	{
		CurrentIndex = 0;
	}
}

bool UMGVehicleSelectWidget::PassesFilter(const FMGVehiclePreviewData& Vehicle) const
{
	// Class restriction always applies
	if (!ClassRestriction.IsEmpty())
	{
		if (!Vehicle.ClassTier.ToString().Equals(ClassRestriction, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	// Filter type
	switch (CurrentFilter)
	{
	case EMGVehicleFilter::Owned:
		return Vehicle.bIsOwned;

	case EMGVehicleFilter::Available:
		return !Vehicle.bIsLocked;

	case EMGVehicleFilter::ClassS:
		return Vehicle.ClassTier.ToString().Equals(TEXT("S"), ESearchCase::IgnoreCase);

	case EMGVehicleFilter::ClassA:
		return Vehicle.ClassTier.ToString().Equals(TEXT("A"), ESearchCase::IgnoreCase);

	case EMGVehicleFilter::ClassB:
		return Vehicle.ClassTier.ToString().Equals(TEXT("B"), ESearchCase::IgnoreCase);

	case EMGVehicleFilter::ClassC:
		return Vehicle.ClassTier.ToString().Equals(TEXT("C"), ESearchCase::IgnoreCase);

	case EMGVehicleFilter::ClassD:
		return Vehicle.ClassTier.ToString().Equals(TEXT("D"), ESearchCase::IgnoreCase);

	case EMGVehicleFilter::All:
	default:
		return true;
	}
}

void UMGVehicleSelectWidget::SortVehicles()
{
	switch (CurrentSort)
	{
	case EMGVehicleSort::PerformanceIndex:
		FilteredVehicles.Sort([](const FMGVehiclePreviewData& A, const FMGVehiclePreviewData& B)
		{
			return A.PerformanceIndex > B.PerformanceIndex;
		});
		break;

	case EMGVehicleSort::Name:
		FilteredVehicles.Sort([](const FMGVehiclePreviewData& A, const FMGVehiclePreviewData& B)
		{
			return A.DisplayName.CompareTo(B.DisplayName) < 0;
		});
		break;

	case EMGVehicleSort::Manufacturer:
		FilteredVehicles.Sort([](const FMGVehiclePreviewData& A, const FMGVehiclePreviewData& B)
		{
			return A.Manufacturer.CompareTo(B.Manufacturer) < 0;
		});
		break;

	case EMGVehicleSort::Price:
		FilteredVehicles.Sort([](const FMGVehiclePreviewData& A, const FMGVehiclePreviewData& B)
		{
			return A.Price < B.Price;
		});
		break;

	default:
		break;
	}
}

// ==========================================
// HELPERS
// ==========================================

FSlateColor UMGVehicleSelectWidget::GetClassColor(const FText& ClassTier) const
{
	FString ClassStr = ClassTier.ToString().ToUpper();

	if (ClassStr == TEXT("S"))
	{
		return FSlateColor(ClassSColor);
	}
	else if (ClassStr == TEXT("A"))
	{
		return FSlateColor(ClassAColor);
	}
	else if (ClassStr == TEXT("B"))
	{
		return FSlateColor(ClassBColor);
	}
	else if (ClassStr == TEXT("C"))
	{
		return FSlateColor(ClassCColor);
	}
	else if (ClassStr == TEXT("D"))
	{
		return FSlateColor(ClassDColor);
	}

	return FSlateColor(FLinearColor::White);
}

FText UMGVehicleSelectWidget::FormatPrice(int64 Price) const
{
	if (Price >= 1000000)
	{
		return FText::Format(FText::FromString(TEXT("${0}M")), FText::AsNumber(Price / 1000000));
	}
	else if (Price >= 1000)
	{
		return FText::Format(FText::FromString(TEXT("${0}K")), FText::AsNumber(Price / 1000));
	}
	else
	{
		return FText::Format(FText::FromString(TEXT("${0}")), FText::AsNumber(Price));
	}
}

FLinearColor UMGVehicleSelectWidget::GetStatBarColor(float Value) const
{
	// Gradient from red (low) through yellow to cyan (high)
	if (Value < 0.5f)
	{
		float T = Value * 2.0f;
		return FMath::Lerp(RedNeon, YellowNeon, T);
	}
	else
	{
		float T = (Value - 0.5f) * 2.0f;
		return FMath::Lerp(YellowNeon, CyanNeon, T);
	}
}
