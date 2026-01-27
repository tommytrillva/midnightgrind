// MidnightGrind - Arcade Street Racing Game
// Modding Subsystem - Implementation

#include "Modding/MGModdingSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"

UMGModdingSubsystem::UMGModdingSubsystem()
{
}

void UMGModdingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Set up mods directory
    ModsDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Mods"));

    // Initialize sample mods for demonstration
    InitializeSampleMods();

    // Load installed mods
    LoadInstalledMods();

    // Set up periodic update check
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGModdingSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            UpdateCheckTimerHandle,
            [WeakThis]()
            {
                if (!WeakThis.IsValid())
                {
                    return;
                }
                // Check for mod updates periodically
                for (const FString& ModId : WeakThis->InstalledModIds)
                {
                    if (FMGModItem* Mod = WeakThis->AllMods.Find(ModId))
                    {
                        // Simulate update check
                        if (Mod->InstalledVersion.GetVersionString() != Mod->CurrentVersion.GetVersionString())
                        {
                            Mod->Status = EMGModStatus::UpdateAvailable;
                        }
                    }
                }
            },
            300.0f, // Check every 5 minutes
            true
        );
    }
}

void UMGModdingSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateCheckTimerHandle);
    }

    SaveModConfiguration();
    Super::Deinitialize();
}

// ===== Workshop Browse =====

void UMGModdingSubsystem::SearchMods(const FMGModSearchFilter& Filter)
{
    TArray<FMGModItem> Results;

    for (const auto& Pair : AllMods)
    {
        const FMGModItem& Mod = Pair.Value;

        // Apply filters
        if (!Filter.SearchQuery.IsEmpty())
        {
            FString SearchLower = Filter.SearchQuery.ToLower();
            if (!Mod.Title.ToString().ToLower().Contains(SearchLower) &&
                !Mod.Description.ToString().ToLower().Contains(SearchLower))
            {
                continue;
            }
        }

        if (Filter.ModTypes.Num() > 0 && !Filter.ModTypes.Contains(Mod.ModType))
        {
            continue;
        }

        if (static_cast<uint8>(Mod.Verification) < static_cast<uint8>(Filter.MinVerification))
        {
            continue;
        }

        if (static_cast<uint8>(Mod.ContentRating) > static_cast<uint8>(Filter.MaxContentRating))
        {
            continue;
        }

        if (Mod.Stats.AverageRating < Filter.MinRating)
        {
            continue;
        }

        Results.Add(Mod);
    }

    // Sort results
    if (Filter.SortBy == FName("Popular"))
    {
        Results.Sort([Filter](const FMGModItem& A, const FMGModItem& B)
        {
            return Filter.bSortDescending ? A.Stats.TotalDownloads > B.Stats.TotalDownloads :
                                            A.Stats.TotalDownloads < B.Stats.TotalDownloads;
        });
    }
    else if (Filter.SortBy == FName("Rating"))
    {
        Results.Sort([Filter](const FMGModItem& A, const FMGModItem& B)
        {
            return Filter.bSortDescending ? A.Stats.AverageRating > B.Stats.AverageRating :
                                            A.Stats.AverageRating < B.Stats.AverageRating;
        });
    }
    else if (Filter.SortBy == FName("Recent"))
    {
        Results.Sort([Filter](const FMGModItem& A, const FMGModItem& B)
        {
            return Filter.bSortDescending ? A.LastUpdated > B.LastUpdated :
                                            A.LastUpdated < B.LastUpdated;
        });
    }

    // Apply pagination
    int32 StartIndex = Filter.PageNumber * Filter.PageSize;
    int32 EndIndex = FMath::Min(StartIndex + Filter.PageSize, Results.Num());

    TArray<FMGModItem> PagedResults;
    for (int32 i = StartIndex; i < EndIndex; i++)
    {
        PagedResults.Add(Results[i]);
    }

    OnModSearchComplete.Broadcast(PagedResults);
}

void UMGModdingSubsystem::GetFeaturedMods()
{
    TArray<FMGModItem> Featured;

    for (const auto& Pair : AllMods)
    {
        if (Pair.Value.Verification == EMGModVerification::Featured ||
            Pair.Value.Verification == EMGModVerification::Staff)
        {
            Featured.Add(Pair.Value);
        }
    }

    OnModSearchComplete.Broadcast(Featured);
}

