// MidnightGrind - Arcade Street Racing Game
// Rental Subsystem - Implementation

#include "Rental/MGRentalSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGRentalSubsystem::UMGRentalSubsystem()
    : MaxFreeTrialsPerMonth(3)
    , FreeTrialsUsedThisMonth(0)
{
}

void UMGRentalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeSampleItems();
    InitializeBundles();
    InitializePasses();

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGRentalSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->TickRentals(1.0f);
                }
            },
            1.0f,
            true
        );
    }
}

void UMGRentalSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGRentalSubsystem::TickRentals(float DeltaTime)
{
    CheckExpirations();
}

// ===== Browse =====

TArray<FMGRentableItem> UMGRentalSubsystem::GetAllRentableItems() const
{
    TArray<FMGRentableItem> Items;
    for (const auto& Pair : RentableItems)
    {
        Items.Add(Pair.Value);
    }
    return Items;
}

TArray<FMGRentableItem> UMGRentalSubsystem::GetRentableItemsByCategory(EMGRentalCategory Category) const
{
    TArray<FMGRentableItem> Items;
    for (const auto& Pair : RentableItems)
    {
        if (Pair.Value.Category == Category)
        {
            Items.Add(Pair.Value);
        }
    }
    return Items;
}

TArray<FMGRentableItem> UMGRentalSubsystem::GetFeaturedRentals() const
{
    TArray<FMGRentableItem> Featured;
    for (const auto& Pair : RentableItems)
    {
        if (Pair.Value.bIsFeatured)
        {
            Featured.Add(Pair.Value);
        }
    }
    return Featured;
}

TArray<FMGRentableItem> UMGRentalSubsystem::GetNewRentals() const
{
    TArray<FMGRentableItem> NewItems;
    for (const auto& Pair : RentableItems)
    {
        if (Pair.Value.bIsNew)
        {
            NewItems.Add(Pair.Value);
        }
    }
    return NewItems;
}

FMGRentableItem UMGRentalSubsystem::GetRentableItem(FName ItemId) const
{
    if (const FMGRentableItem* Item = RentableItems.Find(ItemId))
    {
        return *Item;
    }
    return FMGRentableItem();
}

// ===== Rental Actions =====

bool UMGRentalSubsystem::RentItem(FName ItemId, EMGRentalDuration Duration)
{
    const FMGRentableItem* Item = RentableItems.Find(ItemId);
    if (!Item)
    {
        return false;
    }

    if (IsItemRented(ItemId))
    {
        return false;
    }

    int32 Price = GetRentalPrice(ItemId, Duration);

    FMGActiveRental NewRental;
    NewRental.RentalId = FGuid::NewGuid().ToString();
    NewRental.ItemId = ItemId;
    NewRental.ItemName = Item->DisplayName;
    NewRental.Category = Item->Category;
    NewRental.Status = EMGRentalStatus::Active;
    NewRental.Duration = Duration;
    NewRental.StartTime = FDateTime::Now();
    NewRental.EndTime = FDateTime::Now() + GetDurationTimespan(Duration);
    NewRental.PricePaid = Price;
    NewRental.PurchasePriceCredit = FMath::RoundToInt(Price * 0.5f);

    ActiveRentals.Add(NewRental.RentalId, NewRental);

    if (HasActivePass() && PlayerPass.RentalsRemaining > 0)
    {
        PlayerPass.RentalsRemaining--;
        PlayerPass.RentalsUsedThisMonth++;
    }

    OnRentalStarted.Broadcast(NewRental);
    return true;
}

bool UMGRentalSubsystem::StartFreeTrial(FName ItemId)
{
    if (!CanUseFreeTrial(ItemId))
    {
        return false;
    }

    const FMGRentableItem* Item = RentableItems.Find(ItemId);
    if (!Item || !Item->Pricing.bFreeTrial)
    {
        return false;
    }

    FMGActiveRental NewRental;
    NewRental.RentalId = FGuid::NewGuid().ToString();
    NewRental.ItemId = ItemId;
    NewRental.ItemName = Item->DisplayName;
    NewRental.Category = Item->Category;
    NewRental.Status = EMGRentalStatus::Active;
    NewRental.Duration = EMGRentalDuration::Unlimited;
    NewRental.StartTime = FDateTime::Now();
    NewRental.EndTime = FDateTime::Now() + FTimespan::FromMinutes(Item->Pricing.FreeTrialMinutes);
    NewRental.PricePaid = 0;
    NewRental.bIsFreeTrial = true;
    NewRental.bCanExtend = false;

    ActiveRentals.Add(NewRental.RentalId, NewRental);
    UsedFreeTrials.AddUnique(ItemId);
    FreeTrialsUsedThisMonth++;

    OnTrialStarted.Broadcast(ItemId);
    OnRentalStarted.Broadcast(NewRental);
    return true;
}

