// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGCustomizationWidget.h"
#include "Vehicle/MGStatCalculator.h"
#include "Garage/MGGarageSubsystem.h"
#include "Customization/MGCustomizationSubsystem.h"
#include "Kismet/GameplayStatics.h"

UMGCustomizationWidget::UMGCustomizationWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize camera state
	CurrentCameraState.Location = FVector(-400.0f, 0.0f, 100.0f);
	CurrentCameraState.Rotation = FRotator(-10.0f, 0.0f, 0.0f);
	CurrentCameraState.FOV = 60.0f;
	TargetCameraState = CurrentCameraState;
}

void UMGCustomizationWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMGCustomizationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update camera interpolation
	if (bIsCameraInterpolating)
	{
		UpdateCameraInterpolation(InDeltaTime);
	}
}

FReply UMGCustomizationWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle gamepad navigation
	FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Gamepad_DPad_Up || Key == EKeys::W || Key == EKeys::Up)
	{
		Navigate(EMGUINavigationDirection::Up);
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_DPad_Down || Key == EKeys::S || Key == EKeys::Down)
	{
		Navigate(EMGUINavigationDirection::Down);
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_DPad_Left || Key == EKeys::A || Key == EKeys::Left)
	{
		Navigate(EMGUINavigationDirection::Left);
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_DPad_Right || Key == EKeys::D || Key == EKeys::Right)
	{
		Navigate(EMGUINavigationDirection::Right);
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		ConfirmSelection();
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape || Key == EKeys::BackSpace)
	{
		NavigateBack();
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Q)
	{
		Navigate(EMGUINavigationDirection::LeftShoulder);
		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_RightShoulder || Key == EKeys::E)
	{
		Navigate(EMGUINavigationDirection::RightShoulder);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UMGCustomizationWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UMGCustomizationWidget::InitializeWithVehicle(const FGuid& VehicleID)
{
	SelectedVehicleID = VehicleID;

	// Reset state
	CurrentMenuState = EMGCustomizationMenuState::MainMenu;
	MenuStateHistory.Empty();
	SelectedCategory = EMGCustomizationCategory::Engine;

	// Refresh parts list
	RefreshPartsList();

	// Update preview
	UpdateVehiclePreview();

	// Notify Blueprint
	OnVehicleDataReady();
}

// ==========================================
// NAVIGATION
// ==========================================

void UMGCustomizationWidget::Navigate(EMGUINavigationDirection Direction)
{
	if (bIsInTransition)
	{
		return;
	}

	HandleInputForState(Direction);
}

void UMGCustomizationWidget::NavigateBack()
{
	if (bIsInTransition)
	{
		return;
	}

	// Check if we have state history
	if (MenuStateHistory.Num() > 0)
	{
		EMGCustomizationMenuState PreviousState = MenuStateHistory.Pop();
		SetMenuState(PreviousState);
	}
	else if (CurrentMenuState != EMGCustomizationMenuState::MainMenu)
	{
		// Default back to main menu
		SetMenuState(EMGCustomizationMenuState::MainMenu);
	}
	else
	{
		// At main menu, broadcast cancel event
		OnCustomizationCanceled.Broadcast();
	}
}

void UMGCustomizationWidget::ConfirmSelection()
{
	if (bIsInTransition)
	{
		return;
	}

	switch (CurrentMenuState)
	{
	case EMGCustomizationMenuState::MainMenu:
		// Move to category select
		SetMenuState(EMGCustomizationMenuState::CategorySelect);
		break;

	case EMGCustomizationMenuState::CategorySelect:
		// Move to part select
		SetMenuState(EMGCustomizationMenuState::PartSelect);
		break;

	case EMGCustomizationMenuState::PartSelect:
		// Move to part details
		SetMenuState(EMGCustomizationMenuState::PartDetails);
		break;

	case EMGCustomizationMenuState::PartDetails:
		// Attempt to install or purchase
		if (SelectedPartData.bOwned)
		{
			InstallPart();
		}
		else if (!SelectedPartData.bLocked)
		{
			SetMenuState(EMGCustomizationMenuState::Checkout);
		}
		break;

	case EMGCustomizationMenuState::Checkout:
		// Purchase the part
		if (PurchasePart())
		{
			InstallPart();
		}
		break;

	case EMGCustomizationMenuState::TuningAdjust:
	case EMGCustomizationMenuState::PaintEditor:
	case EMGCustomizationMenuState::VinylEditor:
		// Confirm changes
		NavigateBack();
		break;

	default:
		break;
	}
}

void UMGCustomizationWidget::SetMenuState(EMGCustomizationMenuState NewState)
{
	if (CurrentMenuState == NewState)
	{
		return;
	}

	// Store previous state in history
	MenuStateHistory.Push(CurrentMenuState);

	// Keep history manageable
	if (MenuStateHistory.Num() > 10)
	{
		MenuStateHistory.RemoveAt(0);
	}

	EMGCustomizationMenuState OldState = CurrentMenuState;
	PreviousMenuState = OldState;
	CurrentMenuState = NewState;

	// Notify Blueprint of state exit/enter
	OnExitMenuState(OldState);
	OnEnterMenuState(NewState);

	// Play transition animation
	PlayMenuTransition(OldState, NewState);

	// Broadcast event
	OnMenuStateChanged.Broadcast(NewState);
}

// ==========================================
// CATEGORY MANAGEMENT
// ==========================================

TArray<FMGCategoryDisplayInfo> UMGCustomizationWidget::GetAllCategories() const
{
	TArray<FMGCategoryDisplayInfo> Categories;

	// Performance categories
	Categories.Add({EMGCustomizationCategory::Engine, NSLOCTEXT("MG", "Cat_Engine", "Engine"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Engine_Desc", "Engine internals and upgrades")});
	Categories.Add({EMGCustomizationCategory::ForcedInduction, NSLOCTEXT("MG", "Cat_FI", "Forced Induction"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_FI_Desc", "Turbochargers and superchargers")});
	Categories.Add({EMGCustomizationCategory::Drivetrain, NSLOCTEXT("MG", "Cat_Drive", "Drivetrain"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Drive_Desc", "Transmission and differentials")});
	Categories.Add({EMGCustomizationCategory::Suspension, NSLOCTEXT("MG", "Cat_Susp", "Suspension"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Susp_Desc", "Springs, dampers, and geometry")});
	Categories.Add({EMGCustomizationCategory::Brakes, NSLOCTEXT("MG", "Cat_Brake", "Brakes"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Brake_Desc", "Rotors, calipers, and lines")});
	Categories.Add({EMGCustomizationCategory::Wheels, NSLOCTEXT("MG", "Cat_Wheels", "Wheels"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Wheels_Desc", "Wheel selection and sizing")});
	Categories.Add({EMGCustomizationCategory::Tires, NSLOCTEXT("MG", "Cat_Tires", "Tires"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Tires_Desc", "Tire compounds and widths")});
	Categories.Add({EMGCustomizationCategory::Aero, NSLOCTEXT("MG", "Cat_Aero", "Aero"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Aero_Desc", "Wings, splitters, and diffusers")});
	Categories.Add({EMGCustomizationCategory::Weight, NSLOCTEXT("MG", "Cat_Weight", "Weight"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Weight_Desc", "Weight reduction options")});
	Categories.Add({EMGCustomizationCategory::Nitrous, NSLOCTEXT("MG", "Cat_NOS", "Nitrous"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_NOS_Desc", "Nitrous oxide systems")});

	// Visual categories
	Categories.Add({EMGCustomizationCategory::BodyKits, NSLOCTEXT("MG", "Cat_Body", "Body Kits"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Body_Desc", "Bumpers, skirts, and wide bodies")});
	Categories.Add({EMGCustomizationCategory::Paint, NSLOCTEXT("MG", "Cat_Paint", "Paint"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Paint_Desc", "Colors and finishes")});
	Categories.Add({EMGCustomizationCategory::Vinyl, NSLOCTEXT("MG", "Cat_Vinyl", "Vinyl"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Vinyl_Desc", "Decals and liveries")});
	Categories.Add({EMGCustomizationCategory::Interior, NSLOCTEXT("MG", "Cat_Interior", "Interior"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Interior_Desc", "Seats, gauges, and trim")});
	Categories.Add({EMGCustomizationCategory::Lights, NSLOCTEXT("MG", "Cat_Lights", "Lights"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Lights_Desc", "Headlights and taillights")});
	Categories.Add({EMGCustomizationCategory::Underglow, NSLOCTEXT("MG", "Cat_Glow", "Underglow"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Glow_Desc", "Neon and LED lighting")});
	Categories.Add({EMGCustomizationCategory::Window, NSLOCTEXT("MG", "Cat_Window", "Window Tint"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Window_Desc", "Window tinting options")});

	// Tuning categories
	Categories.Add({EMGCustomizationCategory::ECUTuning, NSLOCTEXT("MG", "Cat_ECU", "ECU Tuning"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_ECU_Desc", "Engine management tuning")});
	Categories.Add({EMGCustomizationCategory::TransmissionTuning, NSLOCTEXT("MG", "Cat_TransTune", "Gearing"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_TransTune_Desc", "Gear ratios and final drive")});
	Categories.Add({EMGCustomizationCategory::SuspensionTuning, NSLOCTEXT("MG", "Cat_SuspTune", "Suspension Tuning"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_SuspTune_Desc", "Spring rates and damping")});
	Categories.Add({EMGCustomizationCategory::AlignmentTuning, NSLOCTEXT("MG", "Cat_Align", "Alignment"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_Align_Desc", "Camber, toe, and caster")});
	Categories.Add({EMGCustomizationCategory::DifferentialTuning, NSLOCTEXT("MG", "Cat_DiffTune", "Differential"), nullptr, 0, 0, false, NSLOCTEXT("MG", "Cat_DiffTune_Desc", "LSD settings and bias")});

	return Categories;
}

TArray<FMGCategoryDisplayInfo> UMGCustomizationWidget::GetCategoriesForTab(int32 TabIndex) const
{
	TArray<FMGCategoryDisplayInfo> AllCategories = GetAllCategories();
	TArray<FMGCategoryDisplayInfo> FilteredCategories;

	for (const FMGCategoryDisplayInfo& Category : AllCategories)
	{
		int32 CatValue = static_cast<int32>(Category.Category);

		switch (TabIndex)
		{
		case 0: // Performance
			if (CatValue >= static_cast<int32>(EMGCustomizationCategory::Engine) &&
				CatValue <= static_cast<int32>(EMGCustomizationCategory::Nitrous))
			{
				FilteredCategories.Add(Category);
			}
			break;

		case 1: // Visual
			if (CatValue >= static_cast<int32>(EMGCustomizationCategory::BodyKits) &&
				CatValue <= static_cast<int32>(EMGCustomizationCategory::Window))
			{
				FilteredCategories.Add(Category);
			}
			break;

		case 2: // Tuning
			if (CatValue >= static_cast<int32>(EMGCustomizationCategory::ECUTuning) &&
				CatValue <= static_cast<int32>(EMGCustomizationCategory::DifferentialTuning))
			{
				FilteredCategories.Add(Category);
			}
			break;
		}
	}

	return FilteredCategories;
}

void UMGCustomizationWidget::SelectCategory(EMGCustomizationCategory Category)
{
	if (SelectedCategory != Category)
	{
		SelectedCategory = Category;
		RefreshPartsList();
		OnCategorySelected.Broadcast(Category);

		// Update camera for category
		FName PresetName = NAME_None;
		switch (Category)
		{
		case EMGCustomizationCategory::Engine:
		case EMGCustomizationCategory::ForcedInduction:
			PresetName = TEXT("Engine");
			break;
		case EMGCustomizationCategory::Suspension:
		case EMGCustomizationCategory::Brakes:
		case EMGCustomizationCategory::Wheels:
		case EMGCustomizationCategory::Tires:
			PresetName = TEXT("WheelFL");
			break;
		case EMGCustomizationCategory::Aero:
			PresetName = TEXT("Rear");
			break;
		case EMGCustomizationCategory::BodyKits:
		case EMGCustomizationCategory::Paint:
		case EMGCustomizationCategory::Vinyl:
			PresetName = TEXT("ThreeQuarter");
			break;
		case EMGCustomizationCategory::Interior:
			PresetName = TEXT("Interior");
			break;
		case EMGCustomizationCategory::Lights:
			PresetName = TEXT("Front");
			break;
		default:
			PresetName = TEXT("Default");
			break;
		}

		SetCameraPreset(PresetName);
	}
}

// ==========================================
// PART MANAGEMENT
// ==========================================

TArray<FMGUIPartData> UMGCustomizationWidget::GetPartsForSelectedCategory() const
{
	return CachedPartsList;
}

TArray<FMGUIPartData> UMGCustomizationWidget::GetFilteredParts(EMGPartFilter Filter, EMGPartSortMode SortMode) const
{
	TArray<FMGUIPartData> FilteredParts = CachedPartsList;

	// Apply filter
	switch (Filter)
	{
	case EMGPartFilter::Owned:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.bOwned; });
		break;
	case EMGPartFilter::Available:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return !Part.bOwned && !Part.bLocked; });
		break;
	case EMGPartFilter::Locked:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.bLocked; });
		break;
	case EMGPartFilter::Equipped:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.bEquipped; });
		break;
	case EMGPartFilter::TierStock:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Stock; });
		break;
	case EMGPartFilter::TierStreet:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Street; });
		break;
	case EMGPartFilter::TierSport:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Sport; });
		break;
	case EMGPartFilter::TierRace:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Race; });
		break;
	case EMGPartFilter::TierPro:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Pro; });
		break;
	case EMGPartFilter::TierLegendary:
		FilteredParts = FilteredParts.FilterByPredicate([](const FMGUIPartData& Part) { return Part.Tier == EMGPartTier::Legendary; });
		break;
	default:
		break;
	}

	// Apply sort
	switch (SortMode)
	{
	case EMGPartSortMode::NameAscending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return A.DisplayName.CompareTo(B.DisplayName) < 0; });
		break;
	case EMGPartSortMode::NameDescending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return A.DisplayName.CompareTo(B.DisplayName) > 0; });
		break;
	case EMGPartSortMode::PriceAscending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return A.Price < B.Price; });
		break;
	case EMGPartSortMode::PriceDescending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return A.Price > B.Price; });
		break;
	case EMGPartSortMode::TierAscending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return static_cast<int32>(A.Tier) < static_cast<int32>(B.Tier); });
		break;
	case EMGPartSortMode::TierDescending:
		FilteredParts.Sort([](const FMGUIPartData& A, const FMGUIPartData& B) { return static_cast<int32>(A.Tier) > static_cast<int32>(B.Tier); });
		break;
	default:
		break;
	}

	return FilteredParts;
}

void UMGCustomizationWidget::SelectPart(const FGuid& PartID)
{
	for (const FMGUIPartData& Part : CachedPartsList)
	{
		if (Part.PartID == PartID)
		{
			SelectedPartData = Part;
			CurrentComparison = CalculatePartComparison(PartID);

			OnPartSelected.Broadcast(SelectedPartData);
			OnStatsPreviewUpdated();
			PlayPartSelectionAnimation(SelectedPartData);

			// Update vehicle preview with selected part
			UpdateVehiclePreview();

			break;
		}
	}
}

bool UMGCustomizationWidget::PurchasePart()
{
	// Check if player can afford
	if (!CanAffordPart(SelectedPartData.PartID))
	{
		return false;
	}

	// Deduct credits through economy subsystem
	// For now, we track purchases locally - will integrate with save system
	PlayerCreditsCache -= SelectedPartData.Price;
	PurchasedPartIDs.Add(SelectedPartData.PartID);

	SelectedPartData.bOwned = true;

	OnPartPurchased.Broadcast(SelectedPartData);
	PlayPurchaseAnimation(SelectedPartData);

	RefreshPartsList();

	return true;
}

bool UMGCustomizationWidget::InstallPart()
{
	if (!SelectedPartData.bOwned && !SelectedPartData.bLocked)
	{
		return false;
	}

	// Install part through vehicle management subsystem
	// Track installed parts locally by category
	InstalledPartsByCategory.FindOrAdd(SelectedCategory) = SelectedPartData.PartID;

	SelectedPartData.bEquipped = true;

	OnPartInstalled.Broadcast(SelectedPartData);
	PlayInstallAnimation(SelectedPartData);

	RefreshPartsList();
	UpdateVehiclePreview();

	// Return to part select
	SetMenuState(EMGCustomizationMenuState::PartSelect);

	return true;
}

bool UMGCustomizationWidget::UninstallPart(EMGCustomizationCategory Category)
{
	// Revert to stock through customization subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
		{
			Customization->RevertToStock(CurrentVehicleID, Category);
			UE_LOG(LogTemp, Log, TEXT("CustomizationWidget: Reverted category %d to stock"), static_cast<int32>(Category));
		}
	}

	RefreshPartsList();
	UpdateVehiclePreview();

	return true;
}

// ==========================================
// FILTERING & SORTING
// ==========================================

void UMGCustomizationWidget::SetPartFilter(EMGPartFilter NewFilter)
{
	if (CurrentFilter != NewFilter)
	{
		CurrentFilter = NewFilter;
		OnPartListUpdated();
	}
}

void UMGCustomizationWidget::SetSortMode(EMGPartSortMode NewSortMode)
{
	if (CurrentSortMode != NewSortMode)
	{
		CurrentSortMode = NewSortMode;
		OnPartListUpdated();
	}
}

// ==========================================
// VEHICLE STATS
// ==========================================

FMGVehicleStats UMGCustomizationWidget::GetCurrentVehicleStats() const
{
	// Return cached stats or generate default stats for testing
	if (CachedCurrentStats.PerformanceIndex > 0.0f)
	{
		return CachedCurrentStats;
	}

	// Generate default mid-tier JDM stats for testing
	FMGVehicleStats Stats;
	Stats.Horsepower = 280.0f;
	Stats.Torque = 260.0f;
	Stats.WeightKG = 1350.0f;
	Stats.Redline = 7500;
	Stats.PowerToWeightRatio = 207.4f;
	Stats.GripFront = 0.92f;
	Stats.GripRear = 0.95f;
	Stats.HandlingRating = 72.0f;
	Stats.BrakingRating = 68.0f;
	Stats.ZeroTo60MPH = 5.2f;
	Stats.TopSpeedMPH = 155.0f;
	Stats.PerformanceIndex = 520.0f;
	Stats.PerformanceClass = EMGPerformanceClass::B;
	return Stats;
}

FMGVehicleStats UMGCustomizationWidget::GetPreviewVehicleStats() const
{
	// Calculate preview stats with selected part
	FMGVehicleStats Stats = GetCurrentVehicleStats();

	// Apply stat changes from selected part preview
	if (SelectedPartData.PartID.IsValid())
	{
		// Apply stat deltas based on part tier
		float TierMultiplier = 1.0f + (static_cast<float>(SelectedPartData.Tier) * 0.05f);

		switch (SelectedCategory)
		{
		case EMGCustomizationCategory::Engine:
		case EMGCustomizationCategory::ForcedInduction:
			Stats.Horsepower *= TierMultiplier;
			Stats.Torque *= TierMultiplier;
			break;

		case EMGCustomizationCategory::Suspension:
			Stats.HandlingRating = FMath::Min(100.0f, Stats.HandlingRating * TierMultiplier);
			break;

		case EMGCustomizationCategory::Brakes:
			Stats.BrakingRating = FMath::Min(100.0f, Stats.BrakingRating * TierMultiplier);
			break;

		case EMGCustomizationCategory::Tires:
			Stats.GripFront = FMath::Min(1.2f, Stats.GripFront * TierMultiplier);
			Stats.GripRear = FMath::Min(1.2f, Stats.GripRear * TierMultiplier);
			break;

		case EMGCustomizationCategory::Weight:
			Stats.WeightKG *= (2.0f - TierMultiplier); // Lighter is better
			break;

		default:
			break;
		}

		// Recalculate derived stats
		Stats.PowerToWeightRatio = Stats.Horsepower / (Stats.WeightKG * 2.20462f) * 1000.0f;
		Stats.PerformanceIndex = UMGStatCalculator::CalculatePerformanceIndex(Stats);
		Stats.PerformanceClass = UMGStatCalculator::GetPerformanceClass(Stats.PerformanceIndex);
	}

	return Stats;
}

int32 UMGCustomizationWidget::GetCurrentPI() const
{
	FMGVehicleStats Stats = GetCurrentVehicleStats();
	return UMGStatCalculator::CalculatePerformanceIndex(Stats);
}

EMGPerformanceClass UMGCustomizationWidget::GetCurrentPerformanceClass() const
{
	return UMGStatCalculator::GetPerformanceClass(GetCurrentPI());
}

int32 UMGCustomizationWidget::GetPreviewPI() const
{
	FMGVehicleStats Stats = GetPreviewVehicleStats();
	return UMGStatCalculator::CalculatePerformanceIndex(Stats);
}

// ==========================================
// TUNING
// ==========================================

TArray<FMGTuningSliderConfig> UMGCustomizationWidget::GetTuningSlidersForCategory() const
{
	TArray<FMGTuningSliderConfig> Sliders;

	switch (SelectedCategory)
	{
	case EMGCustomizationCategory::SuspensionTuning:
		Sliders.Add({TEXT("FrontSprings"), NSLOCTEXT("MG", "Tune_FSprings", "Front Springs"),
			NSLOCTEXT("MG", "Tune_FSprings_Desc", "Adjust front spring stiffness"),
			NSLOCTEXT("MG", "Soft", "Soft"), NSLOCTEXT("MG", "Stiff", "Stiff"),
			0.0f, 100.0f, 50.0f, 50.0f, 1.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		Sliders.Add({TEXT("RearSprings"), NSLOCTEXT("MG", "Tune_RSprings", "Rear Springs"),
			NSLOCTEXT("MG", "Tune_RSprings_Desc", "Adjust rear spring stiffness"),
			NSLOCTEXT("MG", "Soft", "Soft"), NSLOCTEXT("MG", "Stiff", "Stiff"),
			0.0f, 100.0f, 50.0f, 50.0f, 1.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		Sliders.Add({TEXT("FrontDampers"), NSLOCTEXT("MG", "Tune_FDampers", "Front Damping"),
			NSLOCTEXT("MG", "Tune_FDampers_Desc", "Adjust front damper strength"),
			NSLOCTEXT("MG", "Loose", "Loose"), NSLOCTEXT("MG", "Tight", "Tight"),
			0.0f, 100.0f, 50.0f, 50.0f, 1.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		Sliders.Add({TEXT("RearDampers"), NSLOCTEXT("MG", "Tune_RDampers", "Rear Damping"),
			NSLOCTEXT("MG", "Tune_RDampers_Desc", "Adjust rear damper strength"),
			NSLOCTEXT("MG", "Loose", "Loose"), NSLOCTEXT("MG", "Tight", "Tight"),
			0.0f, 100.0f, 50.0f, 50.0f, 1.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		Sliders.Add({TEXT("RideHeight"), NSLOCTEXT("MG", "Tune_Height", "Ride Height"),
			NSLOCTEXT("MG", "Tune_Height_Desc", "Adjust vehicle ride height"),
			NSLOCTEXT("MG", "Low", "Low"), NSLOCTEXT("MG", "High", "High"),
			-50.0f, 50.0f, 0.0f, 0.0f, 5.0f, NSLOCTEXT("MG", "Unit_MM", "mm"), 0});
		break;

	case EMGCustomizationCategory::AlignmentTuning:
		Sliders.Add({TEXT("FrontCamber"), NSLOCTEXT("MG", "Tune_FCamber", "Front Camber"),
			NSLOCTEXT("MG", "Tune_FCamber_Desc", "Adjust front wheel camber angle"),
			NSLOCTEXT("MG", "Negative", "-"), NSLOCTEXT("MG", "Positive", "+"),
			-5.0f, 2.0f, -1.0f, -1.0f, 0.1f, NSLOCTEXT("MG", "Unit_Deg", "deg"), 1});
		Sliders.Add({TEXT("RearCamber"), NSLOCTEXT("MG", "Tune_RCamber", "Rear Camber"),
			NSLOCTEXT("MG", "Tune_RCamber_Desc", "Adjust rear wheel camber angle"),
			NSLOCTEXT("MG", "Negative", "-"), NSLOCTEXT("MG", "Positive", "+"),
			-5.0f, 2.0f, -0.5f, -0.5f, 0.1f, NSLOCTEXT("MG", "Unit_Deg", "deg"), 1});
		Sliders.Add({TEXT("FrontToe"), NSLOCTEXT("MG", "Tune_FToe", "Front Toe"),
			NSLOCTEXT("MG", "Tune_FToe_Desc", "Adjust front wheel toe angle"),
			NSLOCTEXT("MG", "ToeOut", "Out"), NSLOCTEXT("MG", "ToeIn", "In"),
			-2.0f, 2.0f, 0.0f, 0.0f, 0.1f, NSLOCTEXT("MG", "Unit_Deg", "deg"), 1});
		Sliders.Add({TEXT("RearToe"), NSLOCTEXT("MG", "Tune_RToe", "Rear Toe"),
			NSLOCTEXT("MG", "Tune_RToe_Desc", "Adjust rear wheel toe angle"),
			NSLOCTEXT("MG", "ToeOut", "Out"), NSLOCTEXT("MG", "ToeIn", "In"),
			-2.0f, 2.0f, 0.2f, 0.2f, 0.1f, NSLOCTEXT("MG", "Unit_Deg", "deg"), 1});
		Sliders.Add({TEXT("Caster"), NSLOCTEXT("MG", "Tune_Caster", "Caster"),
			NSLOCTEXT("MG", "Tune_Caster_Desc", "Adjust front caster angle"),
			NSLOCTEXT("MG", "Less", "Less"), NSLOCTEXT("MG", "More", "More"),
			2.0f, 8.0f, 5.0f, 5.0f, 0.1f, NSLOCTEXT("MG", "Unit_Deg", "deg"), 1});
		break;

	case EMGCustomizationCategory::TransmissionTuning:
		Sliders.Add({TEXT("FinalDrive"), NSLOCTEXT("MG", "Tune_Final", "Final Drive"),
			NSLOCTEXT("MG", "Tune_Final_Desc", "Adjust final drive ratio"),
			NSLOCTEXT("MG", "Accel", "Acceleration"), NSLOCTEXT("MG", "TopSpeed", "Top Speed"),
			2.5f, 5.0f, 3.5f, 3.5f, 0.05f, NSLOCTEXT("MG", "Unit_Ratio", ":1"), 2});
		// Individual gear ratios would be added here
		break;

	case EMGCustomizationCategory::DifferentialTuning:
		Sliders.Add({TEXT("DiffAccel"), NSLOCTEXT("MG", "Tune_DiffAccel", "Accel Lock"),
			NSLOCTEXT("MG", "Tune_DiffAccel_Desc", "Differential lock under acceleration"),
			NSLOCTEXT("MG", "Open", "Open"), NSLOCTEXT("MG", "Locked", "Locked"),
			0.0f, 100.0f, 30.0f, 30.0f, 5.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		Sliders.Add({TEXT("DiffDecel"), NSLOCTEXT("MG", "Tune_DiffDecel", "Decel Lock"),
			NSLOCTEXT("MG", "Tune_DiffDecel_Desc", "Differential lock under deceleration"),
			NSLOCTEXT("MG", "Open", "Open"), NSLOCTEXT("MG", "Locked", "Locked"),
			0.0f, 100.0f, 20.0f, 20.0f, 5.0f, NSLOCTEXT("MG", "Unit_Percent", "%"), 0});
		break;

	default:
		break;
	}

	return Sliders;
}

void UMGCustomizationWidget::SetTuningValue(FName SliderID, float Value)
{
	// Apply tuning value through customization subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
		{
			Customization->SetTuningParameter(CurrentVehicleID, SliderID, Value);
		}
	}
	// Store locally for preview
	TuningValues.Add(SliderID, Value);
	OnStatsPreviewUpdated();
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::ResetTuningValue(FName SliderID)
{
	// Reset specific tuning slider to default (0.5 = center)
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
		{
			Customization->SetTuningParameter(CurrentVehicleID, SliderID, 0.5f);
		}
	}
	TuningValues.Add(SliderID, 0.5f);
	OnStatsPreviewUpdated();
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::ResetCategoryTuning()
{
	// Reset all tuning in current category
	TArray<FMGTuningSliderConfig> Sliders = GetTuningSlidersForCategory();
	for (const FMGTuningSliderConfig& Slider : Sliders)
	{
		ResetTuningValue(Slider.SliderID);
	}
	OnStatsPreviewUpdated();
	UpdateVehiclePreview();
}

// ==========================================
// PAINT & VISUALS
// ==========================================

TArray<FMGPaintColorData> UMGCustomizationWidget::GetAvailablePaintColors() const
{
	TArray<FMGPaintColorData> Colors;

	// Basic colors (free)
	Colors.Add({NSLOCTEXT("MG", "Color_White", "Pure White"), FLinearColor::White, EMGPaintFinish::Gloss, 0, false, true});
	Colors.Add({NSLOCTEXT("MG", "Color_Black", "Midnight Black"), FLinearColor::Black, EMGPaintFinish::Gloss, 0, false, true});
	Colors.Add({NSLOCTEXT("MG", "Color_Red", "Racing Red"), FLinearColor::Red, EMGPaintFinish::Gloss, 0, false, true});
	Colors.Add({NSLOCTEXT("MG", "Color_Blue", "Electric Blue"), FLinearColor::Blue, EMGPaintFinish::Gloss, 0, false, true});
	Colors.Add({NSLOCTEXT("MG", "Color_Yellow", "Solar Yellow"), FLinearColor::Yellow, EMGPaintFinish::Gloss, 0, false, true});

	// Premium metallics
	Colors.Add({NSLOCTEXT("MG", "Color_ChromeSilver", "Chrome Silver"), FLinearColor(0.8f, 0.8f, 0.85f), EMGPaintFinish::Chrome, 5000, true, false, FLinearColor::White, 1.0f, 0.9f});
	Colors.Add({NSLOCTEXT("MG", "Color_MidnightPurple", "Midnight Purple"), FLinearColor(0.3f, 0.1f, 0.4f), EMGPaintFinish::Pearl, 7500, true, false, FLinearColor(0.5f, 0.2f, 0.6f), 0.6f, 0.7f});
	Colors.Add({NSLOCTEXT("MG", "Color_BaySide", "Bayside Blue"), FLinearColor(0.0f, 0.4f, 0.8f), EMGPaintFinish::Metallic, 5000, true, false, FLinearColor(0.3f, 0.5f, 1.0f), 0.5f, 0.6f});

	return Colors;
}

void UMGCustomizationWidget::SetPaintColor(int32 ZoneIndex, const FMGPaintColorData& ColorData)
{
	// Apply paint color through customization subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
		{
			Customization->SetPaintColor(CurrentVehicleID, ZoneIndex, ColorData.Color, ColorData.Finish);
		}
	}
	// Cache locally for display
	PaintZoneColors.Add(ZoneIndex, ColorData);
	UpdateVehiclePreview();
}

FMGPaintColorData UMGCustomizationWidget::GetPaintForZone(int32 ZoneIndex) const
{
	// Return cached paint data or query from customization subsystem
	if (const FMGPaintColorData* CachedColor = PaintZoneColors.Find(ZoneIndex))
	{
		return *CachedColor;
	}
	// Default white paint
	FMGPaintColorData DefaultPaint;
	DefaultPaint.Color = FLinearColor::White;
	DefaultPaint.Finish = EMGPaintFinish::Gloss;
	return DefaultPaint;
}

void UMGCustomizationWidget::OpenColorPicker(int32 ZoneIndex)
{
	// Set state to indicate we're editing a paint zone
	EditingPaintZone = ZoneIndex;
	SetMenuState(EMGCustomizationMenuState::ColorPicker);
}

// ==========================================
// VINYL/DECALS
// ==========================================

TArray<FMGVinylPlacement> UMGCustomizationWidget::GetVinylPlacements() const
{
	// Return cached vinyl placements
	return VinylPlacements;
}

void UMGCustomizationWidget::AddVinyl(const FGuid& VinylAssetID)
{
	// Add vinyl through customization subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
		{
			FMGVinylPlacement NewPlacement;
			NewPlacement.VinylAssetID = VinylAssetID;
			NewPlacement.Position = FVector2D(0.5f, 0.5f);
			NewPlacement.Scale = FVector2D(1.0f, 1.0f);
			NewPlacement.Rotation = 0.0f;
			Customization->AddVinyl(CurrentVehicleID, NewPlacement);
			VinylPlacements.Add(NewPlacement);
		}
	}
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::UpdateVinylPlacement(int32 VinylIndex, const FMGVinylPlacement& Placement)
{
	// Update vinyl placement through customization subsystem
	if (VinylIndex >= 0 && VinylIndex < VinylPlacements.Num())
	{
		VinylPlacements[VinylIndex] = Placement;
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
			{
				Customization->UpdateVinyl(CurrentVehicleID, VinylIndex, Placement);
			}
		}
	}
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::RemoveVinyl(int32 VinylIndex)
{
	// Remove vinyl through customization subsystem
	if (VinylIndex >= 0 && VinylIndex < VinylPlacements.Num())
	{
		VinylPlacements.RemoveAt(VinylIndex);
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
			{
				Customization->RemoveVinyl(CurrentVehicleID, VinylIndex);
			}
		}
	}
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::EnterVinylEditMode(int32 VinylIndex)
{
	EditingVinylIndex = VinylIndex;
	// Cache current state for potential revert
	if (VinylIndex >= 0 && VinylIndex < VinylPlacements.Num())
	{
		CachedVinylPlacement = VinylPlacements[VinylIndex];
	}
	SetMenuState(EMGCustomizationMenuState::VinylEditor);
	SetCameraPreset(TEXT("VinylEdit"));
}

void UMGCustomizationWidget::ExitVinylEditMode(bool bSaveChanges)
{
	if (!bSaveChanges && EditingVinylIndex >= 0 && EditingVinylIndex < VinylPlacements.Num())
	{
		// Revert vinyl changes
		VinylPlacements[EditingVinylIndex] = CachedVinylPlacement;
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UMGCustomizationSubsystem* Customization = GI->GetSubsystem<UMGCustomizationSubsystem>())
			{
				Customization->UpdateVinyl(CurrentVehicleID, EditingVinylIndex, CachedVinylPlacement);
			}
		}
	}
	EditingVinylIndex = -1;
	NavigateBack();
}

// ==========================================
// CAMERA CONTROL
// ==========================================

void UMGCustomizationWidget::SetCameraPreset(FName PresetName)
{
	// Define camera presets
	if (PresetName == TEXT("Default") || PresetName == TEXT("ThreeQuarter"))
	{
		TargetCameraState.Location = FVector(-400.0f, 200.0f, 100.0f);
		TargetCameraState.Rotation = FRotator(-10.0f, -25.0f, 0.0f);
		TargetCameraState.FOV = 60.0f;
	}
	else if (PresetName == TEXT("Front"))
	{
		TargetCameraState.Location = FVector(-350.0f, 0.0f, 50.0f);
		TargetCameraState.Rotation = FRotator(-5.0f, 0.0f, 0.0f);
		TargetCameraState.FOV = 50.0f;
	}
	else if (PresetName == TEXT("Rear"))
	{
		TargetCameraState.Location = FVector(350.0f, 0.0f, 100.0f);
		TargetCameraState.Rotation = FRotator(-10.0f, 180.0f, 0.0f);
		TargetCameraState.FOV = 55.0f;
	}
	else if (PresetName == TEXT("Side"))
	{
		TargetCameraState.Location = FVector(0.0f, -400.0f, 80.0f);
		TargetCameraState.Rotation = FRotator(-5.0f, 90.0f, 0.0f);
		TargetCameraState.FOV = 60.0f;
	}
	else if (PresetName == TEXT("Engine"))
	{
		TargetCameraState.Location = FVector(-150.0f, 100.0f, 150.0f);
		TargetCameraState.Rotation = FRotator(-30.0f, -30.0f, 0.0f);
		TargetCameraState.FOV = 45.0f;
	}
	else if (PresetName == TEXT("WheelFL"))
	{
		TargetCameraState.Location = FVector(-200.0f, -150.0f, 30.0f);
		TargetCameraState.Rotation = FRotator(0.0f, 35.0f, 0.0f);
		TargetCameraState.FOV = 40.0f;
	}
	else if (PresetName == TEXT("Interior"))
	{
		TargetCameraState.Location = FVector(30.0f, 50.0f, 100.0f);
		TargetCameraState.Rotation = FRotator(-15.0f, -120.0f, 0.0f);
		TargetCameraState.FOV = 70.0f;
	}
	else if (PresetName == TEXT("VinylEdit"))
	{
		TargetCameraState.Location = FVector(0.0f, -300.0f, 80.0f);
		TargetCameraState.Rotation = FRotator(-5.0f, 90.0f, 0.0f);
		TargetCameraState.FOV = 50.0f;
	}

	// Start interpolation
	bIsCameraInterpolating = true;
	CameraInterpAlpha = 0.0f;
}

void UMGCustomizationWidget::RotateCameraOrbit(float YawDelta, float PitchDelta)
{
	if (!CurrentCameraState.bAllowOrbit)
	{
		return;
	}

	CurrentCameraState.Rotation.Yaw += YawDelta;
	CurrentCameraState.Rotation.Pitch = FMath::Clamp(CurrentCameraState.Rotation.Pitch + PitchDelta, -60.0f, 30.0f);

	TargetCameraState = CurrentCameraState;
}

void UMGCustomizationWidget::ZoomCamera(float ZoomDelta)
{
	if (!CurrentCameraState.bAllowZoom)
	{
		return;
	}

	// Adjust FOV for zoom effect
	CurrentCameraState.FOV = FMath::Clamp(CurrentCameraState.FOV + ZoomDelta, 30.0f, 90.0f);
	TargetCameraState = CurrentCameraState;
}

void UMGCustomizationWidget::ResetCamera()
{
	SetCameraPreset(TEXT("Default"));
}

// ==========================================
// PLAYER INFO
// ==========================================

int64 UMGCustomizationWidget::GetPlayerCredits() const
{
	// Return cached credits (starts at 50000 for testing, updated by purchases)
	return PlayerCreditsCache;
}

bool UMGCustomizationWidget::CanAffordPart(const FGuid& PartID) const
{
	for (const FMGUIPartData& Part : CachedPartsList)
	{
		if (Part.PartID == PartID)
		{
			return GetPlayerCredits() >= Part.Price;
		}
	}
	return false;
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGCustomizationWidget::RefreshPartsList()
{
	CachedPartsList.Empty();

	// Generate test parts based on selected category
	// This enables gameplay testing without requiring actual part assets

	struct FPartTemplate
	{
		FString Name;
		FString Manufacturer;
		EMGPartTier Tier;
		int32 BasePrice;
	};

	TArray<FPartTemplate> Templates;

	switch (SelectedCategory)
	{
	case EMGCustomizationCategory::Engine:
		Templates.Add({TEXT("Stage 1 ECU Tune"), TEXT("MG Tuning"), EMGPartTier::Street, 1500});
		Templates.Add({TEXT("Cold Air Intake"), TEXT("Injen"), EMGPartTier::Street, 350});
		Templates.Add({TEXT("Performance Camshafts"), TEXT("Brian Crower"), EMGPartTier::Sport, 2800});
		Templates.Add({TEXT("Forged Internals Kit"), TEXT("Eagle"), EMGPartTier::Race, 4500});
		Templates.Add({TEXT("Stroker Kit"), TEXT("Tomei"), EMGPartTier::Pro, 8000});
		break;

	case EMGCustomizationCategory::ForcedInduction:
		Templates.Add({TEXT("Bolt-On Turbo Kit"), TEXT("GReddy"), EMGPartTier::Sport, 3500});
		Templates.Add({TEXT("Twin Turbo Setup"), TEXT("HKS"), EMGPartTier::Race, 7500});
		Templates.Add({TEXT("Supercharger Kit"), TEXT("Vortech"), EMGPartTier::Race, 6000});
		Templates.Add({TEXT("Big Single Turbo"), TEXT("Garrett"), EMGPartTier::Pro, 5500});
		break;

	case EMGCustomizationCategory::Suspension:
		Templates.Add({TEXT("Lowering Springs"), TEXT("Eibach"), EMGPartTier::Street, 400});
		Templates.Add({TEXT("Coilover Kit"), TEXT("BC Racing"), EMGPartTier::Sport, 1200});
		Templates.Add({TEXT("Adjustable Coilovers"), TEXT("KW"), EMGPartTier::Race, 2800});
		Templates.Add({TEXT("Competition Coilovers"), TEXT("Ohlins"), EMGPartTier::Pro, 4500});
		break;

	case EMGCustomizationCategory::Brakes:
		Templates.Add({TEXT("Performance Pads"), TEXT("Hawk"), EMGPartTier::Street, 250});
		Templates.Add({TEXT("Slotted Rotors"), TEXT("StopTech"), EMGPartTier::Sport, 600});
		Templates.Add({TEXT("Big Brake Kit - 4 Piston"), TEXT("Brembo"), EMGPartTier::Race, 3200});
		Templates.Add({TEXT("Big Brake Kit - 6 Piston"), TEXT("AP Racing"), EMGPartTier::Pro, 5500});
		break;

	case EMGCustomizationCategory::Wheels:
		Templates.Add({TEXT("17x8 Alloy Wheels"), TEXT("Enkei"), EMGPartTier::Street, 800});
		Templates.Add({TEXT("18x9 Forged Wheels"), TEXT("Volk Racing"), EMGPartTier::Sport, 2400});
		Templates.Add({TEXT("18x10 Lightweight Forged"), TEXT("BBS"), EMGPartTier::Race, 3800});
		Templates.Add({TEXT("19x11 Carbon Wheels"), TEXT("Carbon Revolution"), EMGPartTier::Legendary, 12000});
		break;

	case EMGCustomizationCategory::Tires:
		Templates.Add({TEXT("Sport Tires"), TEXT("Michelin"), EMGPartTier::Street, 600});
		Templates.Add({TEXT("Performance Tires"), TEXT("Bridgestone"), EMGPartTier::Sport, 900});
		Templates.Add({TEXT("Semi-Slick R-Compound"), TEXT("Toyo"), EMGPartTier::Race, 1400});
		Templates.Add({TEXT("Full Slicks"), TEXT("Pirelli"), EMGPartTier::Pro, 2000});
		break;

	case EMGCustomizationCategory::Aero:
		Templates.Add({TEXT("Front Lip Spoiler"), TEXT("Vertex"), EMGPartTier::Street, 400});
		Templates.Add({TEXT("Rear Wing"), TEXT("APR"), EMGPartTier::Sport, 900});
		Templates.Add({TEXT("Full Aero Kit"), TEXT("Varis"), EMGPartTier::Race, 3500});
		Templates.Add({TEXT("GT Wing + Splitter"), TEXT("Voltex"), EMGPartTier::Pro, 5500});
		break;

	case EMGCustomizationCategory::Nitrous:
		Templates.Add({TEXT("50 Shot Dry Kit"), TEXT("NOS"), EMGPartTier::Street, 800});
		Templates.Add({TEXT("100 Shot Wet Kit"), TEXT("Nitrous Express"), EMGPartTier::Sport, 1500});
		Templates.Add({TEXT("150 Shot Progressive"), TEXT("ZEX"), EMGPartTier::Race, 2200});
		Templates.Add({TEXT("200 Shot Direct Port"), TEXT("Nitrous Outlet"), EMGPartTier::Pro, 3500});
		break;

	default:
		// Generic parts for other categories
		Templates.Add({TEXT("Street Upgrade"), TEXT("MG Parts"), EMGPartTier::Street, 500});
		Templates.Add({TEXT("Sport Upgrade"), TEXT("MG Parts"), EMGPartTier::Sport, 1200});
		Templates.Add({TEXT("Race Upgrade"), TEXT("MG Parts"), EMGPartTier::Race, 2500});
		break;
	}

	// Generate part data from templates
	FGuid CurrentEquipped = InstalledPartsByCategory.FindRef(SelectedCategory);

	for (int32 i = 0; i < Templates.Num(); ++i)
	{
		const FPartTemplate& Template = Templates[i];

		FMGUIPartData PartData;
		PartData.PartID = FGuid::NewGuid();
		PartData.DisplayName = FText::FromString(Template.Name);
		PartData.Description = FText::FromString(FString::Printf(TEXT("High quality %s from %s"),
			*Template.Name, *Template.Manufacturer));
		PartData.Manufacturer = FText::FromString(Template.Manufacturer);
		PartData.Tier = Template.Tier;
		PartData.Price = Template.BasePrice;
		PartData.bOwned = PurchasedPartIDs.Contains(PartData.PartID) || (Template.Tier == EMGPartTier::Stock);
		PartData.bEquipped = (PartData.PartID == CurrentEquipped);
		PartData.bLocked = (Template.Tier == EMGPartTier::Legendary && GetPlayerCredits() < 10000);

		CachedPartsList.Add(PartData);
	}

	OnPartListUpdated();
}

FMGPartComparison UMGCustomizationWidget::CalculatePartComparison(const FGuid& PartID) const
{
	FMGPartComparison Comparison;

	// Get current and preview stats for comparison
	FMGVehicleStats CurrentStats = GetCurrentVehicleStats();
	FMGVehicleStats PreviewStats = GetPreviewVehicleStats();

	// Calculate stat deltas
	Comparison.HorsepowerDelta = PreviewStats.Horsepower - CurrentStats.Horsepower;
	Comparison.TorqueDelta = PreviewStats.Torque - CurrentStats.Torque;
	Comparison.WeightDelta = PreviewStats.WeightKG - CurrentStats.WeightKG;
	Comparison.HandlingDelta = PreviewStats.HandlingRating - CurrentStats.HandlingRating;
	Comparison.BrakingDelta = PreviewStats.BrakingRating - CurrentStats.BrakingRating;
	Comparison.GripDelta = (PreviewStats.GripFront + PreviewStats.GripRear) / 2.0f - (CurrentStats.GripFront + CurrentStats.GripRear) / 2.0f;
	Comparison.TopSpeedDelta = PreviewStats.TopSpeedMPH - CurrentStats.TopSpeedMPH;
	Comparison.AccelerationDelta = CurrentStats.ZeroTo60MPH - PreviewStats.ZeroTo60MPH; // Lower is better
	Comparison.PIDelta = static_cast<int32>(PreviewStats.PerformanceIndex - CurrentStats.PerformanceIndex);

	return Comparison;
}

void UMGCustomizationWidget::UpdateVehiclePreview()
{
	// Notify the garage/preview system to update the 3D vehicle display
	OnVehiclePreviewRequested.Broadcast(CurrentVehicleID, SelectedPartData.PartID);
}

TArray<FMGCustomizationInputBinding> UMGCustomizationWidget::GetInputBindingsForState() const
{
	TArray<FMGCustomizationInputBinding> Bindings;

	// Common bindings
	Bindings.Add({NSLOCTEXT("MG", "Input_Navigate", "Navigate"), EKeys::Gamepad_DPad_Up, EKeys::W, nullptr, nullptr});
	Bindings.Add({NSLOCTEXT("MG", "Input_Select", "Select"), EKeys::Gamepad_FaceButton_Bottom, EKeys::Enter, nullptr, nullptr});
	Bindings.Add({NSLOCTEXT("MG", "Input_Back", "Back"), EKeys::Gamepad_FaceButton_Right, EKeys::Escape, nullptr, nullptr});

	// State-specific bindings
	switch (CurrentMenuState)
	{
	case EMGCustomizationMenuState::PartSelect:
		Bindings.Add({NSLOCTEXT("MG", "Input_Filter", "Filter"), EKeys::Gamepad_FaceButton_Top, EKeys::F, nullptr, nullptr});
		Bindings.Add({NSLOCTEXT("MG", "Input_Sort", "Sort"), EKeys::Gamepad_FaceButton_Left, EKeys::S, nullptr, nullptr});
		break;

	case EMGCustomizationMenuState::TuningAdjust:
		Bindings.Add({NSLOCTEXT("MG", "Input_Adjust", "Adjust"), EKeys::Gamepad_LeftThumbstick, EKeys::Left, nullptr, nullptr});
		Bindings.Add({NSLOCTEXT("MG", "Input_Reset", "Reset"), EKeys::Gamepad_FaceButton_Top, EKeys::R, nullptr, nullptr});
		break;

	case EMGCustomizationMenuState::VinylEditor:
		Bindings.Add({NSLOCTEXT("MG", "Input_Move", "Move"), EKeys::Gamepad_LeftThumbstick, EKeys::W, nullptr, nullptr});
		Bindings.Add({NSLOCTEXT("MG", "Input_Rotate", "Rotate"), EKeys::Gamepad_RightThumbstick, EKeys::Q, nullptr, nullptr});
		Bindings.Add({NSLOCTEXT("MG", "Input_Scale", "Scale"), EKeys::Gamepad_LeftTrigger, EKeys::Z, nullptr, nullptr});
		break;

	default:
		break;
	}

	return Bindings;
}

void UMGCustomizationWidget::HandleInputForState(EMGUINavigationDirection Direction)
{
	switch (CurrentMenuState)
	{
	case EMGCustomizationMenuState::MainMenu:
	case EMGCustomizationMenuState::CategorySelect:
	case EMGCustomizationMenuState::PartSelect:
	case EMGCustomizationMenuState::PartDetails:
		// Standard list navigation handled by Blueprint
		break;

	case EMGCustomizationMenuState::TuningAdjust:
		// Slider adjustment
		if (Direction == EMGUINavigationDirection::Left)
		{
			// Decrease value
		}
		else if (Direction == EMGUINavigationDirection::Right)
		{
			// Increase value
		}
		break;

	case EMGCustomizationMenuState::VinylEditor:
		// Vinyl positioning
		break;

	default:
		break;
	}
}

void UMGCustomizationWidget::UpdateCameraInterpolation(float DeltaTime)
{
	if (!bIsCameraInterpolating)
	{
		return;
	}

	const float InterpSpeed = 4.0f;
	CameraInterpAlpha += DeltaTime * InterpSpeed;

	if (CameraInterpAlpha >= 1.0f)
	{
		CameraInterpAlpha = 1.0f;
		bIsCameraInterpolating = false;
		CurrentCameraState = TargetCameraState;
	}
	else
	{
		// Smooth interpolation with ease-out
		float Alpha = 1.0f - FMath::Pow(1.0f - CameraInterpAlpha, 3.0f);

		CurrentCameraState.Location = FMath::Lerp(CurrentCameraState.Location, TargetCameraState.Location, Alpha);
		CurrentCameraState.Rotation = FMath::Lerp(CurrentCameraState.Rotation, TargetCameraState.Rotation, Alpha);
		CurrentCameraState.FOV = FMath::Lerp(CurrentCameraState.FOV, TargetCameraState.FOV, Alpha);
	}
}