void UMGModdingSubsystem::GetPopularMods(int32 Count)
{
    TArray<FMGModItem> AllModsList;
    for (const auto& Pair : AllMods)
    {
        AllModsList.Add(Pair.Value);
    }

    AllModsList.Sort([](const FMGModItem& A, const FMGModItem& B)
    {
        return A.Stats.TotalDownloads > B.Stats.TotalDownloads;
    });

    TArray<FMGModItem> Popular;
    for (int32 i = 0; i < FMath::Min(Count, AllModsList.Num()); i++)
    {
        Popular.Add(AllModsList[i]);
    }

    OnModSearchComplete.Broadcast(Popular);
}

void UMGModdingSubsystem::GetRecentMods(int32 Count)
{
    TArray<FMGModItem> AllModsList;
    for (const auto& Pair : AllMods)
    {
        AllModsList.Add(Pair.Value);
    }

    AllModsList.Sort([](const FMGModItem& A, const FMGModItem& B)
    {
        return A.LastUpdated > B.LastUpdated;
    });

    TArray<FMGModItem> Recent;
    for (int32 i = 0; i < FMath::Min(Count, AllModsList.Num()); i++)
    {
        Recent.Add(AllModsList[i]);
    }

    OnModSearchComplete.Broadcast(Recent);
}

void UMGModdingSubsystem::GetModDetails(const FString& ModId)
{
    // In production, this would fetch from server
    // For now, return cached data
}

FMGModItem UMGModdingSubsystem::GetCachedModDetails(const FString& ModId) const
{
    if (const FMGModItem* Mod = AllMods.Find(ModId))
    {
        return *Mod;
    }
    return FMGModItem();
}

// ===== Installation =====

bool UMGModdingSubsystem::SubscribeToMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    if (SubscribedModIds.Contains(ModId))
    {
        return false;
    }

    SubscribedModIds.Add(ModId);
    Mod->bIsSubscribed = true;
    Mod->Stats.UniqueSubscribers++;

    // Automatically start download
    DownloadMod(ModId);
    return true;
}

bool UMGModdingSubsystem::UnsubscribeFromMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    SubscribedModIds.Remove(ModId);
    Mod->bIsSubscribed = false;
    Mod->Stats.UniqueSubscribers = FMath::Max(0, Mod->Stats.UniqueSubscribers - 1);

    // Optionally uninstall
    if (IsModInstalled(ModId))
    {
        UninstallMod(ModId);
    }

    return true;
}

void UMGModdingSubsystem::DownloadMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return;
    }

    Mod->Status = EMGModStatus::Downloading;

    // Create download progress tracker
    FMGModDownloadProgress Progress;
    Progress.ModId = ModId;
    Progress.TotalBytes = Mod->FileSizeBytes;
    Progress.BytesDownloaded = 0;
    ActiveDownloads.Add(ModId, Progress);

    // Simulate download with timer
    if (UWorld* World = GetWorld())
    {
        FTimerHandle DownloadTimer;
        TWeakObjectPtr<UMGModdingSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DownloadTimer,
            [WeakThis, ModId]()
            {
                if (!WeakThis.IsValid())
                {
                    return;
                }
                FMGModDownloadProgress* Progress = WeakThis->ActiveDownloads.Find(ModId);
                if (Progress)
                {
                    // Simulate progress
                    Progress->BytesDownloaded += Progress->TotalBytes / 10;
                    Progress->DownloadSpeedBps = 5 * 1024 * 1024; // 5 MB/s
                    Progress->EstimatedTimeRemaining = (Progress->TotalBytes - Progress->BytesDownloaded) / Progress->DownloadSpeedBps;

                    WeakThis->OnModDownloadProgress.Broadcast(*Progress);

                    if (Progress->BytesDownloaded >= Progress->TotalBytes)
                    {
                        WeakThis->ActiveDownloads.Remove(ModId);
                        WeakThis->InstallMod(ModId);
                        WeakThis->OnModDownloadComplete.Broadcast(ModId, true);
                    }
                }
            },
            0.2f, // Simulate quick download
            true
        );
    }
}

bool UMGModdingSubsystem::InstallMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    // Check dependencies
    if (!AreDependenciesSatisfied(ModId))
    {
        InstallMissingDependencies(ModId);
    }

    Mod->Status = EMGModStatus::Installing;

    // Simulate installation
    Mod->LocalPath = GetModInstallPath(ModId);
    Mod->InstalledVersion = Mod->CurrentVersion;
    Mod->Status = EMGModStatus::Installed;

    InstalledModIds.AddUnique(ModId);
    Mod->Stats.TotalDownloads++;

    // Check for conflicts
    CheckForConflicts();

    OnModInstalled.Broadcast(*Mod);
    OnModListChanged.Broadcast();

    return true;
}