bool UMGRentalSubsystem::ExtendRental(const FString& RentalId, EMGRentalDuration AdditionalDuration)
{
    FMGActiveRental* Rental = ActiveRentals.Find(RentalId);
    if (!Rental || !Rental->bCanExtend)
    {
        return false;
    }

    if (Rental->Status != EMGRentalStatus::Active && Rental->Status != EMGRentalStatus::Expiring)
    {
        return false;
    }

    int32 ExtensionPrice = GetExtensionPrice(RentalId, AdditionalDuration);

    Rental->EndTime += GetDurationTimespan(AdditionalDuration);
    Rental->PricePaid += ExtensionPrice;
    Rental->PurchasePriceCredit += FMath::RoundToInt(ExtensionPrice * 0.5f);
    Rental->Status = EMGRentalStatus::Active;
    Rental->TimesExtended++;

    OnRentalExtended.Broadcast(*Rental);
    return true;
}

bool UMGRentalSubsystem::PurchaseRentedItem(const FString& RentalId)
{
    FMGActiveRental* Rental = ActiveRentals.Find(RentalId);
    if (!Rental)
    {
        return false;
    }

    int32 PurchasePrice = GetPurchasePriceWithCredit(RentalId);

    FMGRentalHistory History;
    History.RentalId = RentalId;
    History.ItemId = Rental->ItemId;
    History.ItemName = Rental->ItemName;
    History.Category = Rental->Category;
    History.Duration = Rental->Duration;
    History.RentalDate = Rental->StartTime;
    History.PricePaid = Rental->PricePaid + PurchasePrice;
    History.bWasPurchased = true;
    RentalHistory.Add(History);

    Rental->Status = EMGRentalStatus::Purchased;
    OnRentalPurchased.Broadcast(Rental->ItemId, Rental->PurchasePriceCredit);

    ActiveRentals.Remove(RentalId);
    return true;
}

bool UMGRentalSubsystem::EndRentalEarly(const FString& RentalId)
{
    FMGActiveRental* Rental = ActiveRentals.Find(RentalId);
    if (!Rental)
    {
        return false;
    }

    FMGRentalHistory History;
    History.RentalId = RentalId;
    History.ItemId = Rental->ItemId;
    History.ItemName = Rental->ItemName;
    History.Category = Rental->Category;
    History.Duration = Rental->Duration;
    History.RentalDate = Rental->StartTime;
    History.PricePaid = Rental->PricePaid;
    History.bWasPurchased = false;
    RentalHistory.Add(History);

    Rental->Status = EMGRentalStatus::Expired;
    OnRentalExpired.Broadcast(*Rental);

    ActiveRentals.Remove(RentalId);
    return true;
}

// ===== Active Rentals =====

TArray<FMGActiveRental> UMGRentalSubsystem::GetActiveRentals() const
{
    TArray<FMGActiveRental> Rentals;
    for (const auto& Pair : ActiveRentals)
    {
        if (Pair.Value.Status == EMGRentalStatus::Active || Pair.Value.Status == EMGRentalStatus::Expiring)
        {
            Rentals.Add(Pair.Value);
        }
    }
    return Rentals;
}

FMGActiveRental UMGRentalSubsystem::GetActiveRental(const FString& RentalId) const
{
    if (const FMGActiveRental* Rental = ActiveRentals.Find(RentalId))
    {
        return *Rental;
    }
    return FMGActiveRental();
}

bool UMGRentalSubsystem::IsItemRented(FName ItemId) const
{
    for (const auto& Pair : ActiveRentals)
    {
        if (Pair.Value.ItemId == ItemId &&
            (Pair.Value.Status == EMGRentalStatus::Active || Pair.Value.Status == EMGRentalStatus::Expiring))
        {
            return true;
        }
    }
    return false;
}

FMGActiveRental UMGRentalSubsystem::GetRentalForItem(FName ItemId) const
{
    for (const auto& Pair : ActiveRentals)
    {
        if (Pair.Value.ItemId == ItemId)
        {
            return Pair.Value;
        }
    }
    return FMGActiveRental();
}

