// MidnightGrind - Arcade Street Racing Game
// Marketplace Subsystem - Implementation

#include "Marketplace/MGMarketplaceSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGMarketplaceSubsystem::UMGMarketplaceSubsystem()
    : CurrentPlayerId(TEXT("LocalPlayer"))
    , MarketplaceFeePercent(5.0f)
{
}

void UMGMarketplaceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize sample listings
    InitializeSampleListings();

    // Start tick timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGMarketplaceSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->TickMarketplace(1.0f);
                }
            },
            1.0f,
            true
        );
    }
}

void UMGMarketplaceSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGMarketplaceSubsystem::TickMarketplace(float DeltaTime)
{
    ProcessAuctions();
    CheckExpiringListings();
}

// ===== Listings =====

bool UMGMarketplaceSubsystem::CreateListing(const FMGMarketItem& Item, EMGListingType Type, int64 Price, FTimespan Duration)
{
    FMGMarketplaceListing NewListing;
    NewListing.ListingId = FGuid::NewGuid().ToString();
    NewListing.Item = Item;
    NewListing.ListingType = Type;
    NewListing.Price = Price;
    NewListing.Status = EMGListingStatus::Active;
    NewListing.ListedTime = FDateTime::Now();
    NewListing.EndTime = FDateTime::Now() + Duration;
    NewListing.CurrencyType = FName("Credits");

    // Set seller info
    NewListing.Seller.SellerId = CurrentPlayerId;
    NewListing.Seller.SellerName = TEXT("LocalPlayer");
    NewListing.Seller.bIsVerified = true;

    AllListings.Add(NewListing.ListingId, NewListing);
    MyListingIds.Add(NewListing.ListingId);

    OnListingCreated.Broadcast(NewListing);
    return true;
}

bool UMGMarketplaceSubsystem::CreateAuction(const FMGMarketItem& Item, int64 StartingBid, int64 ReservePrice, int64 BuyNowPrice, FTimespan Duration)
{
    FMGMarketplaceListing NewListing;
    NewListing.ListingId = FGuid::NewGuid().ToString();
    NewListing.Item = Item;
    NewListing.ListingType = BuyNowPrice > 0 ? EMGListingType::BuyNow : EMGListingType::Auction;
    NewListing.Status = EMGListingStatus::Active;
    NewListing.StartingBid = StartingBid;
    NewListing.CurrentBid = 0;
    NewListing.ReservePrice = ReservePrice;
    NewListing.BuyNowPrice = BuyNowPrice;
    NewListing.bHasReserve = ReservePrice > 0;
    NewListing.MinBidIncrement = FMath::Max(100LL, StartingBid / 20);
    NewListing.ListedTime = FDateTime::Now();
    NewListing.EndTime = FDateTime::Now() + Duration;
    NewListing.CurrencyType = FName("Credits");

    // Set seller info
    NewListing.Seller.SellerId = CurrentPlayerId;
    NewListing.Seller.SellerName = TEXT("LocalPlayer");
    NewListing.Seller.bIsVerified = true;

    AllListings.Add(NewListing.ListingId, NewListing);
    MyListingIds.Add(NewListing.ListingId);

    OnListingCreated.Broadcast(NewListing);
    return true;
}

bool UMGMarketplaceSubsystem::CancelListing(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing)
    {
        return false;
    }

    // Can only cancel own listings
    if (Listing->Seller.SellerId != CurrentPlayerId)
    {
        return false;
    }

    // Can't cancel if there are bids
    if (Listing->BidHistory.Num() > 0)
    {
        return false;
    }

    Listing->Status = EMGListingStatus::Cancelled;
    MyListingIds.Remove(ListingId);

    return true;
}

bool UMGMarketplaceSubsystem::BuyListing(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing || Listing->Status != EMGListingStatus::Active)
    {
        return false;
    }

    // Can't buy own listing
    if (Listing->Seller.SellerId == CurrentPlayerId)
    {
        return false;
    }

    // Handle different listing types
    int64 PurchasePrice = 0;
    if (Listing->ListingType == EMGListingType::FixedPrice)
    {
        PurchasePrice = Listing->Price;
    }
    else if (Listing->ListingType == EMGListingType::BuyNow || Listing->ListingType == EMGListingType::Auction)
    {
        if (Listing->BuyNowPrice <= 0)
        {
            return false; // No buy now option
        }
        PurchasePrice = Listing->BuyNowPrice;
    }

    // Complete the sale
    Listing->Status = EMGListingStatus::Sold;
    MyListingIds.Remove(ListingId);

    OnListingSold.Broadcast(*Listing);
    return true;
}

