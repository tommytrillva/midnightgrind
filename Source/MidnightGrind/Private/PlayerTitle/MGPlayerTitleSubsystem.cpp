// Copyright Midnight Grind. All Rights Reserved.

#include "PlayerTitle/MGPlayerTitleSubsystem.h"

void UMGPlayerTitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultTitles();
	InitializeDefaultBannerElements();
	InitializeDefaultNameplates();

	LoadTitleData();
}

void UMGPlayerTitleSubsystem::Deinitialize()
{
	SaveTitleData();
	Super::Deinitialize();
}

bool UMGPlayerTitleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

TArray<FMGPlayerTitle> UMGPlayerTitleSubsystem::GetAllTitles() const
{
	TArray<FMGPlayerTitle> Result;
	for (const auto& Pair : AllTitles)
	{
		Result.Add(Pair.Value);
	}

	Result.Sort([](const FMGPlayerTitle& A, const FMGPlayerTitle& B)
	{
		if ((int32)A.Rarity != (int32)B.Rarity)
		{
			return (int32)A.Rarity > (int32)B.Rarity;
		}
		return A.SortOrder < B.SortOrder;
	});

	return Result;
}

TArray<FMGPlayerTitle> UMGPlayerTitleSubsystem::GetUnlockedTitles() const
{
	TArray<FMGPlayerTitle> Result;
	for (const auto& Pair : AllTitles)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGPlayerTitle> UMGPlayerTitleSubsystem::GetTitlesByCategory(EMGTitleCategory Category) const
{
	TArray<FMGPlayerTitle> Result;
	for (const auto& Pair : AllTitles)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGPlayerTitle> UMGPlayerTitleSubsystem::GetTitlesByRarity(EMGTitleRarity Rarity) const
{
	TArray<FMGPlayerTitle> Result;
	for (const auto& Pair : AllTitles)
	{
		if (Pair.Value.Rarity == Rarity)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGPlayerTitle UMGPlayerTitleSubsystem::GetTitle(FName TitleID) const
{
	const FMGPlayerTitle* Title = AllTitles.Find(TitleID);
	return Title ? *Title : FMGPlayerTitle();
}

FMGPlayerTitle UMGPlayerTitleSubsystem::GetActiveTitle() const
{
	return GetTitle(CurrentProfile.ActiveTitleID);
}

bool UMGPlayerTitleSubsystem::IsTitleUnlocked(FName TitleID) const
{
	const FMGPlayerTitle* Title = AllTitles.Find(TitleID);
	return Title && Title->bIsUnlocked;
}

bool UMGPlayerTitleSubsystem::EquipTitle(FName TitleID)
{
	if (!IsTitleUnlocked(TitleID))
	{
		return false;
	}

	CurrentProfile.ActiveTitleID = TitleID;
	SaveTitleData();

	OnTitleEquipped.Broadcast(TitleID);
	OnProfileUpdated.Broadcast();

	return true;
}

void UMGPlayerTitleSubsystem::UnequipTitle()
{
	CurrentProfile.ActiveTitleID = FName();
	SaveTitleData();

	OnTitleEquipped.Broadcast(FName());
	OnProfileUpdated.Broadcast();
}

bool UMGPlayerTitleSubsystem::UnlockTitle(FName TitleID)
{
	FMGPlayerTitle* Title = AllTitles.Find(TitleID);
	if (!Title)
	{
		return false;
	}

	if (Title->bIsUnlocked)
	{
		return false;
	}

	Title->bIsUnlocked = true;
	Title->UnlockedAt = FDateTime::UtcNow();

	SaveTitleData();

	OnTitleUnlocked.Broadcast(*Title);

	return true;
}

TArray<FMGBannerElement> UMGPlayerTitleSubsystem::GetAllBannerElements() const
{
	TArray<FMGBannerElement> Result;
	for (const auto& Pair : AllBannerElements)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TArray<FMGBannerElement> UMGPlayerTitleSubsystem::GetUnlockedBannerElements() const
{
	TArray<FMGBannerElement> Result;
	for (const auto& Pair : AllBannerElements)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGBannerElement> UMGPlayerTitleSubsystem::GetBannerElementsBySlot(EMGBannerSlot Slot) const
{
	TArray<FMGBannerElement> Result;
	for (const auto& Pair : AllBannerElements)
	{
		if (Pair.Value.Slot == Slot)
		{
			Result.Add(Pair.Value);
		}
	}

	Result.Sort([](const FMGBannerElement& A, const FMGBannerElement& B)
	{
		return A.SortOrder < B.SortOrder;
	});

	return Result;
}

FMGBannerElement UMGPlayerTitleSubsystem::GetBannerElement(FName ElementID) const
{
	const FMGBannerElement* Element = AllBannerElements.Find(ElementID);
	return Element ? *Element : FMGBannerElement();
}

FMGPlayerBanner UMGPlayerTitleSubsystem::GetCurrentBanner() const
{
	return CurrentProfile.Banner;
}

bool UMGPlayerTitleSubsystem::SetBannerElement(EMGBannerSlot Slot, FName ElementID)
{
	// Verify element exists and is unlocked
	const FMGBannerElement* Element = AllBannerElements.Find(ElementID);
	if (!Element || !Element->bIsUnlocked)
	{
		return false;
	}

	if (Element->Slot != Slot)
	{
		return false;
	}

	switch (Slot)
	{
	case EMGBannerSlot::Background:
		CurrentProfile.Banner.BackgroundID = ElementID;
		break;
	case EMGBannerSlot::Emblem:
		CurrentProfile.Banner.EmblemID = ElementID;
		break;
	case EMGBannerSlot::Frame:
		CurrentProfile.Banner.FrameID = ElementID;
		break;
	case EMGBannerSlot::Effect:
		CurrentProfile.Banner.EffectID = ElementID;
		break;
	case EMGBannerSlot::Accent:
		CurrentProfile.Banner.AccentID = ElementID;
		break;
	}

	SaveTitleData();
	OnBannerChanged.Broadcast();

	return true;
}

void UMGPlayerTitleSubsystem::SetBannerColors(FLinearColor Primary, FLinearColor Secondary, FLinearColor Accent)
{
	CurrentProfile.Banner.PrimaryColor = Primary;
	CurrentProfile.Banner.SecondaryColor = Secondary;
	CurrentProfile.Banner.AccentColor = Accent;

	SaveTitleData();
	OnBannerChanged.Broadcast();
}

bool UMGPlayerTitleSubsystem::UnlockBannerElement(FName ElementID)
{
	FMGBannerElement* Element = AllBannerElements.Find(ElementID);
	if (!Element)
	{
		return false;
	}

	if (Element->bIsUnlocked)
	{
		return false;
	}

	Element->bIsUnlocked = true;
	SaveTitleData();

	OnBannerElementUnlocked.Broadcast(*Element);

	return true;
}

TArray<FMGNameplate> UMGPlayerTitleSubsystem::GetAllNameplates() const
{
	TArray<FMGNameplate> Result;
	for (const auto& Pair : AllNameplates)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TArray<FMGNameplate> UMGPlayerTitleSubsystem::GetUnlockedNameplates() const
{
	TArray<FMGNameplate> Result;
	for (const auto& Pair : AllNameplates)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGNameplate UMGPlayerTitleSubsystem::GetNameplate(FName NameplateID) const
{
	const FMGNameplate* Nameplate = AllNameplates.Find(NameplateID);
	return Nameplate ? *Nameplate : FMGNameplate();
}

FMGNameplate UMGPlayerTitleSubsystem::GetActiveNameplate() const
{
	return GetNameplate(CurrentProfile.ActiveNameplateID);
}

bool UMGPlayerTitleSubsystem::EquipNameplate(FName NameplateID)
{
	const FMGNameplate* Nameplate = AllNameplates.Find(NameplateID);
	if (!Nameplate || !Nameplate->bIsUnlocked)
	{
		return false;
	}

	CurrentProfile.ActiveNameplateID = NameplateID;
	SaveTitleData();

	OnProfileUpdated.Broadcast();

	return true;
}

bool UMGPlayerTitleSubsystem::UnlockNameplate(FName NameplateID)
{
	FMGNameplate* Nameplate = AllNameplates.Find(NameplateID);
	if (!Nameplate)
	{
		return false;
	}

	if (Nameplate->bIsUnlocked)
	{
		return false;
	}

	Nameplate->bIsUnlocked = true;
	SaveTitleData();

	return true;
}

void UMGPlayerTitleSubsystem::SetProfile(const FMGPlayerProfile& Profile)
{
	CurrentProfile = Profile;
	SaveTitleData();

	OnProfileUpdated.Broadcast();
}

void UMGPlayerTitleSubsystem::SetShowcaseTitles(const TArray<FName>& TitleIDs)
{
	CurrentProfile.ShowcaseTitles.Empty();

	for (const FName& TitleID : TitleIDs)
	{
		if (IsTitleUnlocked(TitleID))
		{
			CurrentProfile.ShowcaseTitles.Add(TitleID);
		}
	}

	SaveTitleData();
	OnProfileUpdated.Broadcast();
}

void UMGPlayerTitleSubsystem::SetDisplayOptions(bool bShowRank, bool bShowLevel, bool bShowCrew)
{
	CurrentProfile.bShowRank = bShowRank;
	CurrentProfile.bShowLevel = bShowLevel;
	CurrentProfile.bShowCrew = bShowCrew;

	SaveTitleData();
	OnProfileUpdated.Broadcast();
}

void UMGPlayerTitleSubsystem::SavePreset(FName PresetID, const FText& PresetName)
{
	// Check if preset exists
	for (int32 i = 0; i < Presets.Num(); i++)
	{
		if (Presets[i].PresetID == PresetID)
		{
			Presets[i].PresetName = PresetName;
			Presets[i].Profile = CurrentProfile;
			SaveTitleData();
			return;
		}
	}

	// Create new preset
	FMGTitlePreset NewPreset;
	NewPreset.PresetID = PresetID;
	NewPreset.PresetName = PresetName;
	NewPreset.Profile = CurrentProfile;
	Presets.Add(NewPreset);

	SaveTitleData();
}

bool UMGPlayerTitleSubsystem::LoadPreset(FName PresetID)
{
	for (const FMGTitlePreset& Preset : Presets)
	{
		if (Preset.PresetID == PresetID)
		{
			SetProfile(Preset.Profile);
			return true;
		}
	}
	return false;
}

bool UMGPlayerTitleSubsystem::DeletePreset(FName PresetID)
{
	for (int32 i = Presets.Num() - 1; i >= 0; i--)
	{
		if (Presets[i].PresetID == PresetID)
		{
			Presets.RemoveAt(i);
			SaveTitleData();
			return true;
		}
	}
	return false;
}

TArray<FMGTitlePreset> UMGPlayerTitleSubsystem::GetPresets() const
{
	return Presets;
}

int32 UMGPlayerTitleSubsystem::GetTotalTitlesUnlocked() const
{
	int32 Count = 0;
	for (const auto& Pair : AllTitles)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

int32 UMGPlayerTitleSubsystem::GetTotalBannerElementsUnlocked() const
{
	int32 Count = 0;
	for (const auto& Pair : AllBannerElements)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

float UMGPlayerTitleSubsystem::GetCollectionProgress() const
{
	int32 TotalItems = AllTitles.Num() + AllBannerElements.Num() + AllNameplates.Num();
	if (TotalItems == 0)
	{
		return 0.0f;
	}

	int32 UnlockedItems = GetTotalTitlesUnlocked() + GetTotalBannerElementsUnlocked();
	for (const auto& Pair : AllNameplates)
	{
		if (Pair.Value.bIsUnlocked)
		{
			UnlockedItems++;
		}
	}

	return (float)UnlockedItems / (float)TotalItems;
}

void UMGPlayerTitleSubsystem::SaveTitleData()
{
	// This would integrate with save game system
}

void UMGPlayerTitleSubsystem::LoadTitleData()
{
	// This would integrate with save game system
}

void UMGPlayerTitleSubsystem::InitializeDefaultTitles()
{
	// Racer Title (default unlocked)
	FMGPlayerTitle Racer;
	Racer.TitleID = FName("Title_Racer");
	Racer.TitleText = FText::FromString("Racer");
	Racer.Description = FText::FromString("A street racer in the making");
	Racer.Rarity = EMGTitleRarity::Common;
	Racer.Category = EMGTitleCategory::Achievement;
	Racer.TitleColor = FLinearColor::White;
	Racer.bIsUnlocked = true;
	Racer.SortOrder = 1;
	AllTitles.Add(Racer.TitleID, Racer);

	// Speed Demon
	FMGPlayerTitle SpeedDemon;
	SpeedDemon.TitleID = FName("Title_SpeedDemon");
	SpeedDemon.TitleText = FText::FromString("Speed Demon");
	SpeedDemon.Description = FText::FromString("Reach 300 km/h in any vehicle");
	SpeedDemon.Rarity = EMGTitleRarity::Rare;
	SpeedDemon.Category = EMGTitleCategory::Achievement;
	SpeedDemon.TitleColor = FLinearColor(1.0f, 0.4f, 0.0f, 1.0f);
	SpeedDemon.bHasGlow = true;
	SpeedDemon.GlowColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
	SpeedDemon.SortOrder = 10;
	AllTitles.Add(SpeedDemon.TitleID, SpeedDemon);

	// Drift King
	FMGPlayerTitle DriftKing;
	DriftKing.TitleID = FName("Title_DriftKing");
	DriftKing.TitleText = FText::FromString("Drift King");
	DriftKing.Description = FText::FromString("Accumulate 1,000,000 drift points");
	DriftKing.Rarity = EMGTitleRarity::Epic;
	DriftKing.Category = EMGTitleCategory::Mastery;
	DriftKing.TitleColor = FLinearColor(0.5f, 0.0f, 1.0f, 1.0f);
	DriftKing.bHasGlow = true;
	DriftKing.GlowColor = FLinearColor(0.6f, 0.0f, 1.0f, 1.0f);
	DriftKing.bIsAnimated = true;
	DriftKing.AnimationStyle = FName("Shimmer");
	DriftKing.SortOrder = 20;
	AllTitles.Add(DriftKing.TitleID, DriftKing);

	// Midnight Legend
	FMGPlayerTitle MidnightLegend;
	MidnightLegend.TitleID = FName("Title_MidnightLegend");
	MidnightLegend.TitleText = FText::FromString("Midnight Legend");
	MidnightLegend.Description = FText::FromString("Complete all story missions");
	MidnightLegend.Rarity = EMGTitleRarity::Legendary;
	MidnightLegend.Category = EMGTitleCategory::Achievement;
	MidnightLegend.TitleColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
	MidnightLegend.bHasGlow = true;
	MidnightLegend.GlowColor = FLinearColor(1.0f, 0.9f, 0.3f, 1.0f);
	MidnightLegend.bIsAnimated = true;
	MidnightLegend.AnimationStyle = FName("Pulse");
	MidnightLegend.SortOrder = 100;
	AllTitles.Add(MidnightLegend.TitleID, MidnightLegend);

	// Champion
	FMGPlayerTitle Champion;
	Champion.TitleID = FName("Title_Champion");
	Champion.TitleText = FText::FromString("Champion");
	Champion.Description = FText::FromString("Reach Diamond rank in any mode");
	Champion.Rarity = EMGTitleRarity::Epic;
	Champion.Category = EMGTitleCategory::Rank;
	Champion.TitleColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f);
	Champion.bHasGlow = true;
	Champion.GlowColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);
	Champion.SortOrder = 50;
	AllTitles.Add(Champion.TitleID, Champion);

	// Collector
	FMGPlayerTitle Collector;
	Collector.TitleID = FName("Title_Collector");
	Collector.TitleText = FText::FromString("Collector");
	Collector.Description = FText::FromString("Own 50 different vehicles");
	Collector.Rarity = EMGTitleRarity::Rare;
	Collector.Category = EMGTitleCategory::Collection;
	Collector.TitleColor = FLinearColor(0.2f, 0.8f, 0.4f, 1.0f);
	Collector.SortOrder = 30;
	AllTitles.Add(Collector.TitleID, Collector);
}

void UMGPlayerTitleSubsystem::InitializeDefaultBannerElements()
{
	// Default Background
	FMGBannerElement DefaultBG;
	DefaultBG.ElementID = FName("Banner_BG_Default");
	DefaultBG.ElementName = FText::FromString("Classic");
	DefaultBG.Slot = EMGBannerSlot::Background;
	DefaultBG.Rarity = EMGTitleRarity::Common;
	DefaultBG.bIsUnlocked = true;
	DefaultBG.SortOrder = 1;
	AllBannerElements.Add(DefaultBG.ElementID, DefaultBG);

	// Racing Stripes Background
	FMGBannerElement StripesBG;
	StripesBG.ElementID = FName("Banner_BG_Stripes");
	StripesBG.ElementName = FText::FromString("Racing Stripes");
	StripesBG.Slot = EMGBannerSlot::Background;
	StripesBG.Rarity = EMGTitleRarity::Uncommon;
	StripesBG.bIsUnlocked = false;
	StripesBG.SortOrder = 2;
	AllBannerElements.Add(StripesBG.ElementID, StripesBG);

	// Default Emblem
	FMGBannerElement DefaultEmblem;
	DefaultEmblem.ElementID = FName("Banner_Emblem_Default");
	DefaultEmblem.ElementName = FText::FromString("Star");
	DefaultEmblem.Slot = EMGBannerSlot::Emblem;
	DefaultEmblem.Rarity = EMGTitleRarity::Common;
	DefaultEmblem.bIsUnlocked = true;
	DefaultEmblem.SortOrder = 1;
	AllBannerElements.Add(DefaultEmblem.ElementID, DefaultEmblem);

	// Skull Emblem
	FMGBannerElement SkullEmblem;
	SkullEmblem.ElementID = FName("Banner_Emblem_Skull");
	SkullEmblem.ElementName = FText::FromString("Skull");
	SkullEmblem.Slot = EMGBannerSlot::Emblem;
	SkullEmblem.Rarity = EMGTitleRarity::Rare;
	SkullEmblem.bIsUnlocked = false;
	SkullEmblem.SortOrder = 5;
	AllBannerElements.Add(SkullEmblem.ElementID, SkullEmblem);

	// Default Frame
	FMGBannerElement DefaultFrame;
	DefaultFrame.ElementID = FName("Banner_Frame_Default");
	DefaultFrame.ElementName = FText::FromString("Simple");
	DefaultFrame.Slot = EMGBannerSlot::Frame;
	DefaultFrame.Rarity = EMGTitleRarity::Common;
	DefaultFrame.bIsUnlocked = true;
	DefaultFrame.SortOrder = 1;
	AllBannerElements.Add(DefaultFrame.ElementID, DefaultFrame);

	// Gold Frame
	FMGBannerElement GoldFrame;
	GoldFrame.ElementID = FName("Banner_Frame_Gold");
	GoldFrame.ElementName = FText::FromString("Golden");
	GoldFrame.Slot = EMGBannerSlot::Frame;
	GoldFrame.Rarity = EMGTitleRarity::Epic;
	GoldFrame.PrimaryColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
	GoldFrame.bIsUnlocked = false;
	GoldFrame.SortOrder = 10;
	AllBannerElements.Add(GoldFrame.ElementID, GoldFrame);
}

void UMGPlayerTitleSubsystem::InitializeDefaultNameplates()
{
	// Default Nameplate
	FMGNameplate DefaultPlate;
	DefaultPlate.NameplateID = FName("Nameplate_Default");
	DefaultPlate.NameplateName = FText::FromString("Classic");
	DefaultPlate.Rarity = EMGTitleRarity::Common;
	DefaultPlate.TextColor = FLinearColor::White;
	DefaultPlate.BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
	DefaultPlate.BorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	DefaultPlate.bIsUnlocked = true;
	AllNameplates.Add(DefaultPlate.NameplateID, DefaultPlate);

	// Chrome Nameplate
	FMGNameplate ChromePlate;
	ChromePlate.NameplateID = FName("Nameplate_Chrome");
	ChromePlate.NameplateName = FText::FromString("Chrome");
	ChromePlate.Rarity = EMGTitleRarity::Rare;
	ChromePlate.TextColor = FLinearColor::White;
	ChromePlate.BackgroundColor = FLinearColor(0.3f, 0.3f, 0.35f, 1.0f);
	ChromePlate.BorderColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
	ChromePlate.bIsAnimated = true;
	ChromePlate.bIsUnlocked = false;
	AllNameplates.Add(ChromePlate.NameplateID, ChromePlate);

	// Neon Nameplate
	FMGNameplate NeonPlate;
	NeonPlate.NameplateID = FName("Nameplate_Neon");
	NeonPlate.NameplateName = FText::FromString("Neon Glow");
	NeonPlate.Rarity = EMGTitleRarity::Epic;
	NeonPlate.TextColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
	NeonPlate.BackgroundColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);
	NeonPlate.BorderColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
	NeonPlate.bIsAnimated = true;
	NeonPlate.bIsUnlocked = false;
	AllNameplates.Add(NeonPlate.NameplateID, NeonPlate);
}

FLinearColor UMGPlayerTitleSubsystem::GetRarityColor(EMGTitleRarity Rarity) const
{
	switch (Rarity)
	{
	case EMGTitleRarity::Common: return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
	case EMGTitleRarity::Uncommon: return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	case EMGTitleRarity::Rare: return FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);
	case EMGTitleRarity::Epic: return FLinearColor(0.6f, 0.2f, 0.8f, 1.0f);
	case EMGTitleRarity::Legendary: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
	case EMGTitleRarity::Mythic: return FLinearColor(1.0f, 0.0f, 0.4f, 1.0f);
	case EMGTitleRarity::Unique: return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
	default: return FLinearColor::White;
	}
}