TArray<FMGActiveRental> UMGRentalSubsystem::GetExpiringRentals() const
{
    TArray<FMGActiveRental> Expiring;
    for (const auto& Pair : ActiveRentals)
    {
        if (Pair.Value.IsExpiringSoon())
        {
            Expiring.Add(Pair.Value);
        }
    }
    return Expiring;
}

// ===== Trial =====

bool UMGRentalSubsystem::HasUsedFreeTrial(FName ItemId) const
{
    return UsedFreeTrials.Contains(ItemId);
}

bool UMGRentalSubsystem::CanUseFreeTrial(FName ItemId) const
{
    if (HasUsedFreeTrial(ItemId))
    {
        return false;
    }

    if (IsItemRented(ItemId))
    {
        return false;
    }

    if (FreeTrialsUsedThisMonth >= MaxFreeTrialsPerMonth && !HasActivePass())
    {
        return false;
    }

    const FMGRentableItem* Item = RentableItems.Find(ItemId);
    if (!Item || !Item->Pricing.bFreeTrial)
    {
        return false;
    }

    return true;
}

int32 UMGRentalSubsystem::GetFreeTrialsRemaining() const
{
    if (HasActivePass() && PlayerPass.RentalsRemaining > 0)
    {
        return MaxFreeTrialsPerMonth;
    }
    return FMath::Max(0, MaxFreeTrialsPerMonth - FreeTrialsUsedThisMonth);
}

// ===== Bundles =====

TArray<FMGRentalBundle> UMGRentalSubsystem::GetAvailableBundles() const
{
    TArray<FMGRentalBundle> BundleList;
    for (const auto& Pair : Bundles)
    {
        BundleList.Add(Pair.Value);
    }
    return BundleList;
}

bool UMGRentalSubsystem::RentBundle(FName BundleId)
{
    const FMGRentalBundle* Bundle = Bundles.Find(BundleId);
    if (!Bundle)
    {
        return false;
    }

    for (const FName& ItemId : Bundle->ItemIds)
    {
        if (!IsItemRented(ItemId))
        {
            RentItem(ItemId, Bundle->Duration);
        }
    }

    return true;
}

FMGRentalBundle UMGRentalSubsystem::GetBundle(FName BundleId) const
{
    if (const FMGRentalBundle* Bundle = Bundles.Find(BundleId))
    {
        return *Bundle;
    }
    return FMGRentalBundle();
}

// ===== Passes =====

TArray<FMGRentalPass> UMGRentalSubsystem::GetAvailablePasses() const
{
    TArray<FMGRentalPass> PassList;
    for (const auto& Pair : Passes)
    {
        PassList.Add(Pair.Value);
    }
    return PassList;
}

bool UMGRentalSubsystem::ActivateRentalPass(FName PassId)
{
    const FMGRentalPass* Pass = Passes.Find(PassId);
    if (!Pass)
    {
        return false;
    }

    PlayerPass.PassId = PassId;
    PlayerPass.StartDate = FDateTime::Now();
    PlayerPass.EndDate = FDateTime::Now() + FTimespan::FromDays(30);
    PlayerPass.RentalsRemaining = Pass->RentalsIncluded;
    PlayerPass.RentalsUsedThisMonth = 0;
    PlayerPass.bAutoRenew = false;

    OnRentalPassActivated.Broadcast(*Pass);
    return true;
}

FMGPlayerRentalPass UMGRentalSubsystem::GetActivePass() const
{
    return PlayerPass;
}

bool UMGRentalSubsystem::HasActivePass() const
{
    return PlayerPass.PassId != NAME_None && PlayerPass.IsActive();
}

// ===== Pricing =====

int32 UMGRentalSubsystem::GetRentalPrice(FName ItemId, EMGRentalDuration Duration) const
{
    const FMGRentableItem* Item = RentableItems.Find(ItemId);
    if (!Item)
    {
        return 0;
    }

    int32 BasePrice = Item->Pricing.GetPriceForDuration(Duration);

    if (HasActivePass())
    {
        const FMGRentalPass* Pass = Passes.Find(PlayerPass.PassId);
        if (Pass && Pass->IncludedCategories.Contains(Item->Category))
        {
            if (PlayerPass.RentalsRemaining > 0)
            {
                return 0;
            }
            BasePrice = Item->Pricing.GetDiscountedPrice(BasePrice);
        }
    }

    return BasePrice;
}