FMGMarketplaceListing UMGMarketplaceSubsystem::GetListing(const FString& ListingId) const
{
    if (const FMGMarketplaceListing* Listing = AllListings.Find(ListingId))
    {
        return *Listing;
    }
    return FMGMarketplaceListing();
}

TArray<FMGMarketplaceListing> UMGMarketplaceSubsystem::GetMyListings() const
{
    TArray<FMGMarketplaceListing> Results;

    for (const FString& ListingId : MyListingIds)
    {
        if (const FMGMarketplaceListing* Listing = AllListings.Find(ListingId))
        {
            Results.Add(*Listing);
        }
    }

    return Results;
}

// ===== Bidding =====

bool UMGMarketplaceSubsystem::PlaceBid(const FString& ListingId, int64 BidAmount)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing || Listing->Status != EMGListingStatus::Active)
    {
        return false;
    }

    // Can't bid on own listing
    if (Listing->Seller.SellerId == CurrentPlayerId)
    {
        return false;
    }

    // Verify it's an auction
    if (Listing->ListingType != EMGListingType::Auction && Listing->ListingType != EMGListingType::BuyNow)
    {
        return false;
    }

    // Check minimum bid
    int64 MinBid = Listing->CurrentBid > 0
        ? Listing->CurrentBid + Listing->MinBidIncrement
        : Listing->StartingBid;

    if (BidAmount < MinBid)
    {
        return false;
    }

    // Record old highest bidder for outbid notification
    FString OldHighestBidder;
    if (Listing->BidHistory.Num() > 0)
    {
        OldHighestBidder = Listing->BidHistory.Last().BidderId;
    }

    // Create bid
    FMGBidInfo NewBid;
    NewBid.BidId = FGuid::NewGuid().ToString();
    NewBid.BidderId = CurrentPlayerId;
    NewBid.BidderName = TEXT("LocalPlayer");
    NewBid.BidAmount = BidAmount;
    NewBid.BidTime = FDateTime::Now();

    Listing->CurrentBid = BidAmount;
    Listing->BidHistory.Add(NewBid);

    // Check if reserve is met
    if (Listing->bHasReserve && BidAmount >= Listing->ReservePrice)
    {
        Listing->bReserveMet = true;
    }

    // Add to my bids
    MyBidIds.AddUnique(ListingId);

    OnBidPlaced.Broadcast(ListingId, NewBid);

    // Notify outbid player
    if (!OldHighestBidder.IsEmpty() && OldHighestBidder != CurrentPlayerId)
    {
        OnBidOutbid.Broadcast(ListingId);
    }

    // Process any auto bids
    ProcessAutoBids(ListingId);

    return true;
}

bool UMGMarketplaceSubsystem::SetAutoBid(const FString& ListingId, int64 MaxBid)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing || Listing->Status != EMGListingStatus::Active)
    {
        return false;
    }

    // Find existing bid from this player and update max
    for (FMGBidInfo& Bid : Listing->BidHistory)
    {
        if (Bid.BidderId == CurrentPlayerId)
        {
            Bid.bIsAutoBid = true;
            Bid.MaxAutoBid = MaxBid;
            return true;
        }
    }

    // No existing bid, need to place initial bid first
    return false;
}

bool UMGMarketplaceSubsystem::CancelAutoBid(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing)
    {
        return false;
    }

    for (FMGBidInfo& Bid : Listing->BidHistory)
    {
        if (Bid.BidderId == CurrentPlayerId)
        {
            Bid.bIsAutoBid = false;
            Bid.MaxAutoBid = 0;
            return true;
        }
    }

    return false;
}

TArray<FMGMarketplaceListing> UMGMarketplaceSubsystem::GetMyBids() const
{
    TArray<FMGMarketplaceListing> Results;

    for (const FString& ListingId : MyBidIds)
    {
        if (const FMGMarketplaceListing* Listing = AllListings.Find(ListingId))
        {
            if (Listing->Status == EMGListingStatus::Active)
            {
                Results.Add(*Listing);
            }
        }
    }

    return Results;
}

