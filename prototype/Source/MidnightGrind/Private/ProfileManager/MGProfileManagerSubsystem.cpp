// Copyright Midnight Grind. All Rights Reserved.

#include "ProfileManager/MGProfileManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Misc/SecureHash.h"
#include "HAL/FileManager.h"

UMGProfileManagerSubsystem::UMGProfileManagerSubsystem()
    : bHasLoadedProfile(false)
    , bIsDirty(false)
    , MaxRaceHistoryEntries(500)
    , MaxControlPresets(10)
    , MaxLevel(100)
    , PrestigeMaxLevel(10)
    , AutosaveInterval(60.0f) // Autosave every minute
{
}

void UMGProfileManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Start autosave timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AutosaveTimerHandle,
            this,
            &UMGProfileManagerSubsystem::PerformAutoSave,
            AutosaveInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Subsystem initialized"));
}

void UMGProfileManagerSubsystem::Deinitialize()
{
    // Final save before shutdown
    if (bIsDirty && bHasLoadedProfile)
    {
        SaveProfile();
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutosaveTimerHandle);
    }

    Super::Deinitialize();
}

bool UMGProfileManagerSubsystem::LoadProfile(const FString& PlayerId)
{
    // In a real implementation, this would load from cloud or local storage
    // For now, create a new profile or load cached data

    CurrentProfile = FMGPlayerProfile();
    CurrentProfile.PlayerId = PlayerId;
    CurrentProfile.LastLoginDate = FDateTime::Now();

    InitializeDefaultProfile();

    bHasLoadedProfile = true;
    bIsDirty = false;

    OnProfileLoaded.Broadcast(CurrentProfile);

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Loaded profile for player %s"), *PlayerId);
    return true;
}

bool UMGProfileManagerSubsystem::SaveProfile()
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    // In a real implementation, this would save to cloud or local storage
    // For now, just mark as saved

    bIsDirty = false;
    OnProfileSaved.Broadcast(true);

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Profile saved successfully"));
    return true;
}

bool UMGProfileManagerSubsystem::CreateNewProfile(const FString& PlayerId, const FString& DisplayName)
{
    CurrentProfile = FMGPlayerProfile();
    CurrentProfile.PlayerId = PlayerId;
    CurrentProfile.DisplayName = DisplayName;
    CurrentProfile.CreatedDate = FDateTime::Now();
    CurrentProfile.LastLoginDate = FDateTime::Now();

    InitializeDefaultProfile();

    bHasLoadedProfile = true;
    MarkDirty();

    OnProfileLoaded.Broadcast(CurrentProfile);

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Created new profile for %s (%s)"), *DisplayName, *PlayerId);
    return true;
}

bool UMGProfileManagerSubsystem::DeleteProfile(const FString& PlayerId)
{
    if (CurrentProfile.PlayerId == PlayerId)
    {
        CurrentProfile = FMGPlayerProfile();
        bHasLoadedProfile = false;
        bIsDirty = false;
    }

    // In real implementation, delete from storage
    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Deleted profile for player %s"), *PlayerId);
    return true;
}

FMGPlayerProfile UMGProfileManagerSubsystem::GetCurrentProfile() const
{
    return CurrentProfile;
}

bool UMGProfileManagerSubsystem::HasActiveProfile() const
{
    return bHasLoadedProfile;
}

bool UMGProfileManagerSubsystem::MigrateProfile(EMGProfileVersion TargetVersion)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    EMGProfileVersion OldVersion = CurrentProfile.Version;

    if (OldVersion >= TargetVersion)
    {
        return true; // Already at or past target version
    }

    // Perform migration steps
    if (OldVersion < EMGProfileVersion::AddedStats)
    {
        InitializeDefaultStats();
    }

    if (OldVersion < EMGProfileVersion::AddedPresets)
    {
        InitializeDefaultControlPreset();
    }

    if (OldVersion < EMGProfileVersion::AddedHistory)
    {
        CurrentProfile.RaceHistory.Empty();
    }

    if (OldVersion < EMGProfileVersion::AddedSocial)
    {
        CurrentProfile.Friends.Empty();
        CurrentProfile.BlockedPlayers.Empty();
    }

    CurrentProfile.Version = TargetVersion;
    MarkDirty();

    OnProfileMigrated.Broadcast(OldVersion, TargetVersion);

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Migrated profile from version %d to %d"),
           (int32)OldVersion, (int32)TargetVersion);
    return true;
}