int32 UMGRentalSubsystem::GetExtensionPrice(const FString& RentalId, EMGRentalDuration Duration) const
{
    const FMGActiveRental* Rental = ActiveRentals.Find(RentalId);
    if (!Rental)
    {
        return 0;
    }

    int32 BasePrice = GetRentalPrice(Rental->ItemId, Duration);
    float Discount = FMath::Min(0.25f, Rental->TimesExtended * 0.05f);
    return FMath::RoundToInt(BasePrice * (1.0f - Discount));
}

int32 UMGRentalSubsystem::GetPurchasePriceWithCredit(const FString& RentalId) const
{
    const FMGActiveRental* Rental = ActiveRentals.Find(RentalId);
    if (!Rental)
    {
        return 0;
    }

    const FMGRentableItem* Item = RentableItems.Find(Rental->ItemId);
    if (!Item)
    {
        return 0;
    }

    int32 FullPrice = Item->Pricing.PurchasePrice;
    int32 Credit = Rental->PurchasePriceCredit;

    return FMath::Max(0, FullPrice - Credit);
}

// ===== History =====

TArray<FMGRentalHistory> UMGRentalSubsystem::GetRentalHistory() const
{
    return RentalHistory;
}

bool UMGRentalSubsystem::RateRental(const FString& RentalId, int32 Rating)
{
    for (FMGRentalHistory& History : RentalHistory)
    {
        if (History.RentalId == RentalId)
        {
            History.UserRating = FMath::Clamp(Rating, 1, 5);
            return true;
        }
    }
    return false;
}

// ===== Usage Tracking =====

void UMGRentalSubsystem::RecordRentalUsage(FName ItemId, float MinutesUsed)
{
    for (auto& Pair : ActiveRentals)
    {
        if (Pair.Value.ItemId == ItemId)
        {
            Pair.Value.UsageMinutes += FMath::RoundToInt(MinutesUsed);
            break;
        }
    }
}

void UMGRentalSubsystem::RecordRaceCompleted(FName ItemId)
{
    for (auto& Pair : ActiveRentals)
    {
        if (Pair.Value.ItemId == ItemId)
        {
            Pair.Value.RacesCompleted++;
            break;
        }
    }
}

// ===== Protected =====