bool UMGMarketplaceSubsystem::AmIHighestBidder(const FString& ListingId) const
{
    const FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing || Listing->BidHistory.Num() == 0)
    {
        return false;
    }

    return Listing->BidHistory.Last().BidderId == CurrentPlayerId;
}

// ===== Trading =====

bool UMGMarketplaceSubsystem::CreateTradeOffer(const FString& RecipientId, const TArray<FMGMarketItem>& MyItems, const TArray<FMGMarketItem>& WantedItems, int64 MyCurrency, int64 WantedCurrency)
{
    FMGTradeOffer NewTrade;
    NewTrade.TradeId = FGuid::NewGuid().ToString();
    NewTrade.InitiatorId = CurrentPlayerId;
    NewTrade.RecipientId = RecipientId;
    NewTrade.InitiatorItems = MyItems;
    NewTrade.RecipientItems = WantedItems;
    NewTrade.InitiatorCurrency = MyCurrency;
    NewTrade.RecipientCurrency = WantedCurrency;
    NewTrade.CreatedTime = FDateTime::Now();
    NewTrade.ExpiresTime = FDateTime::Now() + FTimespan::FromDays(3);

    PendingTrades.Add(NewTrade.TradeId, NewTrade);

    // Notify recipient
    OnTradeOfferReceived.Broadcast(NewTrade);
    return true;
}

bool UMGMarketplaceSubsystem::AcceptTrade(const FString& TradeId)
{
    FMGTradeOffer* Trade = PendingTrades.Find(TradeId);
    if (!Trade)
    {
        return false;
    }

    // Verify we're the recipient
    if (Trade->RecipientId != CurrentPlayerId && Trade->InitiatorId != CurrentPlayerId)
    {
        return false;
    }

    // Mark confirmation
    if (Trade->RecipientId == CurrentPlayerId)
    {
        Trade->bRecipientConfirmed = true;
    }
    else
    {
        Trade->bInitiatorConfirmed = true;
    }

    // If both confirmed, complete the trade
    if (Trade->bInitiatorConfirmed && Trade->bRecipientConfirmed)
    {
        OnTradeCompleted.Broadcast(*Trade);
        PendingTrades.Remove(TradeId);
    }

    return true;
}

bool UMGMarketplaceSubsystem::DeclineTrade(const FString& TradeId)
{
    if (!PendingTrades.Contains(TradeId))
    {
        return false;
    }

    PendingTrades.Remove(TradeId);
    return true;
}

bool UMGMarketplaceSubsystem::CounterTrade(const FString& TradeId, const TArray<FMGMarketItem>& MyItems, const TArray<FMGMarketItem>& WantedItems, int64 MyCurrency, int64 WantedCurrency)
{
    FMGTradeOffer* OriginalTrade = PendingTrades.Find(TradeId);
    if (!OriginalTrade)
    {
        return false;
    }

    // Create counter offer
    FMGTradeOffer CounterOffer;
    CounterOffer.TradeId = FGuid::NewGuid().ToString();
    CounterOffer.InitiatorId = CurrentPlayerId;
    CounterOffer.RecipientId = OriginalTrade->InitiatorId;
    CounterOffer.InitiatorItems = MyItems;
    CounterOffer.RecipientItems = WantedItems;
    CounterOffer.InitiatorCurrency = MyCurrency;
    CounterOffer.RecipientCurrency = WantedCurrency;
    CounterOffer.CreatedTime = FDateTime::Now();
    CounterOffer.ExpiresTime = FDateTime::Now() + FTimespan::FromDays(3);
    CounterOffer.bIsCounterOffer = true;
    CounterOffer.OriginalTradeId = TradeId;

    // Remove original trade
    PendingTrades.Remove(TradeId);

    // Add counter offer
    PendingTrades.Add(CounterOffer.TradeId, CounterOffer);
    OnTradeOfferReceived.Broadcast(CounterOffer);

    return true;
}

TArray<FMGTradeOffer> UMGMarketplaceSubsystem::GetPendingTrades() const
{
    TArray<FMGTradeOffer> Results;

    for (const auto& Pair : PendingTrades)
    {
        if (Pair.Value.RecipientId == CurrentPlayerId)
        {
            Results.Add(Pair.Value);
        }
    }

    return Results;
}

TArray<FMGTradeOffer> UMGMarketplaceSubsystem::GetSentTrades() const
{
    TArray<FMGTradeOffer> Results;

    for (const auto& Pair : PendingTrades)
    {
        if (Pair.Value.InitiatorId == CurrentPlayerId)
        {
            Results.Add(Pair.Value);
        }
    }

    return Results;
}

