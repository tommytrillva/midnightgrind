// MidnightGrind - Arcade Street Racing Game
// Marketplace Subsystem - Player-to-player trading, auctions, and marketplace

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMarketplaceSubsystem.generated.h"

// Forward declarations
class UMGMarketplaceSubsystem;

/**
 * EMGListingType - Type of marketplace listing
 */
UENUM(BlueprintType)
enum class EMGListingType : uint8
{
    FixedPrice      UMETA(DisplayName = "Fixed Price"),
    Auction         UMETA(DisplayName = "Auction"),
    BuyNow          UMETA(DisplayName = "Auction with Buy Now"),
    Trade           UMETA(DisplayName = "Trade Request")
};

/**
 * EMGListingStatus - Status of a listing
 */
UENUM(BlueprintType)
enum class EMGListingStatus : uint8
{
    Active          UMETA(DisplayName = "Active"),
    Pending         UMETA(DisplayName = "Pending Approval"),
    Sold            UMETA(DisplayName = "Sold"),
    Expired         UMETA(DisplayName = "Expired"),
    Cancelled       UMETA(DisplayName = "Cancelled"),
    Disputed        UMETA(DisplayName = "Disputed")
};

/**
 * EMGMarketCategory - Category of marketplace items
 */
UENUM(BlueprintType)
enum class EMGMarketCategory : uint8
{
    Vehicles        UMETA(DisplayName = "Vehicles"),
    BodyKits        UMETA(DisplayName = "Body Kits"),
    Vinyls          UMETA(DisplayName = "Vinyls & Liveries"),
    Wheels          UMETA(DisplayName = "Wheels"),
    Spoilers        UMETA(DisplayName = "Spoilers"),
    Interiors       UMETA(DisplayName = "Interiors"),
    PerformanceParts UMETA(DisplayName = "Performance Parts"),
    Cosmetics       UMETA(DisplayName = "Cosmetics"),
    Bundles         UMETA(DisplayName = "Bundles"),
    Collectibles    UMETA(DisplayName = "Collectibles"),
    Limited         UMETA(DisplayName = "Limited Edition")
};

/**
 * EMGMarketRarity - Rarity classification
 */
UENUM(BlueprintType)
enum class EMGMarketRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary"),
    Mythic          UMETA(DisplayName = "Mythic"),
    Exclusive       UMETA(DisplayName = "Exclusive")
};

/**
 * FMGMarketplaceSeller - Seller information
 */
USTRUCT(BlueprintType)
struct FMGMarketplaceSeller
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SellerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SellerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SellerRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSales;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveFeedback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeFeedback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime MemberSince;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVerified;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPremiumSeller;

    FMGMarketplaceSeller()
        : SellerRating(0.0f)
        , TotalSales(0)
        , PositiveFeedback(0)
        , NegativeFeedback(0)
        , bIsVerified(false)
        , bIsPremiumSeller(false)
    {}

    float GetPositiveFeedbackPercent() const
    {
        int32 Total = PositiveFeedback + NegativeFeedback;
        return Total > 0 ? (static_cast<float>(PositiveFeedback) / Total) * 100.0f : 0.0f;
    }
};

/**
 * FMGBidInfo - Information about a bid
 */
USTRUCT(BlueprintType)
struct FMGBidInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BidId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BidderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BidderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BidAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime BidTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAutoBid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxAutoBid;

    FMGBidInfo()
        : BidAmount(0)
        , bIsAutoBid(false)
        , MaxAutoBid(0)
    {}
};

/**
 * FMGMarketItem - Item being sold in marketplace
 */
USTRUCT(BlueprintType)
struct FMGMarketItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMarketCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMarketRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ThumbnailTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UTexture2D>> GalleryImages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> Attributes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnique;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLimitedEdition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EditionNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalEditions;

    FMGMarketItem()
        : Category(EMGMarketCategory::Cosmetics)
        , Rarity(EMGMarketRarity::Common)
        , Quantity(1)
        , bIsUnique(false)
        , bIsLimitedEdition(false)
        , EditionNumber(0)
        , TotalEditions(0)
    {}
};

/**
 * FMGMarketplaceListing - A marketplace listing
 */
