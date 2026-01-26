// MidnightGrind - Arcade Street Racing Game
// Rental Subsystem - Vehicle rentals, trial periods, and temporary unlocks

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRentalSubsystem.generated.h"

// Forward declarations
class UMGRentalSubsystem;

/**
 * EMGRentalDuration - Rental period options
 */
UENUM(BlueprintType)
enum class EMGRentalDuration : uint8
{
    OneHour         UMETA(DisplayName = "1 Hour"),
    ThreeHours      UMETA(DisplayName = "3 Hours"),
    OneDay          UMETA(DisplayName = "24 Hours"),
    ThreeDays       UMETA(DisplayName = "3 Days"),
    OneWeek         UMETA(DisplayName = "7 Days"),
    Unlimited       UMETA(DisplayName = "Unlimited Trial")
};

/**
 * EMGRentalStatus - Status of a rental
 */
UENUM(BlueprintType)
enum class EMGRentalStatus : uint8
{
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Expiring        UMETA(DisplayName = "Expiring Soon"),
    Expired         UMETA(DisplayName = "Expired"),
    Purchased       UMETA(DisplayName = "Purchased")
};

/**
 * EMGRentalCategory - Categories of rentable items
 */
UENUM(BlueprintType)
enum class EMGRentalCategory : uint8
{
    Vehicle         UMETA(DisplayName = "Vehicle"),
    Track           UMETA(DisplayName = "Track"),
    BodyKit         UMETA(DisplayName = "Body Kit"),
    PerformancePart UMETA(DisplayName = "Performance Part"),
    Cosmetic        UMETA(DisplayName = "Cosmetic Item"),
    Bundle          UMETA(DisplayName = "Item Bundle"),
    PremiumFeature  UMETA(DisplayName = "Premium Feature")
};

/**
 * FMGRentalPricing - Pricing for different rental durations
 */
USTRUCT(BlueprintType)
struct FMGRentalPricing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OneHourPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ThreeHourPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OneDayPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ThreeDayPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OneWeekPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PurchasePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DiscountPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFreeTrial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FreeTrialMinutes;

    FMGRentalPricing()
        : OneHourPrice(100)
        , ThreeHourPrice(250)
        , OneDayPrice(500)
        , ThreeDayPrice(1200)
        , OneWeekPrice(2500)
        , PurchasePrice(10000)
        , DiscountPercent(0.0f)
        , bFreeTrial(false)
        , FreeTrialMinutes(15)
    {}

    int32 GetPriceForDuration(EMGRentalDuration Duration) const
    {
        switch (Duration)
        {
        case EMGRentalDuration::OneHour: return OneHourPrice;
        case EMGRentalDuration::ThreeHours: return ThreeHourPrice;
        case EMGRentalDuration::OneDay: return OneDayPrice;
        case EMGRentalDuration::ThreeDays: return ThreeDayPrice;
        case EMGRentalDuration::OneWeek: return OneWeekPrice;
        case EMGRentalDuration::Unlimited: return 0;
        default: return 0;
        }
    }

    int32 GetDiscountedPrice(int32 BasePrice) const
    {
        return FMath::RoundToInt(BasePrice * (1.0f - DiscountPercent / 100.0f));
    }
};

/**
 * FMGRentableItem - An item available for rental
 */
USTRUCT(BlueprintType)
struct FMGRentableItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGRentalPricing Pricing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ThumbnailTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UTexture2D>> PreviewImages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> Attributes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNew;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFeatured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLimitedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime AvailableUntil;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesRented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageRating;

    FMGRentableItem()
        : ItemId(NAME_None)
        , Category(EMGRentalCategory::Vehicle)
        , RequiredLevel(1)
        , bIsNew(false)
        , bIsFeatured(false)
        , bIsLimitedTime(false)
        , TimesRented(0)
        , AverageRating(0.0f)
    {}
};

/**
 * FMGActiveRental - An active rental record
 */
USTRUCT(BlueprintType)
struct FMGActiveRental
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RentalId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalDuration Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PricePaid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PurchasePriceCredit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFreeTrial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UsageMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RacesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanExtend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesExtended;

    FMGActiveRental()
        : ItemId(NAME_None)
        , Category(EMGRentalCategory::Vehicle)
        , Status(EMGRentalStatus::Active)
        , Duration(EMGRentalDuration::OneHour)
        , PricePaid(0)
        , PurchasePriceCredit(0)
        , bIsFreeTrial(false)
        , UsageMinutes(0)
        , RacesCompleted(0)
        , bCanExtend(true)
        , TimesExtended(0)
    {}

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndTime > Now ? EndTime - Now : FTimespan::Zero();
    }

    bool IsExpiringSoon() const
    {
        return GetTimeRemaining().GetTotalMinutes() < 30.0;
    }

    float GetUsagePercent() const
    {
        FTimespan TotalDuration = EndTime - StartTime;
        FTimespan Used = FDateTime::Now() - StartTime;
        return TotalDuration.GetTotalSeconds() > 0
            ? FMath::Clamp(Used.GetTotalSeconds() / TotalDuration.GetTotalSeconds(), 0.0, 1.0) * 100.0f
            : 0.0f;
    }
};