bool UMGModdingSubsystem::UninstallMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    if (!InstalledModIds.Contains(ModId))
    {
        return false;
    }

    // Disable first if enabled
    if (EnabledModIds.Contains(ModId))
    {
        DisableMod(ModId);
    }

    InstalledModIds.Remove(ModId);
    Mod->Status = EMGModStatus::NotInstalled;
    Mod->LocalPath = TEXT("");

    // Remove conflicts
    DetectedConflicts.RemoveAll([ModId](const FMGModConflict& Conflict)
    {
        return Conflict.ModIdA == ModId || Conflict.ModIdB == ModId;
    });

    OnModUninstalled.Broadcast(ModId);
    OnModListChanged.Broadcast();

    return true;
}

void UMGModdingSubsystem::UpdateMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod || Mod->Status != EMGModStatus::UpdateAvailable)
    {
        return;
    }

    // Download and install update
    DownloadMod(ModId);
}

void UMGModdingSubsystem::UpdateAllMods()
{
    for (const FString& ModId : GetModsWithUpdates())
    {
        UpdateMod(ModId);
    }
}

TArray<FString> UMGModdingSubsystem::GetModsWithUpdates() const
{
    TArray<FString> ModsWithUpdates;

    for (const FString& ModId : InstalledModIds)
    {
        if (const FMGModItem* Mod = AllMods.Find(ModId))
        {
            if (Mod->Status == EMGModStatus::UpdateAvailable)
            {
                ModsWithUpdates.Add(ModId);
            }
        }
    }

    return ModsWithUpdates;
}

// ===== Mod Management =====

bool UMGModdingSubsystem::EnableMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod || !InstalledModIds.Contains(ModId))
    {
        return false;
    }

    if (EnabledModIds.Contains(ModId))
    {
        return false;
    }

    // Check dependencies
    if (!AreDependenciesSatisfied(ModId))
    {
        return false;
    }

    EnabledModIds.Add(ModId);
    Mod->Status = EMGModStatus::Enabled;
    Mod->LoadOrder = EnabledModIds.Num();

    SortModsByLoadOrder();
    CheckForConflicts();

    OnModEnabled.Broadcast(ModId, true);
    OnModListChanged.Broadcast();

    return true;
}

bool UMGModdingSubsystem::DisableMod(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    if (!EnabledModIds.Contains(ModId))
    {
        return false;
    }

    EnabledModIds.Remove(ModId);
    Mod->Status = EMGModStatus::Disabled;

    // Update load orders
    for (int32 i = 0; i < EnabledModIds.Num(); i++)
    {
        if (FMGModItem* EnabledMod = AllMods.Find(EnabledModIds[i]))
        {
            EnabledMod->LoadOrder = i + 1;
        }
    }

    OnModEnabled.Broadcast(ModId, false);
    OnModListChanged.Broadcast();

    return true;
}

void UMGModdingSubsystem::SetModLoadOrder(const FString& ModId, int32 LoadOrder)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod || !EnabledModIds.Contains(ModId))
    {
        return;
    }

    int32 ClampedOrder = FMath::Clamp(LoadOrder, 1, EnabledModIds.Num());
    int32 OldIndex = EnabledModIds.IndexOfByKey(ModId);

    if (OldIndex != INDEX_NONE)
    {
        EnabledModIds.RemoveAt(OldIndex);
        EnabledModIds.Insert(ModId, ClampedOrder - 1);
    }

    SortModsByLoadOrder();
    CheckForConflicts();
    OnModListChanged.Broadcast();
}

void UMGModdingSubsystem::MoveModUp(const FString& ModId)
{
    int32 Index = EnabledModIds.IndexOfByKey(ModId);
    if (Index > 0)
    {
        SetModLoadOrder(ModId, Index); // Move up = lower index
    }
}

void UMGModdingSubsystem::MoveModDown(const FString& ModId)
{
    int32 Index = EnabledModIds.IndexOfByKey(ModId);
    if (Index >= 0 && Index < EnabledModIds.Num() - 1)
    {
        SetModLoadOrder(ModId, Index + 2); // Move down = higher index
    }
}