bool UMGProfileManagerSubsystem::SetDisplayName(const FString& NewName)
{
    if (!bHasLoadedProfile || NewName.IsEmpty())
    {
        return false;
    }

    CurrentProfile.DisplayName = NewName;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetAvatar(const FString& AvatarPath)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    CurrentProfile.AvatarPath = AvatarPath;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetBanner(const FString& BannerPath)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    CurrentProfile.BannerPath = BannerPath;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetTitle(const FString& TitleId)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    CurrentProfile.TitleId = TitleId;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetBadge(const FString& BadgeId)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    CurrentProfile.BadgeId = BadgeId;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetStatus(EMGPlayerStatus NewStatus)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    CurrentProfile.Status = NewStatus;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::UpdateCareerStat(const FString& StatId, int64 Value, bool bIsDelta)
{
    if (!bHasLoadedProfile || StatId.IsEmpty())
    {
        return false;
    }

    if (!CurrentProfile.CareerStats.Contains(StatId))
    {
        // Auto-register stat if not exists
        FMGCareerStat NewStat;
        NewStat.StatId = StatId;
        NewStat.DisplayName = FText::FromString(StatId);
        NewStat.bHigherIsBetter = true;
        CurrentProfile.CareerStats.Add(StatId, NewStat);
    }

    FMGCareerStat& Stat = CurrentProfile.CareerStats[StatId];

    int64 OldValue = Stat.Value;

    if (bIsDelta)
    {
        Stat.Value += Value;
    }
    else
    {
        Stat.Value = Value;
    }

    // Update best value
    if (Stat.bHigherIsBetter)
    {
        Stat.BestValue = FMath::Max(Stat.BestValue, Stat.Value);
    }
    else
    {
        if (Stat.BestValue == 0 || Stat.Value < Stat.BestValue)
        {
            Stat.BestValue = Stat.Value;
        }
    }

    Stat.LastUpdated = FDateTime::Now();
    MarkDirty();

    OnStatUpdated.Broadcast(StatId, Stat.Value);
    return true;
}

int64 UMGProfileManagerSubsystem::GetCareerStatValue(const FString& StatId) const
{
    if (const FMGCareerStat* Stat = CurrentProfile.CareerStats.Find(StatId))
    {
        return Stat->Value;
    }
    return 0;
}

int64 UMGProfileManagerSubsystem::GetCareerStatBest(const FString& StatId) const
{
    if (const FMGCareerStat* Stat = CurrentProfile.CareerStats.Find(StatId))
    {
        return Stat->BestValue;
    }
    return 0;
}

TArray<FMGCareerStat> UMGProfileManagerSubsystem::GetAllCareerStats() const
{
    TArray<FMGCareerStat> Stats;
    CurrentProfile.CareerStats.GenerateValueArray(Stats);
    return Stats;
}

void UMGProfileManagerSubsystem::RegisterCareerStat(const FString& StatId, const FText& DisplayName, bool bHigherIsBetter)
{
    if (!CurrentProfile.CareerStats.Contains(StatId))
    {
        FMGCareerStat NewStat;
        NewStat.StatId = StatId;
        NewStat.DisplayName = DisplayName;
        NewStat.bHigherIsBetter = bHigherIsBetter;
        CurrentProfile.CareerStats.Add(StatId, NewStat);
        MarkDirty();
    }
}

void UMGProfileManagerSubsystem::AddRaceToHistory(const FMGRaceHistoryEntry& Entry)
{
    if (!bHasLoadedProfile)
    {
        return;
    }

    CurrentProfile.RaceHistory.Insert(Entry, 0);

    // Trim to max entries
    if (CurrentProfile.RaceHistory.Num() > MaxRaceHistoryEntries)
    {
        CurrentProfile.RaceHistory.SetNum(MaxRaceHistoryEntries);
    }

    MarkDirty();
    OnRaceHistoryAdded.Broadcast(Entry);
}

TArray<FMGRaceHistoryEntry> UMGProfileManagerSubsystem::GetRaceHistory(int32 MaxEntries) const
{
    TArray<FMGRaceHistoryEntry> Result;
    int32 Count = FMath::Min(MaxEntries, CurrentProfile.RaceHistory.Num());

    for (int32 i = 0; i < Count; ++i)
    {
        Result.Add(CurrentProfile.RaceHistory[i]);
    }

    return Result;
}

TArray<FMGRaceHistoryEntry> UMGProfileManagerSubsystem::GetRaceHistoryForTrack(const FString& TrackId) const
{
    TArray<FMGRaceHistoryEntry> Result;

    for (const FMGRaceHistoryEntry& Entry : CurrentProfile.RaceHistory)
    {
        if (Entry.TrackId == TrackId)
        {
            Result.Add(Entry);
        }
    }

    return Result;
}

TArray<FMGRaceHistoryEntry> UMGProfileManagerSubsystem::GetRaceHistoryForVehicle(const FString& VehicleId) const
{
    TArray<FMGRaceHistoryEntry> Result;

    for (const FMGRaceHistoryEntry& Entry : CurrentProfile.RaceHistory)
    {
        if (Entry.VehicleId == VehicleId)
        {
            Result.Add(Entry);
        }
    }

    return Result;
}

void UMGProfileManagerSubsystem::ClearRaceHistory()
{
    CurrentProfile.RaceHistory.Empty();
    MarkDirty();
}

void UMGProfileManagerSubsystem::UpdateVehicleStats(const FMGVehicleUsageStats& Stats)
{
    if (!bHasLoadedProfile || Stats.VehicleId.IsEmpty())
    {
        return;
    }

    if (FMGVehicleUsageStats* Existing = CurrentProfile.VehicleStats.Find(Stats.VehicleId))
    {
        // Merge stats
        Existing->RacesCompleted += Stats.RacesCompleted;
        Existing->Wins += Stats.Wins;
        Existing->Podiums += Stats.Podiums;
        Existing->TotalDistanceDriven += Stats.TotalDistanceDriven;
        Existing->TotalTimeDriven += Stats.TotalTimeDriven;
        Existing->BestTopSpeed = FMath::Max(Existing->BestTopSpeed, Stats.BestTopSpeed);
        Existing->LongestDrift = FMath::Max(Existing->LongestDrift, Stats.LongestDrift);
        Existing->TotalDrifts += Stats.TotalDrifts;
        Existing->TotalTakedowns += Stats.TotalTakedowns;
        Existing->TotalNitroBoosts += Stats.TotalNitroBoosts;
        Existing->LastUsed = FDateTime::Now();
    }
    else
    {
        CurrentProfile.VehicleStats.Add(Stats.VehicleId, Stats);
    }

    MarkDirty();
}

FMGVehicleUsageStats UMGProfileManagerSubsystem::GetVehicleStats(const FString& VehicleId) const
{
    if (const FMGVehicleUsageStats* Stats = CurrentProfile.VehicleStats.Find(VehicleId))
    {
        return *Stats;
    }
    return FMGVehicleUsageStats();
}

TArray<FMGVehicleUsageStats> UMGProfileManagerSubsystem::GetAllVehicleStats() const
{
    TArray<FMGVehicleUsageStats> Stats;
    CurrentProfile.VehicleStats.GenerateValueArray(Stats);
    return Stats;
}

TArray<FMGVehicleUsageStats> UMGProfileManagerSubsystem::GetFavoriteVehicles() const
{
    TArray<FMGVehicleUsageStats> Favorites;

    for (const auto& Pair : CurrentProfile.VehicleStats)
    {
        if (Pair.Value.bIsFavorite)
        {
            Favorites.Add(Pair.Value);
        }
    }

    return Favorites;
}

FMGVehicleUsageStats UMGProfileManagerSubsystem::GetMostUsedVehicle() const
{
    FMGVehicleUsageStats MostUsed;
    int32 MaxRaces = 0;

    for (const auto& Pair : CurrentProfile.VehicleStats)
    {
        if (Pair.Value.RacesCompleted > MaxRaces)
        {
            MaxRaces = Pair.Value.RacesCompleted;
            MostUsed = Pair.Value;
        }
    }

    return MostUsed;
}

void UMGProfileManagerSubsystem::SetVehicleFavorite(const FString& VehicleId, bool bFavorite)
{
    if (FMGVehicleUsageStats* Stats = CurrentProfile.VehicleStats.Find(VehicleId))
    {
        Stats->bIsFavorite = bFavorite;
        MarkDirty();
    }
}

void UMGProfileManagerSubsystem::UpdateTrackRecord(const FMGTrackRecord& Record)
{
    if (!bHasLoadedProfile || Record.TrackId.IsEmpty())
    {
        return;
    }

    if (FMGTrackRecord* Existing = CurrentProfile.TrackRecords.Find(Record.TrackId))
    {
        // Merge records
        Existing->TimesPlayed += Record.TimesPlayed;
        Existing->Wins += Record.Wins;
        Existing->Podiums += Record.Podiums;

        if (Record.BestTime < Existing->BestTime)
        {
            Existing->BestTime = Record.BestTime;
            Existing->BestVehicleId = Record.BestVehicleId;
            Existing->PersonalRecordDate = FDateTime::Now();
        }

        if (Record.BestLapTime < Existing->BestLapTime)
        {
            Existing->BestLapTime = Record.BestLapTime;
        }

        if (Record.BestPosition < Existing->BestPosition)
        {
            Existing->BestPosition = Record.BestPosition;
        }

        // Update average position
        float TotalPositions = Existing->AveragePosition * (Existing->TimesPlayed - Record.TimesPlayed);
        TotalPositions += Record.BestPosition;
        Existing->AveragePosition = (Existing->TimesPlayed > 0) ? TotalPositions / Existing->TimesPlayed : Record.BestPosition;
    }
    else
    {
        CurrentProfile.TrackRecords.Add(Record.TrackId, Record);
    }

    MarkDirty();
}

FMGTrackRecord UMGProfileManagerSubsystem::GetTrackRecord(const FString& TrackId) const
{
    if (const FMGTrackRecord* Record = CurrentProfile.TrackRecords.Find(TrackId))
    {
        return *Record;
    }
    return FMGTrackRecord();
}

TArray<FMGTrackRecord> UMGProfileManagerSubsystem::GetAllTrackRecords() const
{
    TArray<FMGTrackRecord> Records;
    CurrentProfile.TrackRecords.GenerateValueArray(Records);
    return Records;
}

TArray<FMGTrackRecord> UMGProfileManagerSubsystem::GetFavoriteTracks() const
{
    TArray<FMGTrackRecord> Favorites;

    for (const auto& Pair : CurrentProfile.TrackRecords)
    {
        if (Pair.Value.bIsFavorite)
        {
            Favorites.Add(Pair.Value);
        }
    }

    return Favorites;
}

void UMGProfileManagerSubsystem::SetTrackFavorite(const FString& TrackId, bool bFavorite)
{
    if (FMGTrackRecord* Record = CurrentProfile.TrackRecords.Find(TrackId))
    {
        Record->bIsFavorite = bFavorite;
        MarkDirty();
    }
}

void UMGProfileManagerSubsystem::RegisterAchievement(const FMGPlayerAchievement& Achievement)
{
    if (Achievement.AchievementId.IsEmpty())
    {
        return;
    }

    if (!CurrentProfile.Achievements.Contains(Achievement.AchievementId))
    {
        CurrentProfile.Achievements.Add(Achievement.AchievementId, Achievement);
        MarkDirty();
    }
}

bool UMGProfileManagerSubsystem::UpdateAchievementProgress(const FString& AchievementId, float Progress)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    if (FMGPlayerAchievement* Achievement = CurrentProfile.Achievements.Find(AchievementId))
    {
        if (Achievement->bUnlocked)
        {
            return false; // Already unlocked
        }

        Achievement->Progress = FMath::Clamp(Progress, 0.0f, Achievement->TargetValue);
        MarkDirty();

        float ProgressRatio = (Achievement->TargetValue > 0.0f) ? Achievement->Progress / Achievement->TargetValue : 1.0f;
        OnAchievementProgress.Broadcast(AchievementId, ProgressRatio);

        // Check for completion
        if (Achievement->Progress >= Achievement->TargetValue)
        {
            return UnlockAchievement(AchievementId);
        }

        return true;
    }

    return false;
}