void UMGRentalSubsystem::InitializeSampleItems()
{
    // Sample vehicle rental
    {
        FMGRentableItem Vehicle;
        Vehicle.ItemId = FName("rental_vehicle_rx7_fd");
        Vehicle.DisplayName = FText::FromString(TEXT("Mazda RX-7 FD3S"));
        Vehicle.Description = FText::FromString(TEXT("The legendary rotary-powered sports car. Experience the thrill of the 13B twin-turbo engine."));
        Vehicle.Category = EMGRentalCategory::Vehicle;
        Vehicle.Pricing.OneHourPrice = 500;
        Vehicle.Pricing.ThreeHourPrice = 1200;
        Vehicle.Pricing.OneDayPrice = 2500;
        Vehicle.Pricing.ThreeDayPrice = 6000;
        Vehicle.Pricing.OneWeekPrice = 12000;
        Vehicle.Pricing.PurchasePrice = 85000;
        Vehicle.Pricing.bFreeTrial = true;
        Vehicle.Pricing.FreeTrialMinutes = 15;
        Vehicle.RequiredLevel = 10;
        Vehicle.bIsFeatured = true;
        Vehicle.TimesRented = 1250;
        Vehicle.AverageRating = 4.8f;
        Vehicle.Attributes.Add(FName("Power"), TEXT("280hp"));
        Vehicle.Attributes.Add(FName("Engine"), TEXT("13B Twin-Turbo"));
        Vehicle.Tags = { FName("JDM"), FName("Rotary"), FName("Sports") };
        RentableItems.Add(Vehicle.ItemId, Vehicle);
    }

    // Sample performance part
    {
        FMGRentableItem Part;
        Part.ItemId = FName("rental_part_turbo_kit");
        Part.DisplayName = FText::FromString(TEXT("HKS GT3540 Turbo Kit"));
        Part.Description = FText::FromString(TEXT("High-performance turbo kit for maximum boost. Compatible with most JDM vehicles."));
        Part.Category = EMGRentalCategory::PerformancePart;
        Part.Pricing.OneHourPrice = 200;
        Part.Pricing.ThreeHourPrice = 500;
        Part.Pricing.OneDayPrice = 1000;
        Part.Pricing.ThreeDayPrice = 2500;
        Part.Pricing.OneWeekPrice = 5000;
        Part.Pricing.PurchasePrice = 35000;
        Part.Pricing.bFreeTrial = true;
        Part.Pricing.FreeTrialMinutes = 10;
        Part.RequiredLevel = 15;
        Part.bIsNew = true;
        Part.TimesRented = 890;
        Part.AverageRating = 4.6f;
        RentableItems.Add(Part.ItemId, Part);
    }

    // Sample body kit
    {
        FMGRentableItem BodyKit;
        BodyKit.ItemId = FName("rental_bodykit_rocket_bunny");
        BodyKit.DisplayName = FText::FromString(TEXT("Rocket Bunny V2 Wide Body"));
        BodyKit.Description = FText::FromString(TEXT("Aggressive wide body kit with flared fenders and ducktail spoiler."));
        BodyKit.Category = EMGRentalCategory::BodyKit;
        BodyKit.Pricing.OneHourPrice = 150;
        BodyKit.Pricing.ThreeHourPrice = 400;
        BodyKit.Pricing.OneDayPrice = 800;
        BodyKit.Pricing.ThreeDayPrice = 2000;
        BodyKit.Pricing.OneWeekPrice = 4000;
        BodyKit.Pricing.PurchasePrice = 25000;
        BodyKit.Pricing.bFreeTrial = false;
        BodyKit.RequiredLevel = 12;
        BodyKit.bIsFeatured = true;
        BodyKit.TimesRented = 2100;
        BodyKit.AverageRating = 4.9f;
        RentableItems.Add(BodyKit.ItemId, BodyKit);
    }

    // Sample track rental
    {
        FMGRentableItem Track;
        Track.ItemId = FName("rental_track_touge_mountain");
        Track.DisplayName = FText::FromString(TEXT("Midnight Touge - Mountain Pass"));
        Track.Description = FText::FromString(TEXT("Experience the legendary mountain pass under the moonlight. Technical corners and stunning views."));
        Track.Category = EMGRentalCategory::Track;
        Track.Pricing.OneHourPrice = 300;
        Track.Pricing.ThreeHourPrice = 750;
        Track.Pricing.OneDayPrice = 1500;
        Track.Pricing.ThreeDayPrice = 3500;
        Track.Pricing.OneWeekPrice = 7000;
        Track.Pricing.PurchasePrice = 50000;
        Track.Pricing.bFreeTrial = true;
        Track.Pricing.FreeTrialMinutes = 20;
        Track.RequiredLevel = 8;
        Track.bIsLimitedTime = true;
        Track.AvailableUntil = FDateTime::Now() + FTimespan::FromDays(30);
        Track.TimesRented = 3500;
        Track.AverageRating = 4.95f;
        RentableItems.Add(Track.ItemId, Track);
    }
}

void UMGRentalSubsystem::InitializeBundles()
{
    FMGRentalBundle JDMBundle;
    JDMBundle.BundleId = FName("bundle_jdm_starter");
    JDMBundle.DisplayName = FText::FromString(TEXT("JDM Starter Pack"));
    JDMBundle.Description = FText::FromString(TEXT("Everything you need to start your JDM journey. Includes RX-7, turbo kit, and body kit."));
    JDMBundle.ItemIds = { FName("rental_vehicle_rx7_fd"), FName("rental_part_turbo_kit"), FName("rental_bodykit_rocket_bunny") };
    JDMBundle.IndividualPriceTotal = 4300;
    JDMBundle.BundlePrice = 3000;
    JDMBundle.SavingsPercent = 30.0f;
    JDMBundle.Duration = EMGRentalDuration::OneDay;
    Bundles.Add(JDMBundle.BundleId, JDMBundle);

    FMGRentalBundle WeekendBundle;
    WeekendBundle.BundleId = FName("bundle_weekend_racer");
    WeekendBundle.DisplayName = FText::FromString(TEXT("Weekend Racer Bundle"));
    WeekendBundle.Description = FText::FromString(TEXT("Three days of unlimited racing with premium vehicles and tracks."));
    WeekendBundle.ItemIds = { FName("rental_vehicle_rx7_fd"), FName("rental_track_touge_mountain") };
    WeekendBundle.IndividualPriceTotal = 9500;
    WeekendBundle.BundlePrice = 7000;
    WeekendBundle.SavingsPercent = 26.0f;
    WeekendBundle.Duration = EMGRentalDuration::ThreeDays;
    Bundles.Add(WeekendBundle.BundleId, WeekendBundle);
}

