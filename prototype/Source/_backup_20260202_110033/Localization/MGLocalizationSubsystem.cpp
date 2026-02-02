// Copyright Midnight Grind. All Rights Reserved.

#include "Localization/MGLocalizationSubsystem.h"
#include "Internationalization/Internationalization.h"

void UMGLocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();

	if (Settings.bUseSystemLanguage)
	{
		DetectSystemLanguage();
	}

	LoadStringTable(Settings.CurrentLanguage);
	UpdateUIDirection();
}

void UMGLocalizationSubsystem::Deinitialize()
{
	SaveSettings();
	Super::Deinitialize();
}

void UMGLocalizationSubsystem::SetLanguage(EMGLanguage Language)
{
	if (Settings.CurrentLanguage == Language)
		return;

	Settings.CurrentLanguage = Language;
	Settings.bRightToLeftUI = (Language == EMGLanguage::Arabic);

	LoadStringTable(Language);
	UpdateUIDirection();

	// Update Unreal's culture
	FString CultureCode;
	switch (Language)
	{
	case EMGLanguage::English: CultureCode = TEXT("en"); break;
	case EMGLanguage::Spanish: CultureCode = TEXT("es"); break;
	case EMGLanguage::French: CultureCode = TEXT("fr"); break;
	case EMGLanguage::German: CultureCode = TEXT("de"); break;
	case EMGLanguage::Italian: CultureCode = TEXT("it"); break;
	case EMGLanguage::Portuguese: CultureCode = TEXT("pt"); break;
	case EMGLanguage::Japanese: CultureCode = TEXT("ja"); break;
	case EMGLanguage::Korean: CultureCode = TEXT("ko"); break;
	case EMGLanguage::ChineseSimplified: CultureCode = TEXT("zh-Hans"); break;
	case EMGLanguage::ChineseTraditional: CultureCode = TEXT("zh-Hant"); break;
	case EMGLanguage::Russian: CultureCode = TEXT("ru"); break;
	case EMGLanguage::Polish: CultureCode = TEXT("pl"); break;
	case EMGLanguage::Arabic: CultureCode = TEXT("ar"); break;
	case EMGLanguage::Turkish: CultureCode = TEXT("tr"); break;
	}

	FInternationalization::Get().SetCurrentCulture(CultureCode);

	OnLanguageChanged.Broadcast(Language);
	SaveSettings();
}

TArray<EMGLanguage> UMGLocalizationSubsystem::GetAvailableLanguages() const
{
	return {
		EMGLanguage::English,
		EMGLanguage::Spanish,
		EMGLanguage::French,
		EMGLanguage::German,
		EMGLanguage::Italian,
		EMGLanguage::Portuguese,
		EMGLanguage::Japanese,
		EMGLanguage::Korean,
		EMGLanguage::ChineseSimplified,
		EMGLanguage::ChineseTraditional,
		EMGLanguage::Russian,
		EMGLanguage::Polish,
		EMGLanguage::Arabic,
		EMGLanguage::Turkish
	};
}