bool UMGProfileManagerSubsystem::UnlockAchievement(const FString& AchievementId)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    if (FMGPlayerAchievement* Achievement = CurrentProfile.Achievements.Find(AchievementId))
    {
        if (Achievement->bUnlocked)
        {
            return false;
        }

        Achievement->bUnlocked = true;
        Achievement->Progress = Achievement->TargetValue;
        Achievement->UnlockDate = FDateTime::Now();

        CurrentProfile.AchievementPoints += Achievement->PointsValue;

        MarkDirty();

        OnAchievementUnlocked.Broadcast(AchievementId, *Achievement);

        UE_LOG(LogTemp, Log, TEXT("ProfileManager: Achievement unlocked: %s"), *AchievementId);
        return true;
    }

    return false;
}

FMGPlayerAchievement UMGProfileManagerSubsystem::GetAchievement(const FString& AchievementId) const
{
    if (const FMGPlayerAchievement* Achievement = CurrentProfile.Achievements.Find(AchievementId))
    {
        return *Achievement;
    }
    return FMGPlayerAchievement();
}

TArray<FMGPlayerAchievement> UMGProfileManagerSubsystem::GetAllAchievements() const
{
    TArray<FMGPlayerAchievement> Achievements;
    CurrentProfile.Achievements.GenerateValueArray(Achievements);
    return Achievements;
}