TArray<FMGModItem> UMGModdingSubsystem::GetInstalledMods() const
{
    TArray<FMGModItem> Installed;

    for (const FString& ModId : InstalledModIds)
    {
        if (const FMGModItem* Mod = AllMods.Find(ModId))
        {
            Installed.Add(*Mod);
        }
    }

    return Installed;
}

TArray<FMGModItem> UMGModdingSubsystem::GetEnabledMods() const
{
    TArray<FMGModItem> Enabled;

    for (const FString& ModId : EnabledModIds)
    {
        if (const FMGModItem* Mod = AllMods.Find(ModId))
        {
            Enabled.Add(*Mod);
        }
    }

    // Sort by load order
    Enabled.Sort([](const FMGModItem& A, const FMGModItem& B)
    {
        return A.LoadOrder < B.LoadOrder;
    });

    return Enabled;
}

TArray<FMGModItem> UMGModdingSubsystem::GetSubscribedMods() const
{
    TArray<FMGModItem> Subscribed;

    for (const FString& ModId : SubscribedModIds)
    {
        if (const FMGModItem* Mod = AllMods.Find(ModId))
        {
            Subscribed.Add(*Mod);
        }
    }

    return Subscribed;
}

bool UMGModdingSubsystem::IsModInstalled(const FString& ModId) const
{
    return InstalledModIds.Contains(ModId);
}

bool UMGModdingSubsystem::IsModEnabled(const FString& ModId) const
{
    return EnabledModIds.Contains(ModId);
}

// ===== Dependencies =====

TArray<FMGModDependency> UMGModdingSubsystem::GetModDependencies(const FString& ModId) const
{
    if (const FMGModItem* Mod = AllMods.Find(ModId))
    {
        return Mod->Dependencies;
    }
    return TArray<FMGModDependency>();
}

bool UMGModdingSubsystem::AreDependenciesSatisfied(const FString& ModId) const
{
    const FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    for (const FMGModDependency& Dep : Mod->Dependencies)
    {
        if (!Dep.bIsOptional && !Dep.bIsSatisfied)
        {
            if (!IsModInstalled(Dep.ModId))
            {
                return false;
            }
        }
    }

    return true;
}

void UMGModdingSubsystem::InstallMissingDependencies(const FString& ModId)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return;
    }

    for (FMGModDependency& Dep : Mod->Dependencies)
    {
        if (!Dep.bIsOptional && !IsModInstalled(Dep.ModId))
        {
            SubscribeToMod(Dep.ModId);
        }
    }
}

// ===== Conflicts =====

TArray<FMGModConflict> UMGModdingSubsystem::GetAllConflicts() const
{
    return DetectedConflicts;
}

TArray<FMGModConflict> UMGModdingSubsystem::GetConflictsForMod(const FString& ModId) const
{
    TArray<FMGModConflict> ModConflicts;

    for (const FMGModConflict& Conflict : DetectedConflicts)
    {
        if (Conflict.ModIdA == ModId || Conflict.ModIdB == ModId)
        {
            ModConflicts.Add(Conflict);
        }
    }

    return ModConflicts;
}

bool UMGModdingSubsystem::ResolveConflict(const FMGModConflict& Conflict)
{
    if (!Conflict.bCanResolve)
    {
        return false;
    }

    // For load order conflicts, adjust the order
    if (Conflict.ConflictType == FName("LoadOrder"))
    {
        // Move ModIdB after ModIdA
        int32 IndexA = EnabledModIds.IndexOfByKey(Conflict.ModIdA);
        int32 IndexB = EnabledModIds.IndexOfByKey(Conflict.ModIdB);

        if (IndexA != INDEX_NONE && IndexB != INDEX_NONE && IndexB < IndexA)
        {
            SetModLoadOrder(Conflict.ModIdB, IndexA + 2);
        }
    }

    // Remove the conflict
    DetectedConflicts.RemoveAll([Conflict](const FMGModConflict& C)
    {
        return C.ModIdA == Conflict.ModIdA && C.ModIdB == Conflict.ModIdB;
    });

    return true;
}

// ===== Collections =====

void UMGModdingSubsystem::GetCollection(const FString& CollectionId)
{
    // In production, would fetch from server
}

bool UMGModdingSubsystem::SubscribeToCollection(const FString& CollectionId)
{
    FMGWorkshopCollection* Collection = Collections.Find(CollectionId);
    if (!Collection)
    {
        return false;
    }

    // Subscribe to all mods in collection
    for (const FString& ModId : Collection->ModIds)
    {
        SubscribeToMod(ModId);
    }

    Collection->SubscriberCount++;
    return true;
}