USTRUCT(BlueprintType)
struct FMGMarketplaceListing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ListingId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGMarketItem Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGMarketplaceSeller Seller;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGListingType ListingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGListingStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 Price;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 StartingBid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 CurrentBid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BuyNowPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 ReservePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MinBidIncrement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBidInfo> BidHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ListedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ViewCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WatchCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWatched;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasReserve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bReserveMet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrencyType;

    FMGMarketplaceListing()
        : ListingType(EMGListingType::FixedPrice)
        , Status(EMGListingStatus::Active)
        , Price(0)
        , StartingBid(0)
        , CurrentBid(0)
        , BuyNowPrice(0)
        , ReservePrice(0)
        , MinBidIncrement(100)
        , ViewCount(0)
        , WatchCount(0)
        , bIsWatched(false)
        , bHasReserve(false)
        , bReserveMet(false)
        , CurrencyType(FName("Credits"))
    {}

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndTime > Now ? EndTime - Now : FTimespan::Zero();
    }

    bool IsEndingSoon() const
    {
        return GetTimeRemaining().GetTotalHours() < 1.0;
    }
};

/**
 * FMGTradeOffer - A trade offer between players
 */
USTRUCT(BlueprintType)
struct FMGTradeOffer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TradeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InitiatorId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RecipientId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMarketItem> InitiatorItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMarketItem> RecipientItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 InitiatorCurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 RecipientCurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ExpiresTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInitiatorConfirmed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRecipientConfirmed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCounterOffer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OriginalTradeId;

    FMGTradeOffer()
        : InitiatorCurrency(0)
        , RecipientCurrency(0)
        , bInitiatorConfirmed(false)
        , bRecipientConfirmed(false)
        , bIsCounterOffer(false)
    {}
};

/**
 * FMGMarketSearchFilter - Search filter for marketplace
 */
USTRUCT(BlueprintType)
struct FMGMarketSearchFilter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SearchQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGMarketCategory> Categories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGMarketRarity> Rarities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MinPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGListingType ListingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SortBy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSortDescending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEndingSoonOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBuyNowOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bVerifiedSellersOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PageSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PageNumber;

    FMGMarketSearchFilter()
        : MinPrice(0)
        , MaxPrice(0)
        , ListingType(EMGListingType::FixedPrice)
        , SortBy(FName("EndTime"))
        , bSortDescending(false)
        , bEndingSoonOnly(false)
        , bBuyNowOnly(false)
        , bVerifiedSellersOnly(false)
        , PageSize(20)
        , PageNumber(0)
    {}
};

/**
 * FMGMarketplaceStats - Marketplace statistics
 */
USTRUCT(BlueprintType)
struct FMGMarketplaceStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveListings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSalesToday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalVolumeToday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MyActiveListings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MyActiveBids;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PendingTrades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MyTotalSales;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MyTotalPurchases;

    FMGMarketplaceStats()
        : ActiveListings(0)
        , TotalSalesToday(0)
        , TotalVolumeToday(0)
        , MyActiveListings(0)
        , MyActiveBids(0)
        , PendingTrades(0)
        , MyTotalSales(0)
        , MyTotalPurchases(0)
    {}
};

/**
 * FMGPriceHistory - Historical price data
 */
USTRUCT(BlueprintType)
struct FMGPriceHistory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int64> Prices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDateTime> Dates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 LowestPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 HighestPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AveragePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 LastSoldPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSold;

    FMGPriceHistory()
        : ItemId(NAME_None)
        , LowestPrice(0)
        , HighestPrice(0)
        , AveragePrice(0)
        , LastSoldPrice(0)
        , TotalSold(0)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnListingCreated, const FMGMarketplaceListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnListingSold, const FMGMarketplaceListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBidPlaced, const FString&, ListingId, const FMGBidInfo&, Bid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBidOutbid, const FString&, ListingId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAuctionWon, const FMGMarketplaceListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTradeOfferReceived, const FMGTradeOffer&, Offer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTradeCompleted, const FMGTradeOffer&, Trade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMarketSearchComplete, const TArray<FMGMarketplaceListing>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnListingExpiring, const FString&, ListingId, float, MinutesRemaining);

