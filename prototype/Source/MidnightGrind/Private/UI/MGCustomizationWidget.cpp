// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGCustomizationWidget.h"
#include "Vehicle/MGStatCalculator.h"
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

	// TODO: Deduct credits through economy subsystem
	// TODO: Add part to player inventory

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

	// TODO: Install part through vehicle management subsystem

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
	// TODO: Revert to stock through vehicle management subsystem

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
	// TODO: Get from vehicle management subsystem
	FMGVehicleStats Stats;
	return Stats;
}

FMGVehicleStats UMGCustomizationWidget::GetPreviewVehicleStats() const
{
	// TODO: Calculate preview stats with selected part
	FMGVehicleStats Stats = GetCurrentVehicleStats();
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
	// TODO: Apply tuning value to vehicle configuration
	OnStatsPreviewUpdated();
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::ResetTuningValue(FName SliderID)
{
	// TODO: Reset specific slider to default
	OnStatsPreviewUpdated();
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::ResetCategoryTuning()
{
	// TODO: Reset all tuning in current category
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
	// TODO: Apply paint to vehicle material
	UpdateVehiclePreview();
}

FMGPaintColorData UMGCustomizationWidget::GetPaintForZone(int32 ZoneIndex) const
{
	// TODO: Get current paint from vehicle configuration
	return FMGPaintColorData();
}

void UMGCustomizationWidget::OpenColorPicker(int32 ZoneIndex)
{
	// TODO: Open custom color picker widget
}

// ==========================================
// VINYL/DECALS
// ==========================================

TArray<FMGVinylPlacement> UMGCustomizationWidget::GetVinylPlacements() const
{
	// TODO: Get from vehicle configuration
	return TArray<FMGVinylPlacement>();
}

void UMGCustomizationWidget::AddVinyl(const FGuid& VinylAssetID)
{
	// TODO: Add vinyl to vehicle
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::UpdateVinylPlacement(int32 VinylIndex, const FMGVinylPlacement& Placement)
{
	// TODO: Update vinyl placement
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::RemoveVinyl(int32 VinylIndex)
{
	// TODO: Remove vinyl from vehicle
	UpdateVehiclePreview();
}

void UMGCustomizationWidget::EnterVinylEditMode(int32 VinylIndex)
{
	SetMenuState(EMGCustomizationMenuState::VinylEditor);
	SetCameraPreset(TEXT("VinylEdit"));
}

void UMGCustomizationWidget::ExitVinylEditMode(bool bSaveChanges)
{
	if (!bSaveChanges)
	{
		// TODO: Revert vinyl changes
	}
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
	// TODO: Get from player state/save system
	return 50000;
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

	// TODO: Populate from part database based on selected category and vehicle compatibility

	OnPartListUpdated();
}

FMGPartComparison UMGCustomizationWidget::CalculatePartComparison(const FGuid& PartID) const
{
	FMGPartComparison Comparison;

	// TODO: Calculate full stat comparison between current and selected part

	return Comparison;
}

void UMGCustomizationWidget::UpdateVehiclePreview()
{
	// TODO: Update 3D vehicle preview in garage scene
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