TArray<FMGPlayerAchievement> UMGProfileManagerSubsystem::GetUnlockedAchievements() const
{
    TArray<FMGPlayerAchievement> Unlocked;

    for (const auto& Pair : CurrentProfile.Achievements)
    {
        if (Pair.Value.bUnlocked)
        {
            Unlocked.Add(Pair.Value);
        }
    }

    return Unlocked;
}

TArray<FMGPlayerAchievement> UMGProfileManagerSubsystem::GetLockedAchievements() const
{
    TArray<FMGPlayerAchievement> Locked;

    for (const auto& Pair : CurrentProfile.Achievements)
    {
        if (!Pair.Value.bUnlocked && !Pair.Value.bHidden)
        {
            Locked.Add(Pair.Value);
        }
    }

    return Locked;
}

int32 UMGProfileManagerSubsystem::GetTotalAchievementPoints() const
{
    return CurrentProfile.AchievementPoints;
}

float UMGProfileManagerSubsystem::GetAchievementCompletionPercent() const
{
    int32 Total = CurrentProfile.Achievements.Num();
    if (Total == 0)
    {
        return 0.0f;
    }

    int32 Unlocked = GetUnlockedAchievements().Num();
    return (float)Unlocked / (float)Total * 100.0f;
}

bool UMGProfileManagerSubsystem::AddExperience(int64 Amount)
{
    if (!bHasLoadedProfile || Amount <= 0)
    {
        return false;
    }

    int64 OldExperience = CurrentProfile.TotalExperience;
    CurrentProfile.TotalExperience += Amount;

    CheckLevelUp(OldExperience, CurrentProfile.TotalExperience);

    MarkDirty();
    return true;
}

int64 UMGProfileManagerSubsystem::GetExperienceForLevel(int32 Level) const
{
    // Exponential XP curve
    // Level 1 = 0 XP
    // Level 2 = 1000 XP
    // Level 10 = ~27000 XP
    // Level 50 = ~637000 XP
    // Level 100 = ~2,500,000 XP

    if (Level <= 1)
    {
        return 0;
    }

    return static_cast<int64>(FMath::Pow(Level - 1, 2.0f) * 250.0f + 750.0f * (Level - 1));
}

int64 UMGProfileManagerSubsystem::GetExperienceToNextLevel() const
{
    if (CurrentProfile.Level >= MaxLevel)
    {
        return 0;
    }

    int64 NextLevelXP = GetExperienceForLevel(CurrentProfile.Level + 1);
    return NextLevelXP - CurrentProfile.TotalExperience;
}

float UMGProfileManagerSubsystem::GetLevelProgress() const
{
    if (CurrentProfile.Level >= MaxLevel)
    {
        return 1.0f;
    }

    int64 CurrentLevelXP = GetExperienceForLevel(CurrentProfile.Level);
    int64 NextLevelXP = GetExperienceForLevel(CurrentProfile.Level + 1);
    int64 LevelRange = NextLevelXP - CurrentLevelXP;

    if (LevelRange <= 0)
    {
        return 0.0f;
    }

    int64 ProgressInLevel = CurrentProfile.TotalExperience - CurrentLevelXP;
    return FMath::Clamp((float)ProgressInLevel / (float)LevelRange, 0.0f, 1.0f);
}

bool UMGProfileManagerSubsystem::Prestige()
{
    if (!CanPrestige())
    {
        return false;
    }

    CurrentProfile.PrestigeLevel++;
    CurrentProfile.Level = 1;
    CurrentProfile.TotalExperience = 0;

    // Prestige rewards could be added here

    MarkDirty();

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Player prestiged to level %d"), CurrentProfile.PrestigeLevel);
    return true;
}

bool UMGProfileManagerSubsystem::CanPrestige() const
{
    return bHasLoadedProfile &&
           CurrentProfile.Level >= MaxLevel &&
           CurrentProfile.PrestigeLevel < PrestigeMaxLevel;
}

bool UMGProfileManagerSubsystem::AddSoftCurrency(int64 Amount)
{
    if (!bHasLoadedProfile || Amount < 0)
    {
        return false;
    }

    CurrentProfile.SoftCurrency += Amount;
    MarkDirty();

    OnCurrencyChanged.Broadcast(CurrentProfile.SoftCurrency, CurrentProfile.PremiumCurrency);
    return true;
}

bool UMGProfileManagerSubsystem::SpendSoftCurrency(int64 Amount)
{
    if (!bHasLoadedProfile || Amount < 0 || CurrentProfile.SoftCurrency < Amount)
    {
        return false;
    }

    CurrentProfile.SoftCurrency -= Amount;
    MarkDirty();

    OnCurrencyChanged.Broadcast(CurrentProfile.SoftCurrency, CurrentProfile.PremiumCurrency);
    return true;
}

bool UMGProfileManagerSubsystem::AddPremiumCurrency(int64 Amount)
{
    if (!bHasLoadedProfile || Amount < 0)
    {
        return false;
    }

    CurrentProfile.PremiumCurrency += Amount;
    MarkDirty();

    OnCurrencyChanged.Broadcast(CurrentProfile.SoftCurrency, CurrentProfile.PremiumCurrency);
    return true;
}

