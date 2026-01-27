// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Modding Subsystem - User-generated content, custom tracks, mods, and workshop integration

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGModdingSubsystem.generated.h"

// Forward declarations
class UMGModdingSubsystem;

/**
 * EMGModType - Types of mods
 */
UENUM(BlueprintType)
enum class EMGModType : uint8
{
    Vehicle         UMETA(DisplayName = "Vehicle"),
    Track           UMETA(DisplayName = "Track"),
    Vinyl           UMETA(DisplayName = "Vinyl/Livery"),
    Decal           UMETA(DisplayName = "Decal"),
    Wheels          UMETA(DisplayName = "Wheels"),
    BodyKit         UMETA(DisplayName = "Body Kit"),
    Interior        UMETA(DisplayName = "Interior"),
    Environment     UMETA(DisplayName = "Environment"),
    Audio           UMETA(DisplayName = "Audio"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    UITheme         UMETA(DisplayName = "UI Theme"),
    TotalConversion UMETA(DisplayName = "Total Conversion")
};

/**
 * EMGModStatus - Status of a mod
 */
UENUM(BlueprintType)
enum class EMGModStatus : uint8
{
    NotInstalled    UMETA(DisplayName = "Not Installed"),
    Downloading     UMETA(DisplayName = "Downloading"),
    Installing      UMETA(DisplayName = "Installing"),
    Installed       UMETA(DisplayName = "Installed"),
    Enabled         UMETA(DisplayName = "Enabled"),
    Disabled        UMETA(DisplayName = "Disabled"),
    UpdateAvailable UMETA(DisplayName = "Update Available"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * EMGModVerification - Verification status
 */
UENUM(BlueprintType)
enum class EMGModVerification : uint8
{
    Unverified      UMETA(DisplayName = "Unverified"),
    Pending         UMETA(DisplayName = "Pending Review"),
    Verified        UMETA(DisplayName = "Verified"),
    Featured        UMETA(DisplayName = "Featured"),
    Staff           UMETA(DisplayName = "Staff Pick"),
    Rejected        UMETA(DisplayName = "Rejected")
};

/**
 * EMGModRating - Age/content ratings
 */
UENUM(BlueprintType)
enum class EMGModRating : uint8
{
    Everyone        UMETA(DisplayName = "Everyone"),
    Teen            UMETA(DisplayName = "Teen"),
    Mature          UMETA(DisplayName = "Mature"),
    Unrated         UMETA(DisplayName = "Unrated")
};

/**
 * FMGModAuthor - Mod author information
 */
USTRUCT(BlueprintType)
struct FMGModAuthor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AuthorId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> AvatarTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalMods;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalDownloads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVerified;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProfileUrl;

    FMGModAuthor()
        : TotalMods(0)
        , TotalDownloads(0)
        , bIsVerified(false)
    {}
};

/**
 * FMGModDependency - Mod dependency information
 */
USTRUCT(BlueprintType)
struct FMGModDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MinVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSatisfied;

    FMGModDependency()
        : bIsOptional(false)
        , bIsSatisfied(false)
    {}
};

/**
 * FMGModVersion - Version information
 */
USTRUCT(BlueprintType)
struct FMGModVersion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VersionString;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Major;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Minor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Patch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChangeLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ReleaseDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MinGameVersion;

    FMGModVersion()
        : Major(1)
        , Minor(0)
        , Patch(0)
    {}

    FString GetVersionString() const
    {
        return FString::Printf(TEXT("%d.%d.%d"), Major, Minor, Patch);
    }
};

/**
 * FMGModStats - Download and rating statistics
 */
USTRUCT(BlueprintType)
struct FMGModStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalDownloads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UniqueSubscribers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFavorites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalRatings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveRatings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeRatings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalComments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeeklyDownloads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentRank;

    FMGModStats()
        : TotalDownloads(0)
        , UniqueSubscribers(0)
        , TotalFavorites(0)
        , AverageRating(0.0f)
        , TotalRatings(0)
        , PositiveRatings(0)
        , NegativeRatings(0)
        , TotalComments(0)
        , WeeklyDownloads(0)
        , CurrentRank(0)
    {}

    float GetPositivePercent() const
    {
        return TotalRatings > 0 ? (static_cast<float>(PositiveRatings) / TotalRatings) * 100.0f : 0.0f;
    }
};

/**
 * FMGModItem - Complete mod information
 */