/**
 * FMGRentalHistory - Historical rental record
 */
USTRUCT(BlueprintType)
struct FMGRentalHistory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RentalId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalDuration Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RentalDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PricePaid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWasPurchased;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UserRating;

    FMGRentalHistory()
        : ItemId(NAME_None)
        , Category(EMGRentalCategory::Vehicle)
        , Duration(EMGRentalDuration::OneHour)
        , PricePaid(0)
        , bWasPurchased(false)
        , UserRating(0)
    {}
};

/**
 * FMGRentalBundle - A bundle of rentable items
 */
USTRUCT(BlueprintType)
struct FMGRentalBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BundleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ItemIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BundlePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 IndividualPriceTotal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SavingsPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalDuration Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> BundleImage;

    FMGRentalBundle()
        : BundleId(NAME_None)
        , BundlePrice(0)
        , IndividualPriceTotal(0)
        , SavingsPercent(0.0f)
        , Duration(EMGRentalDuration::OneDay)
    {}
};

/**
 * FMGRentalPass - Subscription-style rental pass
 */
USTRUCT(BlueprintType)
struct FMGRentalPass
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PassId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MonthlyPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RentalsIncluded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRentalDuration MaxDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGRentalCategory> IncludedCategories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PurchaseDiscount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUnlimitedTrials;

    FMGRentalPass()
        : PassId(NAME_None)
        , MonthlyPrice(1000)
        , RentalsIncluded(5)
        , MaxDuration(EMGRentalDuration::ThreeDays)
        , PurchaseDiscount(10.0f)
        , bUnlimitedTrials(false)
    {}
};

/**
 * FMGPlayerRentalPass - Player's active rental pass
 */
USTRUCT(BlueprintType)
struct FMGPlayerRentalPass
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PassId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RentalsRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RentalsUsedThisMonth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRenew;

    FMGPlayerRentalPass()
        : PassId(NAME_None)
        , RentalsRemaining(0)
        , RentalsUsedThisMonth(0)
        , bAutoRenew(false)
    {}

    bool IsActive() const
    {
        FDateTime Now = FDateTime::Now();
        return Now >= StartDate && Now <= EndDate;
    }
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRentalStarted, const FMGActiveRental&, Rental);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRentalExpiring, const FMGActiveRental&, Rental);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRentalExpired, const FMGActiveRental&, Rental);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRentalExtended, const FMGActiveRental&, Rental);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRentalPurchased, FName, ItemId, int32, PriceCredit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTrialStarted, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRentalPassActivated, const FMGRentalPass&, Pass);

