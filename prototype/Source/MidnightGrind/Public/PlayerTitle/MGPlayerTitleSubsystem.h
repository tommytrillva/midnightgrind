// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPlayerTitleSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTitleRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Mythic,
	Unique
};

UENUM(BlueprintType)
enum class EMGTitleCategory : uint8
{
	Achievement,
	Rank,
	Season,
	Event,
	Social,
	Collection,
	Mastery,
	Special,
	Developer
};

UENUM(BlueprintType)
enum class EMGBannerSlot : uint8
{
	Background,
	Emblem,
	Frame,
	Effect,
	Accent
};

USTRUCT(BlueprintType)
struct FMGPlayerTitle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TitleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleCategory Category = EMGTitleCategory::Achievement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TitleColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasGlow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AnimationStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime UnlockedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct FMGBannerElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ElementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ElementName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBannerSlot Slot = EMGBannerSlot::Background;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ElementTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> ElementMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsColorizable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct FMGPlayerBanner
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BackgroundID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmblemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrameID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EffectID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AccentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AccentColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGNameplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NameplateID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText NameplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> NameplateTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BackgroundColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BorderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;
};

USTRUCT(BlueprintType)
struct FMGPlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveTitleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerBanner Banner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveNameplateID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveAvatarFrameID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ShowcaseTitles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRank = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLevel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCrew = true;
};

USTRUCT(BlueprintType)
struct FMGTitlePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerProfile Profile;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleUnlocked, const FMGPlayerTitle&, Title);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBannerElementUnlocked, const FMGBannerElement&, Element);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleEquipped, FName, TitleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBannerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfileUpdated);

UCLASS()
class MIDNIGHTGRIND_API UMGPlayerTitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Titles
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetAllTitles() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetUnlockedTitles() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetTitlesByCategory(EMGTitleCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetTitlesByRarity(EMGTitleRarity Rarity) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	FMGPlayerTitle GetTitle(FName TitleID) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	FMGPlayerTitle GetActiveTitle() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	bool IsTitleUnlocked(FName TitleID) const;

	UFUNCTION(BlueprintCallable, Category = "Titles|Equip")
	bool EquipTitle(FName TitleID);

	UFUNCTION(BlueprintCallable, Category = "Titles|Equip")
	void UnequipTitle();

	UFUNCTION(BlueprintCallable, Category = "Titles|Unlock")
	bool UnlockTitle(FName TitleID);

	// Banner
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetAllBannerElements() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetUnlockedBannerElements() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetBannerElementsBySlot(EMGBannerSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	FMGBannerElement GetBannerElement(FName ElementID) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	FMGPlayerBanner GetCurrentBanner() const;

	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	bool SetBannerElement(EMGBannerSlot Slot, FName ElementID);

	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	void SetBannerColors(FLinearColor Primary, FLinearColor Secondary, FLinearColor Accent);

	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	bool UnlockBannerElement(FName ElementID);

	// Nameplates
	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	TArray<FMGNameplate> GetAllNameplates() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	TArray<FMGNameplate> GetUnlockedNameplates() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	FMGNameplate GetNameplate(FName NameplateID) const;

	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	FMGNameplate GetActiveNameplate() const;

	UFUNCTION(BlueprintCallable, Category = "Titles|Nameplate")
	bool EquipNameplate(FName NameplateID);

	UFUNCTION(BlueprintCallable, Category = "Titles|Nameplate")
	bool UnlockNameplate(FName NameplateID);

	// Profile
	UFUNCTION(BlueprintPure, Category = "Titles|Profile")
	FMGPlayerProfile GetProfile() const { return CurrentProfile; }

	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetProfile(const FMGPlayerProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetShowcaseTitles(const TArray<FName>& TitleIDs);

	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetDisplayOptions(bool bShowRank, bool bShowLevel, bool bShowCrew);

	// Presets
	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	void SavePreset(FName PresetID, const FText& PresetName);

	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	bool LoadPreset(FName PresetID);

	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	bool DeletePreset(FName PresetID);

	UFUNCTION(BlueprintPure, Category = "Titles|Presets")
	TArray<FMGTitlePreset> GetPresets() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	int32 GetTotalTitlesUnlocked() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	int32 GetTotalBannerElementsUnlocked() const;

	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	float GetCollectionProgress() const;

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "Titles|Save")
	void SaveTitleData();

	UFUNCTION(BlueprintCallable, Category = "Titles|Save")
	void LoadTitleData();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnTitleUnlocked OnTitleUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnBannerElementUnlocked OnBannerElementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnTitleEquipped OnTitleEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnBannerChanged OnBannerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnProfileUpdated OnProfileUpdated;

protected:
	void InitializeDefaultTitles();
	void InitializeDefaultBannerElements();
	void InitializeDefaultNameplates();
	FLinearColor GetRarityColor(EMGTitleRarity Rarity) const;

	UPROPERTY()
	TMap<FName, FMGPlayerTitle> AllTitles;

	UPROPERTY()
	TMap<FName, FMGBannerElement> AllBannerElements;

	UPROPERTY()
	TMap<FName, FMGNameplate> AllNameplates;

	UPROPERTY()
	FMGPlayerProfile CurrentProfile;

	UPROPERTY()
	TArray<FMGTitlePreset> Presets;
};
