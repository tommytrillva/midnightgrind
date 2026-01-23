// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPlayerMarketSubsystem.generated.h"

class UMGEconomySubsystem;

/**
 * Listing type for market items
 */
UENUM(BlueprintType)
enum class EMGListingType : uint8
{
	Auction UMETA(DisplayName = "Auction"),
	BuyNow UMETA(DisplayName = "Buy Now"),
	Classified UMETA(DisplayName = "Classified Ad")
};

/**
 * Item type that can be listed
 */
UENUM(BlueprintType)
enum class EMGMarketItemType : uint8
{
	Vehicle UMETA(DisplayName = "Vehicle"),
	Part UMETA(DisplayName = "Part"),
	Cosmetic UMETA(DisplayName = "Cosmetic"),
	Livery UMETA(DisplayName = "Livery/Wrap")
};

/**
 * Listing status
 */
UENUM(BlueprintType)
enum class EMGListingStatus : uint8
{
	Active UMETA(DisplayName = "Active"),
	Sold UMETA(DisplayName = "Sold"),
	Expired UMETA(DisplayName = "Expired"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	Pending UMETA(DisplayName = "Pending Review")
};

/**
 * Trade offer status
 */
UENUM(BlueprintType)
enum class EMGTradeStatus : uint8
{
	Pending UMETA(DisplayName = "Pending"),
	Accepted UMETA(DisplayName = "Accepted"),
	Rejected UMETA(DisplayName = "Rejected"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	Expired UMETA(DisplayName = "Expired"),
	Completed UMETA(DisplayName = "Completed")
};

/**
 * Bid information
 */
USTRUCT(BlueprintType)
struct FMGBidInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid BidID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid BidderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BidderDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BidAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BidTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MaxAutoBidAmount = 0; // For autobid feature

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutoBid = false;
};

/**
 * Market listing
 */
USTRUCT(BlueprintType)
struct FMGMarketListing
{
	GENERATED_BODY()

	// Identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ListingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGListingType ListingType = EMGListingType::Auction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMarketItemType ItemType = EMGMarketItemType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGListingStatus Status = EMGListingStatus::Active;

	// Seller info
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SellerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SellerDisplayName;

	// Item info
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemID; // Vehicle GUID, Part ID, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemImagePath;

	// Vehicle-specific (if vehicle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerformanceIndex = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Mileage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OwnerCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RaceWins = 0;

	// Pricing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 StartingPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BuyNowPrice = 0; // 0 = no buy now option

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReservePrice = 0; // Minimum price to sell, 0 = no reserve

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentBid = 0;

	// Bidding
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGBidInfo> BidHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid HighestBidderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BidCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WatchCount = 0;

	// Timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ListedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSnipeProtectionActive = false; // Extends time if bid in last 60 sec

	// Restrictions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVehicleLocked = false; // Pink slip won vehicle locked for 7 days

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LockExpiresTime;

	FMGMarketListing()
	{
		ListingID = FGuid::NewGuid();
		ListedTime = FDateTime::Now();
	}
};

/**
 * Trade offer between players
 */
USTRUCT(BlueprintType)
struct FMGTradeOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TradeID;

	// Initiator
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InitiatorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InitiatorDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> InitiatorItems; // Items offered

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 InitiatorCash = 0; // Cash offered

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInitiatorConfirmed = false;

	// Recipient
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RecipientID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecipientDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> RecipientItems; // Items requested

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RecipientCash = 0; // Cash requested

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecipientConfirmed = false;

	// Status
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTradeStatus Status = EMGTradeStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	FMGTradeOffer()
	{
		TradeID = FGuid::NewGuid();
		CreatedTime = FDateTime::Now();
		ExpiresTime = FDateTime::Now() + FTimespan::FromHours(24);
	}
};

/**
 * Search filter for market
 */
USTRUCT(BlueprintType)
struct FMGMarketSearchFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMarketItemType ItemType = EMGMarketItemType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGListingType ListingType = EMGListingType::Auction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SearchText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 YearMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 YearMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PriceMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PriceMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMax = 999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBuyNowOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEndingSoon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNewListingsOnly = false;

	// Sort
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SortBy = TEXT("EndTime"); // EndTime, Price, PI, BidCount, ListedTime

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSortAscending = true;
};

/**
 * Market transaction record
 */
USTRUCT(BlueprintType)
struct FMGMarketTransaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TransactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ListingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SellerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid BuyerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 SalePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MarketFee = 0; // 5% fee

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 SellerReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime TransactionTime;

	FMGMarketTransaction()
	{
		TransactionID = FGuid::NewGuid();
		TransactionTime = FDateTime::Now();
	}
};

/**
 * Fraud detection flag
 */