/**
 * UMGRentalSubsystem
 *
 * Manages vehicle and item rentals for Midnight Grind.
 * Features include:
 * - Timed rentals with various durations
 * - Free trial periods
 * - Rental to purchase conversion
 * - Rental bundles and passes
 * - Usage tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRentalSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGRentalSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void TickRentals(float DeltaTime);

    // ===== Browse =====

    UFUNCTION(BlueprintPure, Category = "Rental|Browse")
    TArray<FMGRentableItem> GetAllRentableItems() const;

    UFUNCTION(BlueprintPure, Category = "Rental|Browse")
    TArray<FMGRentableItem> GetRentableItemsByCategory(EMGRentalCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Browse")
    TArray<FMGRentableItem> GetFeaturedRentals() const;

    UFUNCTION(BlueprintPure, Category = "Rental|Browse")
    TArray<FMGRentableItem> GetNewRentals() const;

    UFUNCTION(BlueprintPure, Category = "Rental|Browse")
    FMGRentableItem GetRentableItem(FName ItemId) const;

    // ===== Rental Actions =====

    UFUNCTION(BlueprintCallable, Category = "Rental|Actions")
    bool RentItem(FName ItemId, EMGRentalDuration Duration);

    UFUNCTION(BlueprintCallable, Category = "Rental|Actions")
    bool StartFreeTrial(FName ItemId);

    UFUNCTION(BlueprintCallable, Category = "Rental|Actions")
    bool ExtendRental(const FString& RentalId, EMGRentalDuration AdditionalDuration);

    UFUNCTION(BlueprintCallable, Category = "Rental|Actions")
    bool PurchaseRentedItem(const FString& RentalId);

    UFUNCTION(BlueprintCallable, Category = "Rental|Actions")
    bool EndRentalEarly(const FString& RentalId);

    // ===== Active Rentals =====

    UFUNCTION(BlueprintPure, Category = "Rental|Active")
    TArray<FMGActiveRental> GetActiveRentals() const;

    UFUNCTION(BlueprintPure, Category = "Rental|Active")
    FMGActiveRental GetActiveRental(const FString& RentalId) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Active")
    bool IsItemRented(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Active")
    FMGActiveRental GetRentalForItem(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Active")
    TArray<FMGActiveRental> GetExpiringRentals() const;

    // ===== Trial =====

    UFUNCTION(BlueprintPure, Category = "Rental|Trial")
    bool HasUsedFreeTrial(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Trial")
    bool CanUseFreeTrial(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Trial")
    int32 GetFreeTrialsRemaining() const;

    // ===== Bundles =====

    UFUNCTION(BlueprintPure, Category = "Rental|Bundles")
    TArray<FMGRentalBundle> GetAvailableBundles() const;

    UFUNCTION(BlueprintCallable, Category = "Rental|Bundles")
    bool RentBundle(FName BundleId);

    UFUNCTION(BlueprintPure, Category = "Rental|Bundles")
    FMGRentalBundle GetBundle(FName BundleId) const;

    // ===== Passes =====

    UFUNCTION(BlueprintPure, Category = "Rental|Passes")
    TArray<FMGRentalPass> GetAvailablePasses() const;

    UFUNCTION(BlueprintCallable, Category = "Rental|Passes")
    bool ActivateRentalPass(FName PassId);

    UFUNCTION(BlueprintPure, Category = "Rental|Passes")
    FMGPlayerRentalPass GetActivePass() const;

    UFUNCTION(BlueprintPure, Category = "Rental|Passes")
    bool HasActivePass() const;

    // ===== Pricing =====

    UFUNCTION(BlueprintPure, Category = "Rental|Pricing")
    int32 GetRentalPrice(FName ItemId, EMGRentalDuration Duration) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Pricing")
    int32 GetExtensionPrice(const FString& RentalId, EMGRentalDuration Duration) const;

    UFUNCTION(BlueprintPure, Category = "Rental|Pricing")
    int32 GetPurchasePriceWithCredit(const FString& RentalId) const;

    // ===== History =====

    UFUNCTION(BlueprintPure, Category = "Rental|History")
    TArray<FMGRentalHistory> GetRentalHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Rental|History")
    bool RateRental(const FString& RentalId, int32 Rating);

    // ===== Usage Tracking =====

    UFUNCTION(BlueprintCallable, Category = "Rental|Usage")
    void RecordRentalUsage(FName ItemId, float MinutesUsed);

    UFUNCTION(BlueprintCallable, Category = "Rental|Usage")
    void RecordRaceCompleted(FName ItemId);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalStarted OnRentalStarted;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalExpiring OnRentalExpiring;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalExpired OnRentalExpired;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalExtended OnRentalExtended;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalPurchased OnRentalPurchased;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnTrialStarted OnTrialStarted;

    UPROPERTY(BlueprintAssignable, Category = "Rental|Events")
    FMGOnRentalPassActivated OnRentalPassActivated;

protected:
    void InitializeSampleItems();
    void InitializeBundles();
    void InitializePasses();
    void CheckExpirations();
    FTimespan GetDurationTimespan(EMGRentalDuration Duration) const;

private:
    UPROPERTY()
    TMap<FName, FMGRentableItem> RentableItems;

    UPROPERTY()
    TMap<FString, FMGActiveRental> ActiveRentals;

    UPROPERTY()
    TArray<FMGRentalHistory> RentalHistory;

    UPROPERTY()
    TMap<FName, FMGRentalBundle> Bundles;

    UPROPERTY()
    TMap<FName, FMGRentalPass> Passes;

    UPROPERTY()
    FMGPlayerRentalPass PlayerPass;

    UPROPERTY()
    TArray<FName> UsedFreeTrials;

    UPROPERTY()
    int32 MaxFreeTrialsPerMonth;

    UPROPERTY()
    int32 FreeTrialsUsedThisMonth;

    FTimerHandle TickTimerHandle;
};