bool UMGProfileManagerSubsystem::SpendPremiumCurrency(int64 Amount)
{
    if (!bHasLoadedProfile || Amount < 0 || CurrentProfile.PremiumCurrency < Amount)
    {
        return false;
    }

    CurrentProfile.PremiumCurrency -= Amount;
    MarkDirty();

    OnCurrencyChanged.Broadcast(CurrentProfile.SoftCurrency, CurrentProfile.PremiumCurrency);
    return true;
}

int64 UMGProfileManagerSubsystem::GetSoftCurrency() const
{
    return CurrentProfile.SoftCurrency;
}

int64 UMGProfileManagerSubsystem::GetPremiumCurrency() const
{
    return CurrentProfile.PremiumCurrency;
}

void UMGProfileManagerSubsystem::UpdateProfileSettings(const FMGProfileSettings& NewSettings)
{
    CurrentProfile.Settings = NewSettings;
    MarkDirty();
}

FMGProfileSettings UMGProfileManagerSubsystem::GetProfileSettings() const
{
    return CurrentProfile.Settings;
}

bool UMGProfileManagerSubsystem::AddControlPreset(const FMGControlPreset& Preset)
{
    if (!bHasLoadedProfile || CurrentProfile.ControlPresets.Num() >= MaxControlPresets)
    {
        return false;
    }

    CurrentProfile.ControlPresets.Add(Preset);
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::UpdateControlPreset(int32 Index, const FMGControlPreset& Preset)
{
    if (!bHasLoadedProfile || !CurrentProfile.ControlPresets.IsValidIndex(Index))
    {
        return false;
    }

    CurrentProfile.ControlPresets[Index] = Preset;
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::RemoveControlPreset(int32 Index)
{
    if (!bHasLoadedProfile ||
        !CurrentProfile.ControlPresets.IsValidIndex(Index) ||
        CurrentProfile.ControlPresets[Index].bIsDefault ||
        CurrentProfile.ControlPresets.Num() <= 1)
    {
        return false;
    }

    CurrentProfile.ControlPresets.RemoveAt(Index);

    if (CurrentProfile.ActivePresetIndex >= CurrentProfile.ControlPresets.Num())
    {
        CurrentProfile.ActivePresetIndex = CurrentProfile.ControlPresets.Num() - 1;
    }

    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::SetActivePreset(int32 Index)
{
    if (!bHasLoadedProfile || !CurrentProfile.ControlPresets.IsValidIndex(Index))
    {
        return false;
    }

    CurrentProfile.ActivePresetIndex = Index;
    MarkDirty();
    return true;
}

FMGControlPreset UMGProfileManagerSubsystem::GetActiveControlPreset() const
{
    if (CurrentProfile.ControlPresets.IsValidIndex(CurrentProfile.ActivePresetIndex))
    {
        return CurrentProfile.ControlPresets[CurrentProfile.ActivePresetIndex];
    }
    return FMGControlPreset();
}

TArray<FMGControlPreset> UMGProfileManagerSubsystem::GetAllControlPresets() const
{
    return CurrentProfile.ControlPresets;
}

bool UMGProfileManagerSubsystem::AddFriend(const FMGSocialConnection& Friend)
{
    if (!bHasLoadedProfile || Friend.PlayerId.IsEmpty())
    {
        return false;
    }

    // Check if already a friend
    for (const FMGSocialConnection& Existing : CurrentProfile.Friends)
    {
        if (Existing.PlayerId == Friend.PlayerId)
        {
            return false;
        }
    }

    // Remove from blocked if present
    CurrentProfile.BlockedPlayers.Remove(Friend.PlayerId);

    CurrentProfile.Friends.Add(Friend);
    MarkDirty();
    return true;
}

bool UMGProfileManagerSubsystem::RemoveFriend(const FString& PlayerId)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    for (int32 i = CurrentProfile.Friends.Num() - 1; i >= 0; --i)
    {
        if (CurrentProfile.Friends[i].PlayerId == PlayerId)
        {
            CurrentProfile.Friends.RemoveAt(i);
            MarkDirty();
            return true;
        }
    }

    return false;
}

bool UMGProfileManagerSubsystem::BlockPlayer(const FString& PlayerId)
{
    if (!bHasLoadedProfile || PlayerId.IsEmpty())
    {
        return false;
    }

    // Remove from friends first
    RemoveFriend(PlayerId);

    if (!CurrentProfile.BlockedPlayers.Contains(PlayerId))
    {
        CurrentProfile.BlockedPlayers.Add(PlayerId);
        MarkDirty();
    }

    return true;
}

bool UMGProfileManagerSubsystem::UnblockPlayer(const FString& PlayerId)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    if (CurrentProfile.BlockedPlayers.Remove(PlayerId) > 0)
    {
        MarkDirty();
        return true;
    }

    return false;
}

bool UMGProfileManagerSubsystem::SetFriendFavorite(const FString& PlayerId, bool bFavorite)
{
    if (!bHasLoadedProfile)
    {
        return false;
    }

    for (FMGSocialConnection& Friend : CurrentProfile.Friends)
    {
        if (Friend.PlayerId == PlayerId)
        {
            Friend.bIsFavorite = bFavorite;
            MarkDirty();
            return true;
        }
    }

    return false;
}

void UMGProfileManagerSubsystem::UpdateFriendStatus(const FString& PlayerId, EMGPlayerStatus NewStatus)
{
    for (FMGSocialConnection& Friend : CurrentProfile.Friends)
    {
        if (Friend.PlayerId == PlayerId)
        {
            EMGPlayerStatus OldStatus = Friend.Status;
            Friend.Status = NewStatus;

            if (NewStatus != EMGPlayerStatus::Offline)
            {
                Friend.LastOnline = FDateTime::Now();
            }

            if (OldStatus != NewStatus)
            {
                OnFriendStatusChanged.Broadcast(PlayerId, NewStatus);
            }

            return;
        }
    }
}

TArray<FMGSocialConnection> UMGProfileManagerSubsystem::GetFriends() const
{
    return CurrentProfile.Friends;
}

TArray<FMGSocialConnection> UMGProfileManagerSubsystem::GetOnlineFriends() const
{
    TArray<FMGSocialConnection> OnlineFriends;

    for (const FMGSocialConnection& Friend : CurrentProfile.Friends)
    {
        if (Friend.Status != EMGPlayerStatus::Offline &&
            Friend.Status != EMGPlayerStatus::Invisible &&
            !Friend.bIsBlocked)
        {
            OnlineFriends.Add(Friend);
        }
    }

    return OnlineFriends;
}

TArray<FMGSocialConnection> UMGProfileManagerSubsystem::GetFavoriteFriends() const
{
    TArray<FMGSocialConnection> Favorites;

    for (const FMGSocialConnection& Friend : CurrentProfile.Friends)
    {
        if (Friend.bIsFavorite && !Friend.bIsBlocked)
        {
            Favorites.Add(Friend);
        }
    }

    return Favorites;
}

bool UMGProfileManagerSubsystem::IsFriend(const FString& PlayerId) const
{
    for (const FMGSocialConnection& Friend : CurrentProfile.Friends)
    {
        if (Friend.PlayerId == PlayerId)
        {
            return true;
        }
    }
    return false;
}

bool UMGProfileManagerSubsystem::IsBlocked(const FString& PlayerId) const
{
    return CurrentProfile.BlockedPlayers.Contains(PlayerId);
}

void UMGProfileManagerSubsystem::UpdateSeasonalRanking(const FMGSeasonalRanking& Ranking)
{
    if (!bHasLoadedProfile || Ranking.SeasonId.IsEmpty())
    {
        return;
    }

    bool bFound = false;
    for (FMGSeasonalRanking& Existing : CurrentProfile.SeasonalRankings)
    {
        if (Existing.SeasonId == Ranking.SeasonId)
        {
            Existing = Ranking;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        CurrentProfile.SeasonalRankings.Add(Ranking);
    }

    MarkDirty();
}

FMGSeasonalRanking UMGProfileManagerSubsystem::GetCurrentSeasonRanking() const
{
    for (const FMGSeasonalRanking& Ranking : CurrentProfile.SeasonalRankings)
    {
        if (Ranking.bIsActive)
        {
            return Ranking;
        }
    }
    return FMGSeasonalRanking();
}

TArray<FMGSeasonalRanking> UMGProfileManagerSubsystem::GetAllSeasonalRankings() const
{
    return CurrentProfile.SeasonalRankings;
}

void UMGProfileManagerSubsystem::SetGlobalRank(int32 Rank)
{
    CurrentProfile.GlobalRank = Rank;
    MarkDirty();
}

void UMGProfileManagerSubsystem::UpdateReputation(EMGReputationLevel NewLevel)
{
    if (!bHasLoadedProfile)
    {
        return;
    }

    EMGReputationLevel OldLevel = CurrentProfile.Reputation;
    CurrentProfile.Reputation = NewLevel;

    if (OldLevel != NewLevel)
    {
        MarkDirty();
        OnReputationChanged.Broadcast(OldLevel, NewLevel);
    }
}

EMGReputationLevel UMGProfileManagerSubsystem::GetReputation() const
{
    return CurrentProfile.Reputation;
}

FMGProfileExport UMGProfileManagerSubsystem::ExportProfile() const
{
    FMGProfileExport Export;
    Export.Profile = CurrentProfile;
    Export.ExportVersion = TEXT("1.0");
    Export.ExportDate = FDateTime::Now();
    Export.Checksum = GenerateChecksum(CurrentProfile);
    Export.bIsEncrypted = false;
    return Export;
}

bool UMGProfileManagerSubsystem::ImportProfile(const FMGProfileExport& ExportData)
{
    if (!ValidateProfile(ExportData.Profile))
    {
        return false;
    }

    // Verify checksum
    FString ExpectedChecksum = GenerateChecksum(ExportData.Profile);
    if (ExpectedChecksum != ExportData.Checksum)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProfileManager: Import failed - checksum mismatch"));
        return false;
    }

    CurrentProfile = ExportData.Profile;
    CurrentProfile.LastLoginDate = FDateTime::Now();
    bHasLoadedProfile = true;
    MarkDirty();

    OnProfileLoaded.Broadcast(CurrentProfile);

    UE_LOG(LogTemp, Log, TEXT("ProfileManager: Profile imported successfully"));
    return true;
}

FString UMGProfileManagerSubsystem::ExportProfileToJson() const
{
    FMGProfileExport Export = ExportProfile();

    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(Export, JsonString);

    return JsonString;
}

bool UMGProfileManagerSubsystem::ImportProfileFromJson(const FString& JsonString)
{
    FMGProfileExport Export;

    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Export))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProfileManager: Failed to parse JSON"));
        return false;
    }

    return ImportProfile(Export);
}

