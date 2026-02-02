// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGPartListItemWidget.h"
#include "Vehicle/MGStatCalculator.h"

// ==========================================
// UMGPartListItemWidget
// ==========================================

UMGPartListItemWidget::UMGPartListItemWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGPartListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMGPartListItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bIsHovered = true;
	OnUpdateVisualState();
	OnPartItemHovered.Broadcast(PartData.PartID);
}

void UMGPartListItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bIsHovered = false;
	OnUpdateVisualState();
}

FReply UMGPartListItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnPartItemClicked.Broadcast(PartData.PartID);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMGPartListItemWidget::SetPartData(const FMGUIPartData& InPartData)
{
	PartData = InPartData;
	OnPartDataChanged();
	OnUpdateVisualState();
}

void UMGPartListItemWidget::SetSelected(bool bInSelected)
{
	if (bIsSelected != bInSelected)
	{
		bIsSelected = bInSelected;
		OnUpdateVisualState();
	}
}

void UMGPartListItemWidget::SetFocused(bool bInFocused)
{
	if (bIsFocused != bInFocused)
	{
		bIsFocused = bInFocused;
		OnUpdateVisualState();
	}
}

FLinearColor UMGPartListItemWidget::GetTierColor() const
{
	switch (PartData.Tier)
	{
	case EMGPartTier::Stock:
		return TierColorStock;
	case EMGPartTier::Street:
		return TierColorStreet;
	case EMGPartTier::Sport:
		return TierColorSport;
	case EMGPartTier::Race:
		return TierColorRace;
	case EMGPartTier::Pro:
		return TierColorPro;
	case EMGPartTier::Legendary:
		return TierColorLegendary;
	default:
		return TierColorStock;
	}
}