USTRUCT(BlueprintType)
struct FMGFraudFlag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FlagReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuspicionScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FlagTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> RelatedTransactions;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListingCreated, const FGuid&, ListingID, const FMGMarketListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBidPlaced, const FGuid&, ListingID, const FMGBidInfo&, Bid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListingSold, const FGuid&, ListingID, const FMGMarketTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListingExpired, const FGuid&, ListingID, const FMGMarketListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeOfferReceived, const FMGTradeOffer&, Offer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, const FGuid&, TradeID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutbid, const FMGBidInfo&, NewHighBid);

/**
 * Player Market / Auction House subsystem
 * Handles player-to-player vehicle and item trading
 * Per PRD Section 4.4: Marketplace features
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPlayerMarketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// LISTING MANAGEMENT
	// ==========================================

	/**
	 * Create a new auction listing
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Listing")
	bool CreateAuctionListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
		int64 StartingPrice, int64 BuyNowPrice, int64 ReservePrice,
		float DurationHours, FGuid& OutListingID);

	/**
	 * Create a buy-now only listing
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Listing")
	bool CreateBuyNowListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
		int64 Price, float DurationHours, FGuid& OutListingID);

	/**
	 * Create a classified ad (contact seller)
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Listing")
	bool CreateClassifiedListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
		int64 AskingPrice, const FString& Description, FGuid& OutListingID);

	/**
	 * Cancel a listing (before any bids)
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Listing")
	bool CancelListing(FGuid SellerID, FGuid ListingID);

	/**
	 * Get listing details
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Listing")
	bool GetListing(FGuid ListingID, FMGMarketListing& OutListing) const;

	/**
	 * Get all active listings by seller
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Listing")
	TArray<FMGMarketListing> GetSellerListings(FGuid SellerID) const;

	// ==========================================
	// BIDDING
	// ==========================================

	/**
	 * Place a bid on an auction
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Bidding")
	bool PlaceBid(FGuid BidderID, FGuid ListingID, int64 BidAmount, bool bAutoBid, int64 MaxAutoBidAmount);

	/**
	 * Execute buy now
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Bidding")
	bool ExecuteBuyNow(FGuid BuyerID, FGuid ListingID);

	/**
	 * Set/update autobid maximum
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Bidding")
	bool SetAutoBid(FGuid BidderID, FGuid ListingID, int64 MaxAmount);

	/**
	 * Get minimum next bid amount
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Bidding")
	int64 GetMinimumBidIncrement(int64 CurrentBid) const;

	// ==========================================
	// SEARCH & BROWSE
	// ==========================================

	/**
	 * Search listings with filters
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Search")
	TArray<FMGMarketListing> SearchListings(const FMGMarketSearchFilter& Filter, int32 MaxResults = 50);

	/**
	 * Get featured listings (high activity, ending soon)
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Search")
	TArray<FMGMarketListing> GetFeaturedListings(int32 MaxResults = 10) const;

	/**
	 * Get listings ending soon
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Search")
	TArray<FMGMarketListing> GetEndingSoonListings(int32 MaxResults = 20) const;

	/**
	 * Get newly listed items
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Search")
	TArray<FMGMarketListing> GetNewListings(int32 MaxResults = 20) const;

	// ==========================================
	// WATCHLIST
	// ==========================================

	/**
	 * Add listing to player's watchlist
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Watchlist")
	void AddToWatchlist(FGuid PlayerID, FGuid ListingID);

	/**
	 * Remove from watchlist
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Watchlist")
	void RemoveFromWatchlist(FGuid PlayerID, FGuid ListingID);

	/**
	 * Get player's watchlist
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Watchlist")
	TArray<FMGMarketListing> GetWatchlist(FGuid PlayerID) const;

	// ==========================================
	// DIRECT TRADING
	// ==========================================

	/**
	 * Create a trade offer to another player
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Trade")
	bool CreateTradeOffer(FGuid InitiatorID, FGuid RecipientID,
		const TArray<FGuid>& OfferedItems, int64 OfferedCash,
		const TArray<FGuid>& RequestedItems, int64 RequestedCash,
		const FString& Message, FGuid& OutTradeID);

	/**
	 * Accept a trade offer
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Trade")
	bool AcceptTradeOffer(FGuid RecipientID, FGuid TradeID);

	/**
	 * Reject a trade offer
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Trade")
	bool RejectTradeOffer(FGuid RecipientID, FGuid TradeID);

	/**
	 * Cancel a trade offer (initiator only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Trade")
	bool CancelTradeOffer(FGuid InitiatorID, FGuid TradeID);

	/**
	 * Counter-offer (creates new trade)
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Trade")
	bool CreateCounterOffer(FGuid RecipientID, FGuid OriginalTradeID,
		const TArray<FGuid>& CounterItems, int64 CounterCash, FGuid& OutTradeID);

	/**
	 * Get pending trades for player
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Trade")
	TArray<FMGTradeOffer> GetPendingTrades(FGuid PlayerID) const;

	// ==========================================
	// PRICING
	// ==========================================

	/**
	 * Get estimated market value for a vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Pricing")
	int64 GetEstimatedMarketValue(FGuid VehicleID) const;

	/**
	 * Get price history for similar items
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Pricing")
	TArray<FMGMarketTransaction> GetPriceHistory(FName ModelID, int32 DaysBack = 30) const;

	/**
	 * Get minimum allowed listing price
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Pricing")
	int64 GetMinimumListingPrice(FGuid ItemID, EMGMarketItemType ItemType) const;

	/**
	 * Get maximum allowed listing price
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Pricing")
	int64 GetMaximumListingPrice(FGuid ItemID, EMGMarketItemType ItemType) const;

	// ==========================================
	// FEES & ECONOMY
	// ==========================================

	/**
	 * Calculate market fee for a sale
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Economy")
	int64 CalculateMarketFee(int64 SalePrice) const;

	/**
	 * Get listing fee (charged when creating listing)
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Economy")
	int64 GetListingFee(EMGMarketItemType ItemType, EMGListingType ListingType) const;

	// ==========================================
	// FRAUD DETECTION
	// ==========================================

	/**
	 * Check if transaction seems suspicious
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Security")
	bool IsTransactionSuspicious(FGuid SellerID, FGuid BuyerID, int64 Amount) const;

	/**
	 * Get player's fraud risk score
	 */
	UFUNCTION(BlueprintPure, Category = "Market|Security")
	float GetPlayerFraudScore(FGuid PlayerID) const;

	/**
	 * Flag suspicious activity
	 */
	UFUNCTION(BlueprintCallable, Category = "Market|Security")
	void FlagSuspiciousActivity(const FMGFraudFlag& Flag);

	// ==========================================
	// HISTORY
	// ==========================================

	/**
	 * Get player's purchase history
	 */
	UFUNCTION(BlueprintPure, Category = "Market|History")
	TArray<FMGMarketTransaction> GetPurchaseHistory(FGuid PlayerID, int32 MaxResults = 50) const;

	/**
	 * Get player's sale history
	 */
	UFUNCTION(BlueprintPure, Category = "Market|History")
	TArray<FMGMarketTransaction> GetSaleHistory(FGuid PlayerID, int32 MaxResults = 50) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnListingCreated OnListingCreated;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnBidPlaced OnBidPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnListingSold OnListingSold;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnListingExpired OnListingExpired;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnTradeOfferReceived OnTradeOfferReceived;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Market|Events")
	FOnOutbid OnOutbid;