void UMGProfileManagerSubsystem::UpdatePlaytime(float DeltaSeconds)
{
    if (!bHasLoadedProfile)
    {
        return;
    }

    CurrentProfile.TotalPlaytime += DeltaSeconds;
    // Don't mark dirty for every update - autosave will handle it
}

float UMGProfileManagerSubsystem::GetTotalPlaytime() const
{
    return CurrentProfile.TotalPlaytime;
}

FString UMGProfileManagerSubsystem::GetFormattedPlaytime() const
{
    float TotalSeconds = CurrentProfile.TotalPlaytime;

    int32 Hours = FMath::FloorToInt(TotalSeconds / 3600.0f);
    int32 Minutes = FMath::FloorToInt(FMath::Fmod(TotalSeconds, 3600.0f) / 60.0f);

    if (Hours > 0)
    {
        return FString::Printf(TEXT("%dh %dm"), Hours, Minutes);
    }
    else
    {
        return FString::Printf(TEXT("%dm"), Minutes);
    }
}

void UMGProfileManagerSubsystem::InitializeDefaultProfile()
{
    CurrentProfile.Level = 1;
    CurrentProfile.TotalExperience = 0;
    CurrentProfile.PrestigeLevel = 0;
    CurrentProfile.AchievementPoints = 0;
    CurrentProfile.SoftCurrency = 10000; // Starting money
    CurrentProfile.PremiumCurrency = 0;
    CurrentProfile.Status = EMGPlayerStatus::Online;
    CurrentProfile.Reputation = EMGReputationLevel::Unknown;
    CurrentProfile.MainDiscipline = EMGRacingDiscipline::AllRounder;
    CurrentProfile.GlobalRank = 0;
    CurrentProfile.Version = EMGProfileVersion::Current;

    InitializeDefaultStats();
    InitializeDefaultAchievements();
    InitializeDefaultControlPreset();
}