void UMGRentalSubsystem::InitializePasses()
{
    FMGRentalPass BasicPass;
    BasicPass.PassId = FName("pass_basic");
    BasicPass.DisplayName = FText::FromString(TEXT("Rental Pass - Basic"));
    BasicPass.Description = FText::FromString(TEXT("5 rentals per month up to 24 hours each. 10% discount on purchases."));
    BasicPass.MonthlyPrice = 2000;
    BasicPass.RentalsIncluded = 5;
    BasicPass.MaxDuration = EMGRentalDuration::OneDay;
    BasicPass.IncludedCategories = { EMGRentalCategory::Vehicle, EMGRentalCategory::BodyKit, EMGRentalCategory::Cosmetic };
    BasicPass.PurchaseDiscount = 10.0f;
    BasicPass.bUnlimitedTrials = false;
    Passes.Add(BasicPass.PassId, BasicPass);

    FMGRentalPass PremiumPass;
    PremiumPass.PassId = FName("pass_premium");
    PremiumPass.DisplayName = FText::FromString(TEXT("Rental Pass - Premium"));
    PremiumPass.Description = FText::FromString(TEXT("Unlimited rentals up to 7 days each. 25% discount on purchases. Unlimited free trials."));
    PremiumPass.MonthlyPrice = 5000;
    PremiumPass.RentalsIncluded = 999;
    PremiumPass.MaxDuration = EMGRentalDuration::OneWeek;
    PremiumPass.IncludedCategories = { EMGRentalCategory::Vehicle, EMGRentalCategory::Track, EMGRentalCategory::BodyKit, EMGRentalCategory::PerformancePart, EMGRentalCategory::Cosmetic };
    PremiumPass.PurchaseDiscount = 25.0f;
    PremiumPass.bUnlimitedTrials = true;
    Passes.Add(PremiumPass.PassId, PremiumPass);
}

void UMGRentalSubsystem::CheckExpirations()
{
    FDateTime Now = FDateTime::Now();

    for (auto& Pair : ActiveRentals)
    {
        FMGActiveRental& Rental = Pair.Value;

        if (Rental.Status == EMGRentalStatus::Active)
        {
            if (Rental.IsExpiringSoon())
            {
                Rental.Status = EMGRentalStatus::Expiring;
                OnRentalExpiring.Broadcast(Rental);
            }
        }

        if (Rental.Status == EMGRentalStatus::Expiring || Rental.Status == EMGRentalStatus::Active)
        {
            if (Now >= Rental.EndTime)
            {
                Rental.Status = EMGRentalStatus::Expired;

                FMGRentalHistory History;
                History.RentalId = Rental.RentalId;
                History.ItemId = Rental.ItemId;
                History.ItemName = Rental.ItemName;
                History.Category = Rental.Category;
                History.Duration = Rental.Duration;
                History.RentalDate = Rental.StartTime;
                History.PricePaid = Rental.PricePaid;
                History.bWasPurchased = false;
                RentalHistory.Add(History);

                OnRentalExpired.Broadcast(Rental);
            }
        }
    }

    // Clean up expired rentals
    TArray<FString> ToRemove;
    for (const auto& Pair : ActiveRentals)
    {
        if (Pair.Value.Status == EMGRentalStatus::Expired)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (const FString& Key : ToRemove)
    {
        ActiveRentals.Remove(Key);
    }
}

FTimespan UMGRentalSubsystem::GetDurationTimespan(EMGRentalDuration Duration) const
{
    switch (Duration)
    {
    case EMGRentalDuration::OneHour:
        return FTimespan::FromHours(1);
    case EMGRentalDuration::ThreeHours:
        return FTimespan::FromHours(3);
    case EMGRentalDuration::OneDay:
        return FTimespan::FromDays(1);
    case EMGRentalDuration::ThreeDays:
        return FTimespan::FromDays(3);
    case EMGRentalDuration::OneWeek:
        return FTimespan::FromDays(7);
    case EMGRentalDuration::Unlimited:
        return FTimespan::FromDays(365);
    default:
        return FTimespan::FromHours(1);
    }
}