protected:
	// Active listings
	UPROPERTY()
	TMap<FGuid, FMGMarketListing> ActiveListings;

	// Trade offers
	UPROPERTY()
	TMap<FGuid, FMGTradeOffer> PendingTrades;

	// Transaction history (would be backed by database in production)
	UPROPERTY()
	TArray<FMGMarketTransaction> TransactionHistory;

	// Watchlists per player
	TMap<FGuid, TArray<FGuid>> PlayerWatchlists;

	// Fraud flags
	UPROPERTY()
	TArray<FMGFraudFlag> FraudFlags;

	// Timer for auction updates
	FTimerHandle AuctionTickTimer;

	// Economy subsystem reference
	UPROPERTY()
	UMGEconomySubsystem* EconomySubsystem;

	// ==========================================
	// INTERNAL
	// ==========================================

	void OnAuctionTick();
	void ProcessEndedListings();
	void ProcessExpiredTrades();
	void FinalizeAuctionSale(FMGMarketListing& Listing);
	bool TransferItem(FGuid ItemID, EMGMarketItemType ItemType, FGuid FromPlayerID, FGuid ToPlayerID);
	void NotifyOutbidPlayers(const FMGMarketListing& Listing, FGuid NewHighBidderID);
	bool IsVehicleTradeLocked(FGuid VehicleID) const;
	void ApplySnipeProtection(FMGMarketListing& Listing);
	bool ValidatePricing(int64 Price, FGuid ItemID, EMGMarketItemType ItemType) const;
	void RecordTransaction(const FMGMarketTransaction& Transaction);
	void UpdateFraudScores(const FMGMarketTransaction& Transaction);

	// Market fee percentage (5% per PRD)
	static constexpr float MarketFeePercent = 0.05f;

	// Snipe protection window (seconds)
	static constexpr float SnipeProtectionWindow = 60.0f;

	// Snipe protection extension (seconds)
	static constexpr float SnipeProtectionExtension = 120.0f;

	// Minimum bid increment thresholds
	int64 GetBidIncrement(int64 CurrentPrice) const;
};