void UMGProfileManagerSubsystem::InitializeDefaultStats()
{
    // Racing stats
    RegisterCareerStat(TEXT("TotalRaces"), NSLOCTEXT("Profile", "TotalRaces", "Total Races"), true);
    RegisterCareerStat(TEXT("TotalWins"), NSLOCTEXT("Profile", "TotalWins", "Total Wins"), true);
    RegisterCareerStat(TEXT("TotalPodiums"), NSLOCTEXT("Profile", "TotalPodiums", "Total Podiums"), true);
    RegisterCareerStat(TEXT("TotalDNF"), NSLOCTEXT("Profile", "TotalDNF", "Did Not Finish"), false);
    RegisterCareerStat(TEXT("WinStreak"), NSLOCTEXT("Profile", "WinStreak", "Win Streak"), true);
    RegisterCareerStat(TEXT("BestWinStreak"), NSLOCTEXT("Profile", "BestWinStreak", "Best Win Streak"), true);

    // Distance stats
    RegisterCareerStat(TEXT("TotalDistance"), NSLOCTEXT("Profile", "TotalDistance", "Total Distance"), true);
    RegisterCareerStat(TEXT("TotalDriftDistance"), NSLOCTEXT("Profile", "TotalDriftDistance", "Drift Distance"), true);
    RegisterCareerStat(TEXT("TotalAirtime"), NSLOCTEXT("Profile", "TotalAirtime", "Total Airtime"), true);

    // Performance stats
    RegisterCareerStat(TEXT("TopSpeed"), NSLOCTEXT("Profile", "TopSpeed", "Top Speed"), true);
    RegisterCareerStat(TEXT("LongestDrift"), NSLOCTEXT("Profile", "LongestDrift", "Longest Drift"), true);
    RegisterCareerStat(TEXT("LongestJump"), NSLOCTEXT("Profile", "LongestJump", "Longest Jump"), true);
    RegisterCareerStat(TEXT("BestLapTime"), NSLOCTEXT("Profile", "BestLapTime", "Best Lap Time"), false);

    // Action stats
    RegisterCareerStat(TEXT("TotalDrifts"), NSLOCTEXT("Profile", "TotalDrifts", "Total Drifts"), true);
    RegisterCareerStat(TEXT("TotalNitroBoosts"), NSLOCTEXT("Profile", "TotalNitroBoosts", "Nitro Boosts Used"), true);
    RegisterCareerStat(TEXT("TotalTakedowns"), NSLOCTEXT("Profile", "TotalTakedowns", "Total Takedowns"), true);
    RegisterCareerStat(TEXT("TotalNearMisses"), NSLOCTEXT("Profile", "TotalNearMisses", "Near Misses"), true);
    RegisterCareerStat(TEXT("PerfectLaps"), NSLOCTEXT("Profile", "PerfectLaps", "Perfect Laps"), true);

    // Multiplayer stats
    RegisterCareerStat(TEXT("OnlineRaces"), NSLOCTEXT("Profile", "OnlineRaces", "Online Races"), true);
    RegisterCareerStat(TEXT("OnlineWins"), NSLOCTEXT("Profile", "OnlineWins", "Online Wins"), true);
    RegisterCareerStat(TEXT("RankedRaces"), NSLOCTEXT("Profile", "RankedRaces", "Ranked Races"), true);
    RegisterCareerStat(TEXT("RankedWins"), NSLOCTEXT("Profile", "RankedWins", "Ranked Wins"), true);

    // Economic stats
    RegisterCareerStat(TEXT("TotalEarnings"), NSLOCTEXT("Profile", "TotalEarnings", "Total Earnings"), true);
    RegisterCareerStat(TEXT("TotalSpent"), NSLOCTEXT("Profile", "TotalSpent", "Total Spent"), true);
    RegisterCareerStat(TEXT("VehiclesPurchased"), NSLOCTEXT("Profile", "VehiclesPurchased", "Vehicles Purchased"), true);
    RegisterCareerStat(TEXT("UpgradesPurchased"), NSLOCTEXT("Profile", "UpgradesPurchased", "Upgrades Purchased"), true);
}

