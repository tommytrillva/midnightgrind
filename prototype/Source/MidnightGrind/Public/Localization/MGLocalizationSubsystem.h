// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLocalizationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGLanguage : uint8
{
	English,
	Spanish,
	French,
	German,
	Italian,
	Portuguese,
	Japanese,
	Korean,
	ChineseSimplified,
	ChineseTraditional,
	Russian,
	Polish,
	Arabic,
	Turkish
};

UENUM(BlueprintType)
enum class EMGRegion : uint8
{
	NorthAmerica,
	Europe,
	Asia,
	LatinAmerica,
	MiddleEast,
	Oceania
};

USTRUCT(BlueprintType)
struct FMGLocalizedString
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StringID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLanguage, FText> Translations;
};

USTRUCT(BlueprintType)
struct FMGLocalizationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLanguage CurrentLanguage = EMGLanguage::English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLanguage AudioLanguage = EMGLanguage::English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRegion Region = EMGRegion::NorthAmerica;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSystemLanguage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSubtitles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRightToLeftUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DateFormat = TEXT("MM/DD/YYYY");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimeFormat = TEXT("12h");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMetricUnits = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLanguageChanged, EMGLanguage, NewLanguage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRegionChanged, EMGRegion, NewRegion);

UCLASS()
class MIDNIGHTGRIND_API UMGLocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Language
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(EMGLanguage Language);

	UFUNCTION(BlueprintPure, Category = "Localization")
	EMGLanguage GetCurrentLanguage() const { return Settings.CurrentLanguage; }

	UFUNCTION(BlueprintPure, Category = "Localization")
	TArray<EMGLanguage> GetAvailableLanguages() const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLanguageDisplayName(EMGLanguage Language) const;

	// Strings
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedString(FName StringID) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatLocalizedString(FName StringID, const TArray<FText>& Arguments) const;

	// Region & Formatting
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetRegion(EMGRegion Region);

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatNumber(int64 Number) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatCurrency(int64 Amount, bool bIncludeSymbol = true) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDistance(float Meters) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatSpeed(float MetersPerSecond) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDateTime(const FDateTime& DateTime) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDuration(const FTimespan& Duration) const;

	// Settings
	UFUNCTION(BlueprintPure, Category = "Localization")
	FMGLocalizationSettings GetSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetUseMetricUnits(bool bMetric);

	UFUNCTION(BlueprintPure, Category = "Localization")
	bool UsesMetricUnits() const { return Settings.bUseMetricUnits; }

	UFUNCTION(BlueprintPure, Category = "Localization")
	bool IsRightToLeft() const { return Settings.bRightToLeftUI; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Localization|Events")
	FMGOnLanguageChanged OnLanguageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Localization|Events")
	FMGOnRegionChanged OnRegionChanged;

protected:
	void LoadSettings();
	void SaveSettings();
	void DetectSystemLanguage();
	void LoadStringTable(EMGLanguage Language);
	void UpdateUIDirection();

private:
	UPROPERTY()
	FMGLocalizationSettings Settings;

	UPROPERTY()
	TMap<FName, FMGLocalizedString> StringTable;
};