FMGWorkshopCollection UMGModdingSubsystem::CreateCollection(const FText& Title, const FText& Description)
{
    FMGWorkshopCollection NewCollection;
    NewCollection.CollectionId = FGuid::NewGuid().ToString();
    NewCollection.Title = Title;
    NewCollection.Description = Description;
    NewCollection.CreatedDate = FDateTime::Now();
    NewCollection.LastUpdated = FDateTime::Now();

    Collections.Add(NewCollection.CollectionId, NewCollection);
    return NewCollection;
}

bool UMGModdingSubsystem::AddModToCollection(const FString& CollectionId, const FString& ModId)
{
    FMGWorkshopCollection* Collection = Collections.Find(CollectionId);
    if (!Collection)
    {
        return false;
    }

    if (!AllMods.Contains(ModId))
    {
        return false;
    }

    Collection->ModIds.AddUnique(ModId);
    Collection->LastUpdated = FDateTime::Now();
    return true;
}

// ===== Ratings =====

bool UMGModdingSubsystem::RateMod(const FString& ModId, bool bPositive)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    // Remove old rating if exists
    if (Mod->UserRating != 0)
    {
        if (Mod->UserRating > 0)
        {
            Mod->Stats.PositiveRatings--;
        }
        else
        {
            Mod->Stats.NegativeRatings--;
        }
        Mod->Stats.TotalRatings--;
    }

    // Apply new rating
    Mod->UserRating = bPositive ? 1 : -1;
    if (bPositive)
    {
        Mod->Stats.PositiveRatings++;
    }
    else
    {
        Mod->Stats.NegativeRatings++;
    }
    Mod->Stats.TotalRatings++;

    // Recalculate average
    Mod->Stats.AverageRating = Mod->Stats.GetPositivePercent() / 20.0f; // Convert to 0-5 scale

    return true;
}

bool UMGModdingSubsystem::FavoriteMod(const FString& ModId, bool bFavorite)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    if (Mod->bIsFavorited == bFavorite)
    {
        return false;
    }

    Mod->bIsFavorited = bFavorite;
    Mod->Stats.TotalFavorites += bFavorite ? 1 : -1;
    Mod->Stats.TotalFavorites = FMath::Max(0, Mod->Stats.TotalFavorites);

    return true;
}

void UMGModdingSubsystem::ReportMod(const FString& ModId, FName ReportReason, const FString& Details)
{
    // In production, this would send report to server
}

// ===== User Creations =====

bool UMGModdingSubsystem::ExportVinyl(const FString& VinylId, const FString& ExportPath)
{
    // In production, this would export vinyl data to file
    return true;
}

bool UMGModdingSubsystem::ExportTrack(const FString& TrackId, const FString& ExportPath)
{
    // In production, this would export track data to file
    return true;
}

bool UMGModdingSubsystem::UploadMod(const FMGModItem& ModInfo, const FString& ContentPath)
{
    // In production, this would upload to workshop
    FMGModItem NewMod = ModInfo;
    NewMod.ModId = FGuid::NewGuid().ToString();
    NewMod.CreatedDate = FDateTime::Now();
    NewMod.LastUpdated = FDateTime::Now();
    NewMod.Verification = EMGModVerification::Pending;

    AllMods.Add(NewMod.ModId, NewMod);
    return true;
}

bool UMGModdingSubsystem::UpdateUploadedMod(const FString& ModId, const FMGModVersion& NewVersion, const FString& ContentPath)
{
    FMGModItem* Mod = AllMods.Find(ModId);
    if (!Mod)
    {
        return false;
    }

    Mod->CurrentVersion = NewVersion;
    Mod->LastUpdated = FDateTime::Now();

    // Notify subscribers
    OnModUpdated.Broadcast(ModId, NewVersion);
    return true;
}

// ===== Protected =====