// ===== Search =====

void UMGMarketplaceSubsystem::SearchListings(const FMGMarketSearchFilter& Filter)
{
    TArray<FMGMarketplaceListing> Results;

    for (const auto& Pair : AllListings)
    {
        const FMGMarketplaceListing& Listing = Pair.Value;

        if (Listing.Status != EMGListingStatus::Active)
        {
            continue;
        }

        // Apply filters
        if (!Filter.SearchQuery.IsEmpty())
        {
            FString SearchLower = Filter.SearchQuery.ToLower();
            if (!Listing.Item.ItemName.ToString().ToLower().Contains(SearchLower))
            {
                continue;
            }
        }

        if (Filter.Categories.Num() > 0 && !Filter.Categories.Contains(Listing.Item.Category))
        {
            continue;
        }

        if (Filter.Rarities.Num() > 0 && !Filter.Rarities.Contains(Listing.Item.Rarity))
        {
            continue;
        }

        int64 ListingPrice = Listing.ListingType == EMGListingType::FixedPrice ? Listing.Price : Listing.CurrentBid;
        if (Filter.MinPrice > 0 && ListingPrice < Filter.MinPrice)
        {
            continue;
        }
        if (Filter.MaxPrice > 0 && ListingPrice > Filter.MaxPrice)
        {
            continue;
        }

        if (Filter.bEndingSoonOnly && !Listing.IsEndingSoon())
        {
            continue;
        }

        if (Filter.bBuyNowOnly && Listing.BuyNowPrice <= 0)
        {
            continue;
        }

        if (Filter.bVerifiedSellersOnly && !Listing.Seller.bIsVerified)
        {
            continue;
        }

        Results.Add(Listing);
    }

    // Sort
    if (Filter.SortBy == FName("EndTime"))
    {
        Results.Sort([Filter](const FMGMarketplaceListing& A, const FMGMarketplaceListing& B)
        {
            return Filter.bSortDescending ? A.EndTime > B.EndTime : A.EndTime < B.EndTime;
        });
    }
    else if (Filter.SortBy == FName("Price"))
    {
        Results.Sort([Filter](const FMGMarketplaceListing& A, const FMGMarketplaceListing& B)
        {
            int64 PriceA = A.ListingType == EMGListingType::FixedPrice ? A.Price : A.CurrentBid;
            int64 PriceB = B.ListingType == EMGListingType::FixedPrice ? B.Price : B.CurrentBid;
            return Filter.bSortDescending ? PriceA > PriceB : PriceA < PriceB;
        });
    }
    else if (Filter.SortBy == FName("Listed"))
    {
        Results.Sort([Filter](const FMGMarketplaceListing& A, const FMGMarketplaceListing& B)
        {
            return Filter.bSortDescending ? A.ListedTime > B.ListedTime : A.ListedTime < B.ListedTime;
        });
    }

    // Paginate
    int32 StartIndex = Filter.PageNumber * Filter.PageSize;
    int32 EndIndex = FMath::Min(StartIndex + Filter.PageSize, Results.Num());

    TArray<FMGMarketplaceListing> PagedResults;
    for (int32 i = StartIndex; i < EndIndex; i++)
    {
        PagedResults.Add(Results[i]);
    }

    OnMarketSearchComplete.Broadcast(PagedResults);
}

void UMGMarketplaceSubsystem::GetFeaturedListings()
{
    TArray<FMGMarketplaceListing> Featured;

    for (const auto& Pair : AllListings)
    {
        if (Pair.Value.Status == EMGListingStatus::Active &&
            (Pair.Value.Item.Rarity == EMGMarketRarity::Legendary ||
             Pair.Value.Item.Rarity == EMGMarketRarity::Mythic ||
             Pair.Value.Item.bIsLimitedEdition))
        {
            Featured.Add(Pair.Value);
        }
    }

    OnMarketSearchComplete.Broadcast(Featured);
}