USTRUCT(BlueprintType)
struct FMGModItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ShortDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModType ModType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModVerification Verification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModRating ContentRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGModAuthor Author;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGModVersion CurrentVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGModVersion InstalledVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGModStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGModDependency> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ThumbnailTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UTexture2D>> Screenshots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PreviewVideoUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 FileSizeBytes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DownloadUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LocalPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSubscribed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFavorited;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UserRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadOrder;

    FMGModItem()
        : ModType(EMGModType::Vehicle)
        , Status(EMGModStatus::NotInstalled)
        , Verification(EMGModVerification::Unverified)
        , ContentRating(EMGModRating::Everyone)
        , FileSizeBytes(0)
        , bIsSubscribed(false)
        , bIsFavorited(false)
        , UserRating(0)
        , LoadOrder(0)
    {}

    FString GetFileSizeFormatted() const
    {
        if (FileSizeBytes < 1024)
        {
            return FString::Printf(TEXT("%lld B"), FileSizeBytes);
        }
        else if (FileSizeBytes < 1024 * 1024)
        {
            return FString::Printf(TEXT("%.1f KB"), FileSizeBytes / 1024.0f);
        }
        else if (FileSizeBytes < 1024 * 1024 * 1024)
        {
            return FString::Printf(TEXT("%.1f MB"), FileSizeBytes / (1024.0f * 1024.0f));
        }
        else
        {
            return FString::Printf(TEXT("%.2f GB"), FileSizeBytes / (1024.0f * 1024.0f * 1024.0f));
        }
    }
};

/**
 * FMGModDownloadProgress - Download progress information
 */
USTRUCT(BlueprintType)
struct FMGModDownloadProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BytesDownloaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DownloadSpeedBps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedTimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsExtracting;

    FMGModDownloadProgress()
        : BytesDownloaded(0)
        , TotalBytes(0)
        , DownloadSpeedBps(0.0f)
        , EstimatedTimeRemaining(0.0f)
        , bIsExtracting(false)
    {}

    float GetProgress() const
    {
        return TotalBytes > 0 ? static_cast<float>(BytesDownloaded) / TotalBytes : 0.0f;
    }
};

/**
 * FMGModSearchFilter - Search filter options
 */
USTRUCT(BlueprintType)
struct FMGModSearchFilter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SearchQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGModType> ModTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModVerification MinVerification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGModRating MaxContentRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SortBy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSortDescending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PageSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PageNumber;

    FMGModSearchFilter()
        : MinVerification(EMGModVerification::Unverified)
        , MaxContentRating(EMGModRating::Mature)
        , MinRating(0.0f)
        , SortBy(FName("Popular"))
        , bSortDescending(true)
        , PageSize(20)
        , PageNumber(0)
    {}
};

/**
 * FMGModConflict - Mod conflict information
 */
USTRUCT(BlueprintType)
struct FMGModConflict
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModIdA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModIdB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ConflictDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ConflictType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanResolve;

    FMGModConflict()
        : ConflictType(FName("LoadOrder"))
        , bCanResolve(true)
    {}
};

/**
 * FMGWorkshopCollection - Collection of mods
 */
USTRUCT(BlueprintType)
struct FMGWorkshopCollection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CollectionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGModAuthor Author;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ModIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SubscriberCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> CoverImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdated;

    FMGWorkshopCollection()
        : SubscriberCount(0)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnModInstalled, const FMGModItem&, Mod);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnModUninstalled, const FString&, ModId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnModEnabled, const FString&, ModId, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnModDownloadProgress, const FMGModDownloadProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnModDownloadComplete, const FString&, ModId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnModUpdated, const FString&, ModId, const FMGModVersion&, NewVersion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnModSearchComplete, const TArray<FMGModItem>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnModConflictDetected, const FMGModConflict&, Conflict);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnModListChanged);