void UMGModdingSubsystem::InitializeSampleMods()
{
    FDateTime Now = FDateTime::Now();

    // Sample Vehicle Mod
    {
        FMGModItem VehicleMod;
        VehicleMod.ModId = TEXT("mod_vehicle_nissan_r34");
        VehicleMod.Title = FText::FromString(TEXT("Nissan Skyline R34 GT-R"));
        VehicleMod.ShortDescription = FText::FromString(TEXT("High-quality Nissan Skyline R34 with customizable parts"));
        VehicleMod.Description = FText::FromString(TEXT("The legendary Nissan Skyline R34 GT-R, faithfully recreated with attention to detail. Includes multiple body kit options, wheel choices, and engine swap options. Features RB26DETT engine sounds."));
        VehicleMod.ModType = EMGModType::Vehicle;
        VehicleMod.Status = EMGModStatus::NotInstalled;
        VehicleMod.Verification = EMGModVerification::Featured;
        VehicleMod.ContentRating = EMGModRating::Everyone;
        VehicleMod.Author.DisplayName = TEXT("JDM_Legends");
        VehicleMod.Author.AuthorId = TEXT("author_jdm_001");
        VehicleMod.Author.bIsVerified = true;
        VehicleMod.Author.TotalMods = 15;
        VehicleMod.Author.TotalDownloads = 250000;
        VehicleMod.CurrentVersion.Major = 2;
        VehicleMod.CurrentVersion.Minor = 1;
        VehicleMod.CurrentVersion.Patch = 0;
        VehicleMod.CurrentVersion.ChangeLog = TEXT("Added new widebody kit option");
        VehicleMod.CurrentVersion.ReleaseDate = Now - FTimespan::FromDays(7);
        VehicleMod.Stats.TotalDownloads = 85000;
        VehicleMod.Stats.UniqueSubscribers = 42000;
        VehicleMod.Stats.AverageRating = 4.8f;
        VehicleMod.Stats.TotalRatings = 1250;
        VehicleMod.Stats.PositiveRatings = 1200;
        VehicleMod.Stats.TotalFavorites = 8500;
        VehicleMod.Stats.CurrentRank = 3;
        VehicleMod.Tags = { TEXT("JDM"), TEXT("Nissan"), TEXT("GT-R"), TEXT("Vehicle") };
        VehicleMod.FileSizeBytes = 150 * 1024 * 1024;
        VehicleMod.CreatedDate = Now - FTimespan::FromDays(180);
        VehicleMod.LastUpdated = Now - FTimespan::FromDays(7);

        AllMods.Add(VehicleMod.ModId, VehicleMod);
    }

    // Sample Track Mod
    {
        FMGModItem TrackMod;
        TrackMod.ModId = TEXT("mod_track_tokyo_highway");
        TrackMod.Title = FText::FromString(TEXT("Tokyo Highway Loop"));
        TrackMod.ShortDescription = FText::FromString(TEXT("Night-time Tokyo highway racing"));
        TrackMod.Description = FText::FromString(TEXT("Race through the neon-lit highways of Tokyo at night. Features multiple routes, heavy traffic mode, and police chase events. Inspired by classic street racing culture."));
        TrackMod.ModType = EMGModType::Track;
        TrackMod.Status = EMGModStatus::NotInstalled;
        TrackMod.Verification = EMGModVerification::Staff;
        TrackMod.ContentRating = EMGModRating::Everyone;
        TrackMod.Author.DisplayName = TEXT("TrackMaster_Pro");
        TrackMod.Author.AuthorId = TEXT("author_track_001");
        TrackMod.Author.bIsVerified = true;
        TrackMod.Author.TotalMods = 8;
        TrackMod.Author.TotalDownloads = 180000;
        TrackMod.CurrentVersion.Major = 1;
        TrackMod.CurrentVersion.Minor = 5;
        TrackMod.CurrentVersion.Patch = 2;
        TrackMod.Stats.TotalDownloads = 120000;
        TrackMod.Stats.UniqueSubscribers = 65000;
        TrackMod.Stats.AverageRating = 4.9f;
        TrackMod.Stats.TotalRatings = 2100;
        TrackMod.Stats.PositiveRatings = 2050;
        TrackMod.Stats.TotalFavorites = 15000;
        TrackMod.Stats.CurrentRank = 1;
        TrackMod.Tags = { TEXT("Track"), TEXT("Tokyo"), TEXT("Night"), TEXT("Highway") };
        TrackMod.FileSizeBytes = 450 * 1024 * 1024;
        TrackMod.CreatedDate = Now - FTimespan::FromDays(120);
        TrackMod.LastUpdated = Now - FTimespan::FromDays(14);

        AllMods.Add(TrackMod.ModId, TrackMod);
    }

    // Sample Vinyl Pack
    {
        FMGModItem VinylMod;
        VinylMod.ModId = TEXT("mod_vinyl_racing_legends");
        VinylMod.Title = FText::FromString(TEXT("Racing Legends Vinyl Pack"));
        VinylMod.ShortDescription = FText::FromString(TEXT("50+ iconic racing liveries"));
        VinylMod.Description = FText::FromString(TEXT("A collection of 50+ iconic racing liveries from motorsport history. Includes classic Gulf, Martini, Rothmans, and more. Compatible with all vehicles."));
        VinylMod.ModType = EMGModType::Vinyl;
        VinylMod.Status = EMGModStatus::NotInstalled;
        VinylMod.Verification = EMGModVerification::Verified;
        VinylMod.ContentRating = EMGModRating::Everyone;
        VinylMod.Author.DisplayName = TEXT("VinylArtist_X");
        VinylMod.Author.AuthorId = TEXT("author_vinyl_001");
        VinylMod.Author.bIsVerified = false;
        VinylMod.CurrentVersion.Major = 3;
        VinylMod.CurrentVersion.Minor = 0;
        VinylMod.CurrentVersion.Patch = 0;
        VinylMod.Stats.TotalDownloads = 45000;
        VinylMod.Stats.UniqueSubscribers = 22000;
        VinylMod.Stats.AverageRating = 4.6f;
        VinylMod.Stats.TotalRatings = 800;
        VinylMod.Stats.PositiveRatings = 720;
        VinylMod.Stats.TotalFavorites = 3500;
        VinylMod.Stats.CurrentRank = 12;
        VinylMod.Tags = { TEXT("Vinyl"), TEXT("Livery"), TEXT("Racing"), TEXT("Classic") };
        VinylMod.FileSizeBytes = 80 * 1024 * 1024;
        VinylMod.CreatedDate = Now - FTimespan::FromDays(90);
        VinylMod.LastUpdated = Now - FTimespan::FromDays(30);

        AllMods.Add(VinylMod.ModId, VinylMod);
    }

    // Sample Wheel Pack
    {
        FMGModItem WheelMod;
        WheelMod.ModId = TEXT("mod_wheels_work_collection");
        WheelMod.Title = FText::FromString(TEXT("Work Wheels Collection"));
        WheelMod.ShortDescription = FText::FromString(TEXT("Premium Japanese wheel designs"));
        WheelMod.Description = FText::FromString(TEXT("A premium collection of Work Wheels designs including Meister, Emotion, and Equip series. High-quality models with accurate spoke designs and multiple finish options."));
        WheelMod.ModType = EMGModType::Wheels;
        WheelMod.Status = EMGModStatus::NotInstalled;
        WheelMod.Verification = EMGModVerification::Verified;
        WheelMod.ContentRating = EMGModRating::Everyone;
        WheelMod.Author.DisplayName = TEXT("WheelWizard");
        WheelMod.CurrentVersion.Major = 1;
        WheelMod.CurrentVersion.Minor = 2;
        WheelMod.CurrentVersion.Patch = 1;
        WheelMod.Stats.TotalDownloads = 32000;
        WheelMod.Stats.AverageRating = 4.7f;
        WheelMod.Stats.TotalRatings = 450;
        WheelMod.Stats.CurrentRank = 18;
        WheelMod.Tags = { TEXT("Wheels"), TEXT("JDM"), TEXT("Work"), TEXT("Premium") };
        WheelMod.FileSizeBytes = 45 * 1024 * 1024;
        WheelMod.CreatedDate = Now - FTimespan::FromDays(60);
        WheelMod.LastUpdated = Now - FTimespan::FromDays(21);

        AllMods.Add(WheelMod.ModId, WheelMod);
    }

    // Sample Audio Mod
    {
        FMGModItem AudioMod;
        AudioMod.ModId = TEXT("mod_audio_eurobeat_pack");
        AudioMod.Title = FText::FromString(TEXT("Eurobeat Music Pack"));
        AudioMod.ShortDescription = FText::FromString(TEXT("High-energy racing soundtrack"));
        AudioMod.Description = FText::FromString(TEXT("30 high-energy Eurobeat tracks for the ultimate racing experience. Features original compositions inspired by classic 90s racing anime soundtracks."));
        AudioMod.ModType = EMGModType::Audio;
        AudioMod.Status = EMGModStatus::NotInstalled;
        AudioMod.Verification = EMGModVerification::Verified;
        AudioMod.ContentRating = EMGModRating::Everyone;
        AudioMod.Author.DisplayName = TEXT("EurobeatKing");
        AudioMod.CurrentVersion.Major = 2;
        AudioMod.CurrentVersion.Minor = 0;
        AudioMod.CurrentVersion.Patch = 0;
        AudioMod.Stats.TotalDownloads = 55000;
        AudioMod.Stats.AverageRating = 4.5f;
        AudioMod.Stats.TotalRatings = 920;
        AudioMod.Stats.CurrentRank = 8;
        AudioMod.Tags = { TEXT("Audio"), TEXT("Music"), TEXT("Eurobeat"), TEXT("Soundtrack") };
        AudioMod.FileSizeBytes = 200 * 1024 * 1024;
        AudioMod.CreatedDate = Now - FTimespan::FromDays(150);
        AudioMod.LastUpdated = Now - FTimespan::FromDays(45);

        AllMods.Add(AudioMod.ModId, AudioMod);
    }

    // Sample UI Theme
    {
        FMGModItem UIMod;
        UIMod.ModId = TEXT("mod_ui_retro_arcade");
        UIMod.Title = FText::FromString(TEXT("Retro Arcade UI Theme"));
        UIMod.ShortDescription = FText::FromString(TEXT("80s arcade-inspired UI overhaul"));
        UIMod.Description = FText::FromString(TEXT("Transform your game UI with this 80s arcade-inspired theme. Features neon colors, scan lines, CRT effects, and pixel art icons. Perfect for the retro gaming experience."));
        UIMod.ModType = EMGModType::UITheme;
        UIMod.Status = EMGModStatus::NotInstalled;
        UIMod.Verification = EMGModVerification::Verified;
        UIMod.ContentRating = EMGModRating::Everyone;
        UIMod.Author.DisplayName = TEXT("RetroUIDesigner");
        UIMod.CurrentVersion.Major = 1;
        UIMod.CurrentVersion.Minor = 1;
        UIMod.CurrentVersion.Patch = 0;
        UIMod.Stats.TotalDownloads = 28000;
        UIMod.Stats.AverageRating = 4.4f;
        UIMod.Stats.TotalRatings = 380;
        UIMod.Stats.CurrentRank = 25;
        UIMod.Tags = { TEXT("UI"), TEXT("Theme"), TEXT("Retro"), TEXT("Arcade") };
        UIMod.FileSizeBytes = 35 * 1024 * 1024;
        UIMod.CreatedDate = Now - FTimespan::FromDays(75);
        UIMod.LastUpdated = Now - FTimespan::FromDays(40);

        AllMods.Add(UIMod.ModId, UIMod);
    }
}