/**
 * UMGMarketplaceSubsystem
 *
 * Manages the player marketplace for Midnight Grind.
 * Features include:
 * - Fixed price listings
 * - Auctions with bidding
 * - Player-to-player trades
 * - Price history tracking
 * - Search and filtering
 * - Watch lists
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMarketplaceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGMarketplaceSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void TickMarketplace(float DeltaTime);

    // ===== Listings =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Listings")
    bool CreateListing(const FMGMarketItem& Item, EMGListingType Type, int64 Price, FTimespan Duration);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Listings")
    bool CreateAuction(const FMGMarketItem& Item, int64 StartingBid, int64 ReservePrice, int64 BuyNowPrice, FTimespan Duration);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Listings")
    bool CancelListing(const FString& ListingId);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Listings")
    bool BuyListing(const FString& ListingId);

    UFUNCTION(BlueprintPure, Category = "Marketplace|Listings")
    FMGMarketplaceListing GetListing(const FString& ListingId) const;

    UFUNCTION(BlueprintPure, Category = "Marketplace|Listings")
    TArray<FMGMarketplaceListing> GetMyListings() const;

    // ===== Bidding =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Bidding")
    bool PlaceBid(const FString& ListingId, int64 BidAmount);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Bidding")
    bool SetAutoBid(const FString& ListingId, int64 MaxBid);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Bidding")
    bool CancelAutoBid(const FString& ListingId);

    UFUNCTION(BlueprintPure, Category = "Marketplace|Bidding")
    TArray<FMGMarketplaceListing> GetMyBids() const;

    UFUNCTION(BlueprintPure, Category = "Marketplace|Bidding")
    bool AmIHighestBidder(const FString& ListingId) const;

    // ===== Trading =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Trading")
    bool CreateTradeOffer(const FString& RecipientId, const TArray<FMGMarketItem>& MyItems, const TArray<FMGMarketItem>& WantedItems, int64 MyCurrency, int64 WantedCurrency);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Trading")
    bool AcceptTrade(const FString& TradeId);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Trading")
    bool DeclineTrade(const FString& TradeId);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Trading")
    bool CounterTrade(const FString& TradeId, const TArray<FMGMarketItem>& MyItems, const TArray<FMGMarketItem>& WantedItems, int64 MyCurrency, int64 WantedCurrency);

    UFUNCTION(BlueprintPure, Category = "Marketplace|Trading")
    TArray<FMGTradeOffer> GetPendingTrades() const;

    UFUNCTION(BlueprintPure, Category = "Marketplace|Trading")
    TArray<FMGTradeOffer> GetSentTrades() const;

    // ===== Search =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Search")
    void SearchListings(const FMGMarketSearchFilter& Filter);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Search")
    void GetFeaturedListings();

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Search")
    void GetEndingSoonListings(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Search")
    void GetRecentlySold(int32 Count);

    // ===== Watch List =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Watch")
    bool AddToWatchList(const FString& ListingId);

    UFUNCTION(BlueprintCallable, Category = "Marketplace|Watch")
    bool RemoveFromWatchList(const FString& ListingId);

    UFUNCTION(BlueprintPure, Category = "Marketplace|Watch")
    TArray<FMGMarketplaceListing> GetWatchList() const;

    UFUNCTION(BlueprintPure, Category = "Marketplace|Watch")
    bool IsWatched(const FString& ListingId) const;

    // ===== Price History =====

    UFUNCTION(BlueprintCallable, Category = "Marketplace|History")
    void FetchPriceHistory(FName ItemId, int32 DaysBack);

    UFUNCTION(BlueprintPure, Category = "Marketplace|History")
    FMGPriceHistory GetPriceHistory(FName ItemId) const;

    // ===== Stats =====

    UFUNCTION(BlueprintPure, Category = "Marketplace|Stats")
    FMGMarketplaceStats GetMarketplaceStats() const;

    UFUNCTION(BlueprintPure, Category = "Marketplace|Stats")
    int64 GetMarketplaceFee(int64 SalePrice) const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnListingCreated OnListingCreated;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnListingSold OnListingSold;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnBidPlaced OnBidPlaced;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnBidOutbid OnBidOutbid;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnAuctionWon OnAuctionWon;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnTradeOfferReceived OnTradeOfferReceived;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnTradeCompleted OnTradeCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnMarketSearchComplete OnMarketSearchComplete;

    UPROPERTY(BlueprintAssignable, Category = "Marketplace|Events")
    FMGOnListingExpiring OnListingExpiring;

protected:
    void ProcessAuctions();
    void CheckExpiringListings();
    void ProcessAutoBids(const FString& ListingId);
    void InitializeSampleListings();

private:
    UPROPERTY()
    TMap<FString, FMGMarketplaceListing> AllListings;

    UPROPERTY()
    TArray<FString> MyListingIds;

    UPROPERTY()
    TArray<FString> MyBidIds;

    UPROPERTY()
    TArray<FString> WatchedListingIds;

    UPROPERTY()
    TMap<FString, FMGTradeOffer> PendingTrades;

    UPROPERTY()
    TMap<FName, FMGPriceHistory> PriceHistories;

    UPROPERTY()
    FString CurrentPlayerId;

    UPROPERTY()
    float MarketplaceFeePercent;

    FTimerHandle TickTimerHandle;
};