void UMGMarketplaceSubsystem::GetEndingSoonListings(int32 Count)
{
    TArray<FMGMarketplaceListing> EndingSoon;

    for (const auto& Pair : AllListings)
    {
        if (Pair.Value.Status == EMGListingStatus::Active && Pair.Value.IsEndingSoon())
        {
            EndingSoon.Add(Pair.Value);
        }
    }

    EndingSoon.Sort([](const FMGMarketplaceListing& A, const FMGMarketplaceListing& B)
    {
        return A.EndTime < B.EndTime;
    });

    if (EndingSoon.Num() > Count)
    {
        EndingSoon.SetNum(Count);
    }

    OnMarketSearchComplete.Broadcast(EndingSoon);
}

void UMGMarketplaceSubsystem::GetRecentlySold(int32 Count)
{
    TArray<FMGMarketplaceListing> RecentlySold;

    for (const auto& Pair : AllListings)
    {
        if (Pair.Value.Status == EMGListingStatus::Sold)
        {
            RecentlySold.Add(Pair.Value);
        }
    }

    RecentlySold.Sort([](const FMGMarketplaceListing& A, const FMGMarketplaceListing& B)
    {
        return A.EndTime > B.EndTime;
    });

    if (RecentlySold.Num() > Count)
    {
        RecentlySold.SetNum(Count);
    }

    OnMarketSearchComplete.Broadcast(RecentlySold);
}

// ===== Watch List =====

bool UMGMarketplaceSubsystem::AddToWatchList(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing)
    {
        return false;
    }

    WatchedListingIds.AddUnique(ListingId);
    Listing->bIsWatched = true;
    Listing->WatchCount++;

    return true;
}

bool UMGMarketplaceSubsystem::RemoveFromWatchList(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (Listing)
    {
        Listing->bIsWatched = false;
        Listing->WatchCount = FMath::Max(0, Listing->WatchCount - 1);
    }

    return WatchedListingIds.Remove(ListingId) > 0;
}

TArray<FMGMarketplaceListing> UMGMarketplaceSubsystem::GetWatchList() const
{
    TArray<FMGMarketplaceListing> Results;

    for (const FString& ListingId : WatchedListingIds)
    {
        if (const FMGMarketplaceListing* Listing = AllListings.Find(ListingId))
        {
            Results.Add(*Listing);
        }
    }

    return Results;
}

bool UMGMarketplaceSubsystem::IsWatched(const FString& ListingId) const
{
    return WatchedListingIds.Contains(ListingId);
}

// ===== Price History =====

void UMGMarketplaceSubsystem::FetchPriceHistory(FName ItemId, int32 DaysBack)
{
    // In production, would fetch from server
    // Generate sample data
    FMGPriceHistory History;
    History.ItemId = ItemId;
    History.TotalSold = FMath::RandRange(10, 100);

    int64 BasePrice = FMath::RandRange(10000, 100000);
    History.LowestPrice = BasePrice * 0.7f;
    History.HighestPrice = BasePrice * 1.5f;
    History.AveragePrice = BasePrice;
    History.LastSoldPrice = BasePrice + FMath::RandRange(-5000, 5000);

    FDateTime Now = FDateTime::Now();
    for (int32 i = DaysBack; i >= 0; i--)
    {
        History.Dates.Add(Now - FTimespan::FromDays(i));
        History.Prices.Add(BasePrice + FMath::RandRange(-10000, 10000));
    }

    PriceHistories.Add(ItemId, History);
}

FMGPriceHistory UMGMarketplaceSubsystem::GetPriceHistory(FName ItemId) const
{
    if (const FMGPriceHistory* History = PriceHistories.Find(ItemId))
    {
        return *History;
    }
    return FMGPriceHistory();
}

// ===== Stats =====

FMGMarketplaceStats UMGMarketplaceSubsystem::GetMarketplaceStats() const
{
    FMGMarketplaceStats Stats;

    for (const auto& Pair : AllListings)
    {
        if (Pair.Value.Status == EMGListingStatus::Active)
        {
            Stats.ActiveListings++;
        }
    }

    Stats.MyActiveListings = MyListingIds.Num();
    Stats.MyActiveBids = MyBidIds.Num();
    Stats.PendingTrades = PendingTrades.Num();

    return Stats;
}

int64 UMGMarketplaceSubsystem::GetMarketplaceFee(int64 SalePrice) const
{
    return FMath::RoundToInt(SalePrice * (MarketplaceFeePercent / 100.0f));
}

// ===== Protected =====