FText UMGPartListItemWidget::GetTierText() const
{
	switch (PartData.Tier)
	{
	case EMGPartTier::Stock:
		return NSLOCTEXT("MG", "Tier_Stock", "STOCK");
	case EMGPartTier::Street:
		return NSLOCTEXT("MG", "Tier_Street", "STREET");
	case EMGPartTier::Sport:
		return NSLOCTEXT("MG", "Tier_Sport", "SPORT");
	case EMGPartTier::Race:
		return NSLOCTEXT("MG", "Tier_Race", "RACE");
	case EMGPartTier::Pro:
		return NSLOCTEXT("MG", "Tier_Pro", "PRO");
	case EMGPartTier::Legendary:
		return NSLOCTEXT("MG", "Tier_Legendary", "LEGENDARY");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UMGPartListItemWidget::GetBackgroundColor() const
{
	if (PartData.bLocked)
	{
		return NormalBackgroundColor * LockedTintColor;
	}

	if (bIsSelected || bIsFocused)
	{
		return bIsSelected ? SelectedBackgroundColor : FocusedBackgroundColor;
	}

	if (bIsHovered)
	{
		return HoveredBackgroundColor;
	}

	return NormalBackgroundColor;
}

FLinearColor UMGPartListItemWidget::GetBorderColor() const
{
	if (bIsSelected || bIsFocused)
	{
		return SelectedBorderColor;
	}

	return NormalBorderColor;
}

FText UMGPartListItemWidget::GetFormattedPrice() const
{
	if (PartData.bOwned)
	{
		return NSLOCTEXT("MG", "Part_Owned", "OWNED");
	}

	if (PartData.bLocked)
	{
		return NSLOCTEXT("MG", "Part_Locked", "LOCKED");
	}

	// Format with thousand separators
	FNumberFormattingOptions Options;
	Options.UseGrouping = true;

	return FText::Format(NSLOCTEXT("MG", "PriceFormat", "${0}"),
		FText::AsNumber(PartData.Price, &Options));
}

TSoftObjectPtr<UTexture2D> UMGPartListItemWidget::GetStatusIcon() const
{
	// Return appropriate icon texture based on part status
	// Icons expected at /Game/UI/Icons/Parts/

	FString IconPath;

	if (PartData.bLocked)
	{
		IconPath = TEXT("/Game/UI/Icons/Parts/Icon_Locked");
	}
	else if (PartData.bEquipped)
	{
		IconPath = TEXT("/Game/UI/Icons/Parts/Icon_Equipped");
	}
	else if (PartData.bOwned)
	{
		IconPath = TEXT("/Game/UI/Icons/Parts/Icon_Owned");
	}
	else
	{
		// Available for purchase - show tier icon
		switch (PartData.Tier)
		{
		case EMGPartTier::Street:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Street");
			break;
		case EMGPartTier::Sport:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Sport");
			break;
		case EMGPartTier::Race:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Race");
			break;
		case EMGPartTier::Pro:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Pro");
			break;
		case EMGPartTier::Legendary:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Legendary");
			break;
		default:
			IconPath = TEXT("/Game/UI/Icons/Parts/Tier_Stock");
			break;
		}
	}

	if (!IconPath.IsEmpty())
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
	}

	return nullptr;
}

// ==========================================
// UMGStatBarWidget
// ==========================================

void UMGStatBarWidget::SetStatData(const FMGStatChange& InStatChange)
{
	StatChange = InStatChange;
	OnStatDataChanged();
}

void UMGStatBarWidget::SetShowPreview(bool bShow)
{
	if (bShowPreview != bShow)
	{
		bShowPreview = bShow;
		OnStatDataChanged();
	}
}

FLinearColor UMGStatBarWidget::GetChangeColor() const
{
	if (!bShowPreview)
	{
		return CurrentBarColor;
	}

	return StatChange.IsPositiveChange() ? PositiveChangeColor : NegativeChangeColor;
}

FText UMGStatBarWidget::GetChangeText() const
{
	if (!bShowPreview || FMath::IsNearlyEqual(StatChange.CurrentValue, StatChange.NewValue, 0.01f))
	{
		return FText::GetEmpty();
	}

	float Change = StatChange.GetChange();
	FString ChangeStr;

	// Format based on decimal places
	switch (StatChange.DecimalPlaces)
	{
	case 0:
		ChangeStr = FString::Printf(TEXT("%+d"), FMath::RoundToInt(Change));
		break;
	case 1:
		ChangeStr = FString::Printf(TEXT("%+.1f"), Change);
		break;
	case 2:
	default:
		ChangeStr = FString::Printf(TEXT("%+.2f"), Change);
		break;
	}

	return FText::FromString(ChangeStr);
}

float UMGStatBarWidget::GetCurrentFillPercent() const
{
	if (StatChange.MaxValue <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(StatChange.CurrentValue / StatChange.MaxValue, 0.0f, 1.0f);
}

float UMGStatBarWidget::GetPreviewFillPercent() const
{
	if (!bShowPreview || StatChange.MaxValue <= 0.0f)
	{
		return GetCurrentFillPercent();
	}

	return FMath::Clamp(StatChange.NewValue / StatChange.MaxValue, 0.0f, 1.0f);
}

// ==========================================
// UMGPerformanceIndexWidget
// ==========================================

void UMGPerformanceIndexWidget::SetCurrentPI(int32 InPI)
{
	CurrentPI = FMath::Clamp(InPI, 100, 999);
	OnPIChanged();
}

void UMGPerformanceIndexWidget::SetPreviewPI(int32 InPreviewPI)
{
	PreviewPI = FMath::Clamp(InPreviewPI, 100, 999);
	bHasPreview = true;
	OnPIChanged();
}

void UMGPerformanceIndexWidget::ClearPreview()
{
	bHasPreview = false;
	PreviewPI = CurrentPI;
	OnPIChanged();
}

FText UMGPerformanceIndexWidget::GetClassLetter(int32 PI) const
{
	EMGPerformanceClass Class = UMGStatCalculator::GetPerformanceClass(PI);

	switch (Class)
	{
	case EMGPerformanceClass::D:
		return NSLOCTEXT("MG", "Class_D", "D");
	case EMGPerformanceClass::C:
		return NSLOCTEXT("MG", "Class_C", "C");
	case EMGPerformanceClass::B:
		return NSLOCTEXT("MG", "Class_B", "B");
	case EMGPerformanceClass::A:
		return NSLOCTEXT("MG", "Class_A", "A");
	case EMGPerformanceClass::S:
		return NSLOCTEXT("MG", "Class_S", "S");
	case EMGPerformanceClass::X:
		return NSLOCTEXT("MG", "Class_X", "X");
	default:
		return NSLOCTEXT("MG", "Class_D", "D");
	}
}

FLinearColor UMGPerformanceIndexWidget::GetClassColor(int32 PI) const
{
	EMGPerformanceClass Class = UMGStatCalculator::GetPerformanceClass(PI);

	switch (Class)
	{
	case EMGPerformanceClass::D:
		return ClassD_Color;
	case EMGPerformanceClass::C:
		return ClassC_Color;
	case EMGPerformanceClass::B:
		return ClassB_Color;
	case EMGPerformanceClass::A:
		return ClassA_Color;
	case EMGPerformanceClass::S:
		return ClassS_Color;
	case EMGPerformanceClass::X:
		return ClassX_Color;
	default:
		return ClassD_Color;
	}
}

FText UMGPerformanceIndexWidget::GetPIText(int32 PI) const
{
	return FText::AsNumber(PI);
}

bool UMGPerformanceIndexWidget::IsClassChanging() const
{
	if (!bHasPreview)
	{
		return false;
	}

	EMGPerformanceClass CurrentClass = UMGStatCalculator::GetPerformanceClass(CurrentPI);
	EMGPerformanceClass PreviewClass = UMGStatCalculator::GetPerformanceClass(PreviewPI);

	return CurrentClass != PreviewClass;
}