/**
 * UMGModdingSubsystem
 *
 * Manages modding and user-generated content for Midnight Grind.
 * Features include:
 * - Workshop/mod browser integration
 * - Mod download and installation
 * - Dependency management
 * - Load order configuration
 * - Conflict detection
 * - Custom content creation
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModdingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGModdingSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Workshop Browse =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Browse")
    void SearchMods(const FMGModSearchFilter& Filter);

    UFUNCTION(BlueprintCallable, Category = "Modding|Browse")
    void GetFeaturedMods();

    UFUNCTION(BlueprintCallable, Category = "Modding|Browse")
    void GetPopularMods(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Modding|Browse")
    void GetRecentMods(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Modding|Browse")
    void GetModDetails(const FString& ModId);

    UFUNCTION(BlueprintPure, Category = "Modding|Browse")
    FMGModItem GetCachedModDetails(const FString& ModId) const;

    // ===== Installation =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    bool SubscribeToMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    bool UnsubscribeFromMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    void DownloadMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    bool InstallMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    bool UninstallMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    void UpdateMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Install")
    void UpdateAllMods();

    UFUNCTION(BlueprintPure, Category = "Modding|Install")
    TArray<FString> GetModsWithUpdates() const;

    // ===== Mod Management =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Manage")
    bool EnableMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Manage")
    bool DisableMod(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Manage")
    void SetModLoadOrder(const FString& ModId, int32 LoadOrder);

    UFUNCTION(BlueprintCallable, Category = "Modding|Manage")
    void MoveModUp(const FString& ModId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Manage")
    void MoveModDown(const FString& ModId);

    UFUNCTION(BlueprintPure, Category = "Modding|Manage")
    TArray<FMGModItem> GetInstalledMods() const;

    UFUNCTION(BlueprintPure, Category = "Modding|Manage")
    TArray<FMGModItem> GetEnabledMods() const;

    UFUNCTION(BlueprintPure, Category = "Modding|Manage")
    TArray<FMGModItem> GetSubscribedMods() const;

    UFUNCTION(BlueprintPure, Category = "Modding|Manage")
    bool IsModInstalled(const FString& ModId) const;

    UFUNCTION(BlueprintPure, Category = "Modding|Manage")
    bool IsModEnabled(const FString& ModId) const;

    // ===== Dependencies =====

    UFUNCTION(BlueprintPure, Category = "Modding|Dependencies")
    TArray<FMGModDependency> GetModDependencies(const FString& ModId) const;

    UFUNCTION(BlueprintPure, Category = "Modding|Dependencies")
    bool AreDependenciesSatisfied(const FString& ModId) const;

    UFUNCTION(BlueprintCallable, Category = "Modding|Dependencies")
    void InstallMissingDependencies(const FString& ModId);

    // ===== Conflicts =====

    UFUNCTION(BlueprintPure, Category = "Modding|Conflicts")
    TArray<FMGModConflict> GetAllConflicts() const;

    UFUNCTION(BlueprintPure, Category = "Modding|Conflicts")
    TArray<FMGModConflict> GetConflictsForMod(const FString& ModId) const;

    UFUNCTION(BlueprintCallable, Category = "Modding|Conflicts")
    bool ResolveConflict(const FMGModConflict& Conflict);

    // ===== Collections =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Collections")
    void GetCollection(const FString& CollectionId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Collections")
    bool SubscribeToCollection(const FString& CollectionId);

    UFUNCTION(BlueprintCallable, Category = "Modding|Collections")
    FMGWorkshopCollection CreateCollection(const FText& Title, const FText& Description);

    UFUNCTION(BlueprintCallable, Category = "Modding|Collections")
    bool AddModToCollection(const FString& CollectionId, const FString& ModId);

    // ===== Ratings =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Ratings")
    bool RateMod(const FString& ModId, bool bPositive);

    UFUNCTION(BlueprintCallable, Category = "Modding|Ratings")
    bool FavoriteMod(const FString& ModId, bool bFavorite);

    UFUNCTION(BlueprintCallable, Category = "Modding|Ratings")
    void ReportMod(const FString& ModId, FName ReportReason, const FString& Details);

    // ===== User Creations =====

    UFUNCTION(BlueprintCallable, Category = "Modding|Create")
    bool ExportVinyl(const FString& VinylId, const FString& ExportPath);

    UFUNCTION(BlueprintCallable, Category = "Modding|Create")
    bool ExportTrack(const FString& TrackId, const FString& ExportPath);

    UFUNCTION(BlueprintCallable, Category = "Modding|Create")
    bool UploadMod(const FMGModItem& ModInfo, const FString& ContentPath);

    UFUNCTION(BlueprintCallable, Category = "Modding|Create")
    bool UpdateUploadedMod(const FString& ModId, const FMGModVersion& NewVersion, const FString& ContentPath);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModInstalled OnModInstalled;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModUninstalled OnModUninstalled;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModEnabled OnModEnabled;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModDownloadProgress OnModDownloadProgress;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModDownloadComplete OnModDownloadComplete;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModUpdated OnModUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModSearchComplete OnModSearchComplete;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModConflictDetected OnModConflictDetected;

    UPROPERTY(BlueprintAssignable, Category = "Modding|Events")
    FMGOnModListChanged OnModListChanged;

protected:
    void InitializeSampleMods();
    void LoadInstalledMods();
    void SaveModConfiguration();
    void CheckForConflicts();
    void SortModsByLoadOrder();
    FString GetModInstallPath(const FString& ModId) const;

private:
    UPROPERTY()
    TMap<FString, FMGModItem> AllMods;

    UPROPERTY()
    TArray<FString> InstalledModIds;

    UPROPERTY()
    TArray<FString> EnabledModIds;

    UPROPERTY()
    TArray<FString> SubscribedModIds;

    UPROPERTY()
    TMap<FString, FMGModDownloadProgress> ActiveDownloads;

    UPROPERTY()
    TArray<FMGModConflict> DetectedConflicts;

    UPROPERTY()
    TMap<FString, FMGWorkshopCollection> Collections;

    UPROPERTY()
    FString ModsDirectory;

    FTimerHandle UpdateCheckTimerHandle;
};