void UMGMarketplaceSubsystem::ProcessAuctions()
{
    FDateTime Now = FDateTime::Now();

    for (auto& Pair : AllListings)
    {
        FMGMarketplaceListing& Listing = Pair.Value;

        if (Listing.Status == EMGListingStatus::Active &&
            (Listing.ListingType == EMGListingType::Auction || Listing.ListingType == EMGListingType::BuyNow))
        {
            if (Now >= Listing.EndTime)
            {
                // Auction ended
                if (Listing.BidHistory.Num() > 0 && (!Listing.bHasReserve || Listing.bReserveMet))
                {
                    Listing.Status = EMGListingStatus::Sold;
                    FString WinnerId = Listing.BidHistory.Last().BidderId;

                    if (WinnerId == CurrentPlayerId)
                    {
                        OnAuctionWon.Broadcast(Listing);
                    }

                    OnListingSold.Broadcast(Listing);
                }
                else
                {
                    Listing.Status = EMGListingStatus::Expired;
                }
            }
        }
    }
}

void UMGMarketplaceSubsystem::CheckExpiringListings()
{
    FDateTime Now = FDateTime::Now();

    for (const FString& ListingId : WatchedListingIds)
    {
        if (const FMGMarketplaceListing* Listing = AllListings.Find(ListingId))
        {
            if (Listing->Status == EMGListingStatus::Active)
            {
                FTimespan Remaining = Listing->EndTime - Now;
                float MinutesRemaining = Remaining.GetTotalMinutes();

                // Notify at 60, 30, 15, 5, 1 minute marks
                if (MinutesRemaining <= 60 && MinutesRemaining > 59)
                {
                    OnListingExpiring.Broadcast(ListingId, 60.0f);
                }
                else if (MinutesRemaining <= 5 && MinutesRemaining > 4)
                {
                    OnListingExpiring.Broadcast(ListingId, 5.0f);
                }
            }
        }
    }
}

void UMGMarketplaceSubsystem::ProcessAutoBids(const FString& ListingId)
{
    FMGMarketplaceListing* Listing = AllListings.Find(ListingId);
    if (!Listing)
    {
        return;
    }

    // Find auto-bidders who can outbid current
    for (const FMGBidInfo& Bid : Listing->BidHistory)
    {
        if (Bid.bIsAutoBid && Bid.BidderId != Listing->BidHistory.Last().BidderId)
        {
            int64 RequiredBid = Listing->CurrentBid + Listing->MinBidIncrement;
            if (Bid.MaxAutoBid >= RequiredBid)
            {
                // Place auto bid
                int64 AutoBidAmount = FMath::Min(RequiredBid, Bid.MaxAutoBid);

                FMGBidInfo AutoBid;
                AutoBid.BidId = FGuid::NewGuid().ToString();
                AutoBid.BidderId = Bid.BidderId;
                AutoBid.BidderName = Bid.BidderName;
                AutoBid.BidAmount = AutoBidAmount;
                AutoBid.BidTime = FDateTime::Now();
                AutoBid.bIsAutoBid = true;
                AutoBid.MaxAutoBid = Bid.MaxAutoBid;

                Listing->CurrentBid = AutoBidAmount;
                Listing->BidHistory.Add(AutoBid);

                OnBidPlaced.Broadcast(ListingId, AutoBid);
                break;
            }
        }
    }
}