void UMGModdingSubsystem::LoadInstalledMods()
{
    // In production, would load from save file and verify files exist
}

void UMGModdingSubsystem::SaveModConfiguration()
{
    // In production, would save to file
}

void UMGModdingSubsystem::CheckForConflicts()
{
    DetectedConflicts.Empty();

    // Check for conflicts between enabled mods
    for (int32 i = 0; i < EnabledModIds.Num(); i++)
    {
        for (int32 j = i + 1; j < EnabledModIds.Num(); j++)
        {
            const FMGModItem* ModA = AllMods.Find(EnabledModIds[i]);
            const FMGModItem* ModB = AllMods.Find(EnabledModIds[j]);

            if (!ModA || !ModB)
            {
                continue;
            }

            // Check for same type conflicts (e.g., two UI themes)
            if (ModA->ModType == ModB->ModType && ModA->ModType == EMGModType::UITheme)
            {
                FMGModConflict Conflict;
                Conflict.ModIdA = ModA->ModId;
                Conflict.ModIdB = ModB->ModId;
                Conflict.ConflictDescription = FText::FromString(TEXT("Multiple UI themes cannot be active simultaneously"));
                Conflict.ConflictType = FName("TypeConflict");
                Conflict.bCanResolve = true;
                DetectedConflicts.Add(Conflict);
                OnModConflictDetected.Broadcast(Conflict);
            }
        }
    }
}

void UMGModdingSubsystem::SortModsByLoadOrder()
{
    for (int32 i = 0; i < EnabledModIds.Num(); i++)
    {
        if (FMGModItem* Mod = AllMods.Find(EnabledModIds[i]))
        {
            Mod->LoadOrder = i + 1;
        }
    }
}

FString UMGModdingSubsystem::GetModInstallPath(const FString& ModId) const
{
    return FPaths::Combine(ModsDirectory, ModId);
}