void UMGProfileManagerSubsystem::InitializeDefaultAchievements()
{
    // Starter achievements
    FMGPlayerAchievement FirstRace;
    FirstRace.AchievementId = TEXT("FIRST_RACE");
    FirstRace.DisplayName = NSLOCTEXT("Achievements", "FirstRace", "First Timer");
    FirstRace.Description = NSLOCTEXT("Achievements", "FirstRaceDesc", "Complete your first race");
    FirstRace.Rarity = EMGAchievementRarity::Common;
    FirstRace.PointsValue = 10;
    FirstRace.TargetValue = 1.0f;
    RegisterAchievement(FirstRace);

    FMGPlayerAchievement FirstWin;
    FirstWin.AchievementId = TEXT("FIRST_WIN");
    FirstWin.DisplayName = NSLOCTEXT("Achievements", "FirstWin", "Winner's Circle");
    FirstWin.Description = NSLOCTEXT("Achievements", "FirstWinDesc", "Win your first race");
    FirstWin.Rarity = EMGAchievementRarity::Common;
    FirstWin.PointsValue = 20;
    FirstWin.TargetValue = 1.0f;
    RegisterAchievement(FirstWin);

    FMGPlayerAchievement DriftMaster;
    DriftMaster.AchievementId = TEXT("DRIFT_MASTER");
    DriftMaster.DisplayName = NSLOCTEXT("Achievements", "DriftMaster", "Drift Master");
    DriftMaster.Description = NSLOCTEXT("Achievements", "DriftMasterDesc", "Perform 1000 drifts");
    DriftMaster.Rarity = EMGAchievementRarity::Rare;
    DriftMaster.PointsValue = 50;
    DriftMaster.TargetValue = 1000.0f;
    RegisterAchievement(DriftMaster);

    FMGPlayerAchievement SpeedDemon;
    SpeedDemon.AchievementId = TEXT("SPEED_DEMON");
    SpeedDemon.DisplayName = NSLOCTEXT("Achievements", "SpeedDemon", "Speed Demon");
    SpeedDemon.Description = NSLOCTEXT("Achievements", "SpeedDemonDesc", "Reach 300 km/h");
    SpeedDemon.Rarity = EMGAchievementRarity::Uncommon;
    SpeedDemon.PointsValue = 30;
    SpeedDemon.TargetValue = 300.0f;
    RegisterAchievement(SpeedDemon);

    FMGPlayerAchievement NightOwl;
    NightOwl.AchievementId = TEXT("NIGHT_OWL");
    NightOwl.DisplayName = NSLOCTEXT("Achievements", "NightOwl", "Night Owl");
    NightOwl.Description = NSLOCTEXT("Achievements", "NightOwlDesc", "Win 50 night races");
    NightOwl.Rarity = EMGAchievementRarity::Epic;
    NightOwl.PointsValue = 75;
    NightOwl.TargetValue = 50.0f;
    RegisterAchievement(NightOwl);

    FMGPlayerAchievement Legendary;
    Legendary.AchievementId = TEXT("LEGENDARY");
    Legendary.DisplayName = NSLOCTEXT("Achievements", "Legendary", "Legendary");
    Legendary.Description = NSLOCTEXT("Achievements", "LegendaryDesc", "Reach maximum prestige level");
    Legendary.Rarity = EMGAchievementRarity::Legendary;
    Legendary.PointsValue = 200;
    Legendary.TargetValue = 1.0f;
    RegisterAchievement(Legendary);
}

void UMGProfileManagerSubsystem::InitializeDefaultControlPreset()
{
    FMGControlPreset DefaultPreset;
    DefaultPreset.PresetId = TEXT("DEFAULT");
    DefaultPreset.PresetName = TEXT("Default");
    DefaultPreset.bIsDefault = true;

    CurrentProfile.ControlPresets.Add(DefaultPreset);
    CurrentProfile.ActivePresetIndex = 0;
}

void UMGProfileManagerSubsystem::PerformAutoSave()
{
    if (bIsDirty && bHasLoadedProfile)
    {
        SaveProfile();
    }
}

bool UMGProfileManagerSubsystem::ValidateProfile(const FMGPlayerProfile& Profile) const
{
    // Basic validation
    if (Profile.PlayerId.IsEmpty())
    {
        return false;
    }

    if (Profile.Level < 1 || Profile.Level > MaxLevel)
    {
        return false;
    }

    if (Profile.PrestigeLevel < 0 || Profile.PrestigeLevel > PrestigeMaxLevel)
    {
        return false;
    }

    if (Profile.TotalExperience < 0 || Profile.SoftCurrency < 0 || Profile.PremiumCurrency < 0)
    {
        return false;
    }

    return true;
}

FString UMGProfileManagerSubsystem::GenerateChecksum(const FMGPlayerProfile& Profile) const
{
    // Generate a checksum based on key profile fields
    FString CheckData = FString::Printf(
        TEXT("%s|%s|%d|%lld|%d|%lld|%lld"),
        *Profile.PlayerId,
        *Profile.DisplayName,
        Profile.Level,
        Profile.TotalExperience,
        Profile.PrestigeLevel,
        Profile.SoftCurrency,
        Profile.PremiumCurrency
    );

    return FMD5::HashAnsiString(*CheckData);
}

int32 UMGProfileManagerSubsystem::CalculateLevelFromExperience(int64 Experience) const
{
    int32 Level = 1;

    while (Level < MaxLevel && GetExperienceForLevel(Level + 1) <= Experience)
    {
        Level++;
    }

    return Level;
}

void UMGProfileManagerSubsystem::CheckLevelUp(int64 OldExperience, int64 NewExperience)
{
    int32 OldLevel = CalculateLevelFromExperience(OldExperience);
    int32 NewLevel = CalculateLevelFromExperience(NewExperience);

    if (NewLevel > OldLevel)
    {
        CurrentProfile.Level = NewLevel;
        int64 XPGained = NewExperience - OldExperience;

        OnLevelUp.Broadcast(NewLevel, XPGained);

        UE_LOG(LogTemp, Log, TEXT("ProfileManager: Player leveled up to %d"), NewLevel);
    }
}

void UMGProfileManagerSubsystem::MarkDirty()
{
    bIsDirty = true;
}