void UMGMarketplaceSubsystem::InitializeSampleListings()
{
    FDateTime Now = FDateTime::Now();

    // Sample vehicle listing
    {
        FMGMarketItem VehicleItem;
        VehicleItem.ItemId = FName("vehicle_nissan_silvia_s15");
        VehicleItem.ItemName = FText::FromString(TEXT("Nissan Silvia S15 Spec-R"));
        VehicleItem.Description = FText::FromString(TEXT("Fully built drift-spec S15 with SR20DET. 400hp, HKS coilovers, full aero kit."));
        VehicleItem.Category = EMGMarketCategory::Vehicles;
        VehicleItem.Rarity = EMGMarketRarity::Epic;
        VehicleItem.bIsUnique = false;
        VehicleItem.Attributes.Add(FName("Power"), TEXT("400hp"));
        VehicleItem.Attributes.Add(FName("Mileage"), TEXT("45,000"));

        FMGMarketplaceListing VehicleListing;
        VehicleListing.ListingId = FGuid::NewGuid().ToString();
        VehicleListing.Item = VehicleItem;
        VehicleListing.ListingType = EMGListingType::Auction;
        VehicleListing.Status = EMGListingStatus::Active;
        VehicleListing.StartingBid = 250000;
        VehicleListing.CurrentBid = 325000;
        VehicleListing.BuyNowPrice = 500000;
        VehicleListing.MinBidIncrement = 5000;
        VehicleListing.ListedTime = Now - FTimespan::FromDays(2);
        VehicleListing.EndTime = Now + FTimespan::FromHours(6);
        VehicleListing.ViewCount = 1250;
        VehicleListing.WatchCount = 89;
        VehicleListing.Seller.SellerId = TEXT("seller_drift_king");
        VehicleListing.Seller.SellerName = TEXT("DriftKing_JDM");
        VehicleListing.Seller.SellerRating = 4.9f;
        VehicleListing.Seller.TotalSales = 156;
        VehicleListing.Seller.bIsVerified = true;

        AllListings.Add(VehicleListing.ListingId, VehicleListing);
    }

    // Sample vinyl listing
    {
        FMGMarketItem VinylItem;
        VinylItem.ItemId = FName("vinyl_midnight_purple");
        VinylItem.ItemName = FText::FromString(TEXT("Midnight Purple III Custom Livery"));
        VinylItem.Description = FText::FromString(TEXT("Authentic Midnight Purple III recreation with custom fade effects."));
        VinylItem.Category = EMGMarketCategory::Vinyls;
        VinylItem.Rarity = EMGMarketRarity::Rare;

        FMGMarketplaceListing VinylListing;
        VinylListing.ListingId = FGuid::NewGuid().ToString();
        VinylListing.Item = VinylItem;
        VinylListing.ListingType = EMGListingType::FixedPrice;
        VinylListing.Status = EMGListingStatus::Active;
        VinylListing.Price = 15000;
        VinylListing.ListedTime = Now - FTimespan::FromHours(12);
        VinylListing.EndTime = Now + FTimespan::FromDays(7);
        VinylListing.ViewCount = 340;
        VinylListing.WatchCount = 28;
        VinylListing.Seller.SellerId = TEXT("seller_vinyl_pro");
        VinylListing.Seller.SellerName = TEXT("VinylArtist");
        VinylListing.Seller.SellerRating = 4.7f;
        VinylListing.Seller.TotalSales = 89;
        VinylListing.Seller.bIsVerified = true;

        AllListings.Add(VinylListing.ListingId, VinylListing);
    }

    // Sample limited edition wheels
    {
        FMGMarketItem WheelsItem;
        WheelsItem.ItemId = FName("wheels_work_meister_le");
        WheelsItem.ItemName = FText::FromString(TEXT("Work Meister S1 Limited Edition"));
        WheelsItem.Description = FText::FromString(TEXT("Limited edition Work Meister S1 3-piece wheels. Only 50 sets made."));
        WheelsItem.Category = EMGMarketCategory::Wheels;
        WheelsItem.Rarity = EMGMarketRarity::Legendary;
        WheelsItem.bIsLimitedEdition = true;
        WheelsItem.EditionNumber = 23;
        WheelsItem.TotalEditions = 50;

        FMGMarketplaceListing WheelsListing;
        WheelsListing.ListingId = FGuid::NewGuid().ToString();
        WheelsListing.Item = WheelsItem;
        WheelsListing.ListingType = EMGListingType::BuyNow;
        WheelsListing.Status = EMGListingStatus::Active;
        WheelsListing.StartingBid = 100000;
        WheelsListing.CurrentBid = 175000;
        WheelsListing.BuyNowPrice = 350000;
        WheelsListing.ReservePrice = 150000;
        WheelsListing.bHasReserve = true;
        WheelsListing.bReserveMet = true;
        WheelsListing.MinBidIncrement = 10000;
        WheelsListing.ListedTime = Now - FTimespan::FromDays(1);
        WheelsListing.EndTime = Now + FTimespan::FromMinutes(45);
        WheelsListing.ViewCount = 2100;
        WheelsListing.WatchCount = 156;
        WheelsListing.Seller.SellerId = TEXT("seller_wheel_dealer");
        WheelsListing.Seller.SellerName = TEXT("WheelDealer_Premium");
        WheelsListing.Seller.SellerRating = 5.0f;
        WheelsListing.Seller.TotalSales = 312;
        WheelsListing.Seller.bIsVerified = true;
        WheelsListing.Seller.bIsPremiumSeller = true;

        AllListings.Add(WheelsListing.ListingId, WheelsListing);
    }
}