FText UMGLocalizationSubsystem::GetLanguageDisplayName(EMGLanguage Language) const
{
	switch (Language)
	{
	case EMGLanguage::English: return FText::FromString(TEXT("English"));
	case EMGLanguage::Spanish: return FText::FromString(TEXT("Español"));
	case EMGLanguage::French: return FText::FromString(TEXT("Français"));
	case EMGLanguage::German: return FText::FromString(TEXT("Deutsch"));
	case EMGLanguage::Italian: return FText::FromString(TEXT("Italiano"));
	case EMGLanguage::Portuguese: return FText::FromString(TEXT("Português"));
	case EMGLanguage::Japanese: return FText::FromString(TEXT("日本語"));
	case EMGLanguage::Korean: return FText::FromString(TEXT("한국어"));
	case EMGLanguage::ChineseSimplified: return FText::FromString(TEXT("简体中文"));
	case EMGLanguage::ChineseTraditional: return FText::FromString(TEXT("繁體中文"));
	case EMGLanguage::Russian: return FText::FromString(TEXT("Русский"));
	case EMGLanguage::Polish: return FText::FromString(TEXT("Polski"));
	case EMGLanguage::Arabic: return FText::FromString(TEXT("العربية"));
	case EMGLanguage::Turkish: return FText::FromString(TEXT("Türkçe"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGLocalizationSubsystem::GetLocalizedString(FName StringID) const
{
	const FMGLocalizedString* LocalString = StringTable.Find(StringID);
	if (LocalString)
	{
		const FText* Translation = LocalString->Translations.Find(Settings.CurrentLanguage);
		if (Translation)
			return *Translation;

		// Fallback to English
		Translation = LocalString->Translations.Find(EMGLanguage::English);
		if (Translation)
			return *Translation;
	}
	return FText::FromName(StringID);
}

FText UMGLocalizationSubsystem::FormatLocalizedString(FName StringID, const TArray<FText>& Arguments) const
{
	FText BaseString = GetLocalizedString(StringID);
	FString Result = BaseString.ToString();

	for (int32 i = 0; i < Arguments.Num(); i++)
	{
		FString Placeholder = FString::Printf(TEXT("{%d}"), i);
		Result = Result.Replace(*Placeholder, *Arguments[i].ToString());
	}

	return FText::FromString(Result);
}

void UMGLocalizationSubsystem::SetRegion(EMGRegion Region)
{
	if (Settings.Region == Region)
		return;

	Settings.Region = Region;

	// Set defaults based on region
	switch (Region)
	{
	case EMGRegion::NorthAmerica:
		Settings.bUseMetricUnits = false;
		Settings.DateFormat = TEXT("MM/DD/YYYY");
		Settings.TimeFormat = TEXT("12h");
		break;

	case EMGRegion::Europe:
	case EMGRegion::Asia:
	case EMGRegion::Oceania:
		Settings.bUseMetricUnits = true;
		Settings.DateFormat = TEXT("DD/MM/YYYY");
		Settings.TimeFormat = TEXT("24h");
		break;

	case EMGRegion::LatinAmerica:
		Settings.bUseMetricUnits = true;
		Settings.DateFormat = TEXT("DD/MM/YYYY");
		Settings.TimeFormat = TEXT("12h");
		break;

	case EMGRegion::MiddleEast:
		Settings.bUseMetricUnits = true;
		Settings.DateFormat = TEXT("DD/MM/YYYY");
		Settings.TimeFormat = TEXT("12h");
		break;
	}

	OnRegionChanged.Broadcast(Region);
	SaveSettings();
}

FText UMGLocalizationSubsystem::FormatNumber(int64 Number) const
{
	return FText::AsNumber(Number);
}

FText UMGLocalizationSubsystem::FormatCurrency(int64 Amount, bool bIncludeSymbol) const
{
	FString Symbol = bIncludeSymbol ? TEXT("$") : TEXT("");
	return FText::FromString(FString::Printf(TEXT("%s%lld"), *Symbol, Amount));
}

FText UMGLocalizationSubsystem::FormatDistance(float Meters) const
{
	if (Settings.bUseMetricUnits)
	{
		if (Meters >= 1000.0f)
			return FText::FromString(FString::Printf(TEXT("%.1f km"), Meters / 1000.0f));
		else
			return FText::FromString(FString::Printf(TEXT("%.0f m"), Meters));
	}
	else
	{
		float Miles = Meters * 0.000621371f;
		if (Miles >= 0.1f)
			return FText::FromString(FString::Printf(TEXT("%.1f mi"), Miles));
		else
		{
			float Feet = Meters * 3.28084f;
			return FText::FromString(FString::Printf(TEXT("%.0f ft"), Feet));
		}
	}
}

FText UMGLocalizationSubsystem::FormatSpeed(float MetersPerSecond) const
{
	if (Settings.bUseMetricUnits)
	{
		float KPH = MetersPerSecond * 3.6f;
		return FText::FromString(FString::Printf(TEXT("%.0f km/h"), KPH));
	}
	else
	{
		float MPH = MetersPerSecond * 2.23694f;
		return FText::FromString(FString::Printf(TEXT("%.0f mph"), MPH));
	}
}

FText UMGLocalizationSubsystem::FormatDateTime(const FDateTime& DateTime) const
{
	return FText::AsDateTime(DateTime);
}

FText UMGLocalizationSubsystem::FormatDuration(const FTimespan& Duration) const
{
	int32 Minutes = Duration.GetMinutes();
	int32 Seconds = Duration.GetSeconds();
	int32 Millis = Duration.GetFractionMilli();

	return FText::FromString(FString::Printf(TEXT("%d:%02d.%03d"), Minutes, Seconds, Millis));
}

void UMGLocalizationSubsystem::SetUseMetricUnits(bool bMetric)
{
	Settings.bUseMetricUnits = bMetric;
	SaveSettings();
}

void UMGLocalizationSubsystem::LoadSettings()
{
	// Would load from save
}

void UMGLocalizationSubsystem::SaveSettings()
{
	// Would save settings
}

void UMGLocalizationSubsystem::DetectSystemLanguage()
{
	FString SystemCulture = FInternationalization::Get().GetCurrentCulture()->GetName();

	if (SystemCulture.StartsWith(TEXT("en"))) Settings.CurrentLanguage = EMGLanguage::English;
	else if (SystemCulture.StartsWith(TEXT("es"))) Settings.CurrentLanguage = EMGLanguage::Spanish;
	else if (SystemCulture.StartsWith(TEXT("fr"))) Settings.CurrentLanguage = EMGLanguage::French;
	else if (SystemCulture.StartsWith(TEXT("de"))) Settings.CurrentLanguage = EMGLanguage::German;
	else if (SystemCulture.StartsWith(TEXT("it"))) Settings.CurrentLanguage = EMGLanguage::Italian;
	else if (SystemCulture.StartsWith(TEXT("pt"))) Settings.CurrentLanguage = EMGLanguage::Portuguese;
	else if (SystemCulture.StartsWith(TEXT("ja"))) Settings.CurrentLanguage = EMGLanguage::Japanese;
	else if (SystemCulture.StartsWith(TEXT("ko"))) Settings.CurrentLanguage = EMGLanguage::Korean;
	else if (SystemCulture.StartsWith(TEXT("zh-Hans"))) Settings.CurrentLanguage = EMGLanguage::ChineseSimplified;
	else if (SystemCulture.StartsWith(TEXT("zh"))) Settings.CurrentLanguage = EMGLanguage::ChineseTraditional;
	else if (SystemCulture.StartsWith(TEXT("ru"))) Settings.CurrentLanguage = EMGLanguage::Russian;
	else if (SystemCulture.StartsWith(TEXT("pl"))) Settings.CurrentLanguage = EMGLanguage::Polish;
	else if (SystemCulture.StartsWith(TEXT("ar"))) Settings.CurrentLanguage = EMGLanguage::Arabic;
	else if (SystemCulture.StartsWith(TEXT("tr"))) Settings.CurrentLanguage = EMGLanguage::Turkish;
	else Settings.CurrentLanguage = EMGLanguage::English;
}

void UMGLocalizationSubsystem::LoadStringTable(EMGLanguage Language)
{
	// Would load localized strings from data assets
	StringTable.Empty();
}

void UMGLocalizationSubsystem::UpdateUIDirection()
{
	// Would notify UI system about RTL/LTR change
}
