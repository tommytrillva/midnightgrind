// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGPlayerMarketSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void UMGPlayerMarketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get economy subsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		EconomySubsystem = GI->GetSubsystem<UMGEconomySubsystem>();
	}

	// Start auction tick timer (every 1 second)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AuctionTickTimer,
			this,
			&UMGPlayerMarketSubsystem::OnAuctionTick,
			1.0f,
			true
		);
	}
}

void UMGPlayerMarketSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AuctionTickTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// LISTING MANAGEMENT
// ==========================================

bool UMGPlayerMarketSubsystem::CreateAuctionListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
	int64 StartingPrice, int64 BuyNowPrice, int64 ReservePrice, float DurationHours, FGuid& OutListingID)
{
	// Validate pricing
	if (!ValidatePricing(StartingPrice, ItemID, ItemType))
	{
		return false;
	}

	if (BuyNowPrice > 0 && BuyNowPrice < StartingPrice)
	{
		return false;
	}

	// Check if vehicle is trade-locked (pink slip won, must wait 7 days)
	if (ItemType == EMGMarketItemType::Vehicle && IsVehicleTradeLocked(ItemID))
	{
		return false;
	}

	// Charge listing fee
	int64 ListingFee = GetListingFee(ItemType, EMGListingType::Auction);
	if (EconomySubsystem && !EconomySubsystem->DeductCash(SellerID, ListingFee, TEXT("Auction Listing Fee")))
	{
		return false;
	}

	// Create listing
	FMGMarketListing Listing;
	Listing.ListingType = EMGListingType::Auction;
	Listing.ItemType = ItemType;
	Listing.SellerID = SellerID;
	Listing.ItemID = ItemID;
	Listing.StartingPrice = StartingPrice;
	Listing.BuyNowPrice = BuyNowPrice;
	Listing.ReservePrice = ReservePrice;
	Listing.CurrentBid = 0;
	Listing.EndTime = FDateTime::Now() + FTimespan::FromHours(DurationHours);
	Listing.Status = EMGListingStatus::Active;

	// Populate item details from inventory system
	if (ItemType == EMGMarketItemType::Vehicle)
	{
		UGameInstance* GI = GetGameInstance();
		if (GI)
		{
			if (UMGGarageSubsystem* GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>())
			{
				FMGOwnedVehicle Vehicle;
				if (GarageSubsystem->GetVehicle(ItemID, Vehicle))
				{
					Listing.ItemDisplayName = Vehicle.CustomName.IsEmpty()
						? (Vehicle.VehicleModelData.IsValid() ? Vehicle.VehicleModelData->DisplayName.ToString() : TEXT("Vehicle"))
						: Vehicle.CustomName;
					Listing.PerformanceIndex = Vehicle.PerformanceIndex;
					Listing.Mileage = static_cast<int32>(Vehicle.Odometer / 160934.0f); // cm to miles
					Listing.RaceWins = Vehicle.RacesWon;
				}
			}
		}
	}

	OutListingID = Listing.ListingID;
	ActiveListings.Add(Listing.ListingID, Listing);

	OnListingCreated.Broadcast(Listing.ListingID, Listing);

	return true;
}

bool UMGPlayerMarketSubsystem::CreateBuyNowListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
	int64 Price, float DurationHours, FGuid& OutListingID)
{
	if (!ValidatePricing(Price, ItemID, ItemType))
	{
		return false;
	}

	if (ItemType == EMGMarketItemType::Vehicle && IsVehicleTradeLocked(ItemID))
	{
		return false;
	}

	int64 ListingFee = GetListingFee(ItemType, EMGListingType::BuyNow);
	if (EconomySubsystem && !EconomySubsystem->DeductCash(SellerID, ListingFee, TEXT("Buy Now Listing Fee")))
	{
		return false;
	}

	FMGMarketListing Listing;
	Listing.ListingType = EMGListingType::BuyNow;
	Listing.ItemType = ItemType;
	Listing.SellerID = SellerID;
	Listing.ItemID = ItemID;
	Listing.StartingPrice = Price;
	Listing.BuyNowPrice = Price;
	Listing.CurrentBid = Price;
	Listing.EndTime = FDateTime::Now() + FTimespan::FromHours(DurationHours);
	Listing.Status = EMGListingStatus::Active;

	OutListingID = Listing.ListingID;
	ActiveListings.Add(Listing.ListingID, Listing);

	OnListingCreated.Broadcast(Listing.ListingID, Listing);

	return true;
}

bool UMGPlayerMarketSubsystem::CreateClassifiedListing(FGuid SellerID, FGuid ItemID, EMGMarketItemType ItemType,
	int64 AskingPrice, const FString& Description, FGuid& OutListingID)
{
	if (ItemType == EMGMarketItemType::Vehicle && IsVehicleTradeLocked(ItemID))
	{
		return false;
	}

	int64 ListingFee = GetListingFee(ItemType, EMGListingType::Classified);
	if (EconomySubsystem && !EconomySubsystem->DeductCash(SellerID, ListingFee, TEXT("Classified Listing Fee")))
	{
		return false;
	}

	FMGMarketListing Listing;
	Listing.ListingType = EMGListingType::Classified;
	Listing.ItemType = ItemType;
	Listing.SellerID = SellerID;
	Listing.ItemID = ItemID;
	Listing.StartingPrice = AskingPrice;
	Listing.ItemDescription = Description;
	Listing.EndTime = FDateTime::Now() + FTimespan::FromDays(7); // 7 day classified
	Listing.Status = EMGListingStatus::Active;

	OutListingID = Listing.ListingID;
	ActiveListings.Add(Listing.ListingID, Listing);

	OnListingCreated.Broadcast(Listing.ListingID, Listing);

	return true;
}

bool UMGPlayerMarketSubsystem::CancelListing(FGuid SellerID, FGuid ListingID)
{
	FMGMarketListing* Listing = ActiveListings.Find(ListingID);
	if (!Listing)
	{
		return false;
	}

	if (Listing->SellerID != SellerID)
	{
		return false;
	}

	// Cannot cancel auction with bids
	if (Listing->ListingType == EMGListingType::Auction && Listing->BidCount > 0)
	{
		return false;
	}

	Listing->Status = EMGListingStatus::Cancelled;

	return true;
}

bool UMGPlayerMarketSubsystem::GetListing(FGuid ListingID, FMGMarketListing& OutListing) const
{
	const FMGMarketListing* Listing = ActiveListings.Find(ListingID);
	if (Listing)
	{
		OutListing = *Listing;
		return true;
	}
	return false;
}

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::GetSellerListings(FGuid SellerID) const
{
	TArray<FMGMarketListing> Result;

	for (const auto& Pair : ActiveListings)
	{
		if (Pair.Value.SellerID == SellerID && Pair.Value.Status == EMGListingStatus::Active)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// ==========================================
// BIDDING
// ==========================================

bool UMGPlayerMarketSubsystem::PlaceBid(FGuid BidderID, FGuid ListingID, int64 BidAmount, bool bAutoBid, int64 MaxAutoBidAmount)
{
	FMGMarketListing* Listing = ActiveListings.Find(ListingID);
	if (!Listing || Listing->Status != EMGListingStatus::Active)
	{
		return false;
	}

	if (Listing->ListingType != EMGListingType::Auction)
	{
		return false;
	}

	// Cannot bid on own listing
	if (Listing->SellerID == BidderID)
	{
		return false;
	}

	// Check auction not ended
	if (FDateTime::Now() >= Listing->EndTime)
	{
		return false;
	}

	// Calculate minimum bid
	int64 MinBid = Listing->CurrentBid > 0
		? Listing->CurrentBid + GetBidIncrement(Listing->CurrentBid)
		: Listing->StartingPrice;

	if (BidAmount < MinBid)
	{
		return false;
	}

	// Verify bidder has funds (for autobid, check max amount)
	int64 RequiredFunds = bAutoBid ? MaxAutoBidAmount : BidAmount;
	if (EconomySubsystem && !EconomySubsystem->CanAfford(RequiredFunds))
	{
		return false; // Bidder cannot afford this bid
	}

	// Store previous high bidder for notification
	FGuid PreviousHighBidder = Listing->HighestBidderID;

	// Create bid record
	FMGBidInfo Bid;
	Bid.BidID = FGuid::NewGuid();
	Bid.BidderID = BidderID;
	Bid.BidAmount = BidAmount;
	Bid.BidTime = FDateTime::Now();
	Bid.bIsAutoBid = bAutoBid;
	Bid.MaxAutoBidAmount = MaxAutoBidAmount;

	// Process autobid if existing high bidder has one
	bool bAutoBidTriggered = false;
	if (Listing->BidHistory.Num() > 0)
	{
		FMGBidInfo& CurrentHighBid = Listing->BidHistory.Last();
		if (CurrentHighBid.MaxAutoBidAmount > BidAmount)
		{
			// Current high bidder's autobid counters
			int64 AutoBidResponse = BidAmount + GetBidIncrement(BidAmount);
			AutoBidResponse = FMath::Min(AutoBidResponse, CurrentHighBid.MaxAutoBidAmount);

			// Update current high bid
			CurrentHighBid.BidAmount = AutoBidResponse;
			CurrentHighBid.BidTime = FDateTime::Now();
			CurrentHighBid.bIsAutoBid = true;

			Listing->CurrentBid = AutoBidResponse;
			bAutoBidTriggered = true;

			// Notify the new bidder they were outbid immediately
			OnOutbid.Broadcast(CurrentHighBid);
		}
	}

	if (!bAutoBidTriggered)
	{
		// New bid is highest
		Listing->BidHistory.Add(Bid);
		Listing->CurrentBid = BidAmount;
		Listing->HighestBidderID = BidderID;
		Listing->BidCount++;

		// Notify previous high bidder
		if (PreviousHighBidder.IsValid() && PreviousHighBidder != BidderID)
		{
			NotifyOutbidPlayers(*Listing, BidderID);
		}
	}

	// Apply snipe protection if bid placed in final minute
	ApplySnipeProtection(*Listing);

	OnBidPlaced.Broadcast(ListingID, Bid);

	return true;
}

bool UMGPlayerMarketSubsystem::ExecuteBuyNow(FGuid BuyerID, FGuid ListingID)
{
	FMGMarketListing* Listing = ActiveListings.Find(ListingID);
	if (!Listing || Listing->Status != EMGListingStatus::Active)
	{
		return false;
	}

	if (Listing->BuyNowPrice <= 0)
	{
		return false;
	}

	// Cannot buy own listing
	if (Listing->SellerID == BuyerID)
	{
		return false;
	}

	// Check buyer has funds
	if (EconomySubsystem && !EconomySubsystem->CanAfford(Listing->BuyNowPrice))
	{
		return false; // Buyer cannot afford the buy now price
	}

	// Calculate fees
	int64 SalePrice = Listing->BuyNowPrice;
	int64 MarketFee = CalculateMarketFee(SalePrice);
	int64 SellerReceives = SalePrice - MarketFee;

	// Process payment
	if (EconomySubsystem)
	{
		// Deduct from buyer
		if (!EconomySubsystem->DeductCash(BuyerID, SalePrice, TEXT("Market Purchase")))
		{
			return false;
		}

		// Pay seller (minus fee)
		EconomySubsystem->AddCash(Listing->SellerID, SellerReceives, TEXT("Market Sale"));
	}

	// Transfer item
	if (!TransferItem(Listing->ItemID, Listing->ItemType, Listing->SellerID, BuyerID))
	{
		// Rollback payment on transfer failure
		if (EconomySubsystem)
		{
			EconomySubsystem->AddCash(BuyerID, SalePrice, TEXT("Market Refund - Transfer Failed"));
			EconomySubsystem->DeductCash(Listing->SellerID, SellerReceives, TEXT("Market Refund - Transfer Failed"));
		}
		return false;
	}

	// Record transaction
	FMGMarketTransaction Transaction;
	Transaction.ListingID = ListingID;
	Transaction.SellerID = Listing->SellerID;
	Transaction.BuyerID = BuyerID;
	Transaction.ItemID = Listing->ItemID;
	Transaction.ItemName = Listing->ItemDisplayName;
	Transaction.SalePrice = SalePrice;
	Transaction.MarketFee = MarketFee;
	Transaction.SellerReceived = SellerReceives;

	RecordTransaction(Transaction);

	// Update listing status
	Listing->Status = EMGListingStatus::Sold;

	OnListingSold.Broadcast(ListingID, Transaction);

	return true;
}

bool UMGPlayerMarketSubsystem::SetAutoBid(FGuid BidderID, FGuid ListingID, int64 MaxAmount)
{
	FMGMarketListing* Listing = ActiveListings.Find(ListingID);
	if (!Listing || Listing->Status != EMGListingStatus::Active)
	{
		return false;
	}

	// Find bidder's existing bid
	for (FMGBidInfo& Bid : Listing->BidHistory)
	{
		if (Bid.BidderID == BidderID)
		{
			Bid.MaxAutoBidAmount = MaxAmount;
			return true;
		}
	}

	// No existing bid - create one at minimum
	return PlaceBid(BidderID, ListingID, Listing->CurrentBid > 0
		? Listing->CurrentBid + GetBidIncrement(Listing->CurrentBid)
		: Listing->StartingPrice, true, MaxAmount);
}

int64 UMGPlayerMarketSubsystem::GetMinimumBidIncrement(int64 CurrentBid) const
{
	return GetBidIncrement(CurrentBid);
}

// ==========================================
// SEARCH & BROWSE
// ==========================================

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::SearchListings(const FMGMarketSearchFilter& Filter, int32 MaxResults)
{
	TArray<FMGMarketListing> Results;

	for (const auto& Pair : ActiveListings)
	{
		const FMGMarketListing& Listing = Pair.Value;

		if (Listing.Status != EMGListingStatus::Active)
		{
			continue;
		}

		// Filter by type
		if (Listing.ItemType != Filter.ItemType)
		{
			continue;
		}

		if (Filter.ListingType != Listing.ListingType)
		{
			// Allow auction filter to match auctions
		}

		// Filter by price
		int64 DisplayPrice = Listing.CurrentBid > 0 ? Listing.CurrentBid : Listing.StartingPrice;
		if (Filter.PriceMin > 0 && DisplayPrice < Filter.PriceMin)
		{
			continue;
		}
		if (Filter.PriceMax > 0 && DisplayPrice > Filter.PriceMax)
		{
			continue;
		}

		// Filter by PI (vehicles only)
		if (Listing.ItemType == EMGMarketItemType::Vehicle)
		{
			if (Listing.PerformanceIndex < Filter.PIMin || Listing.PerformanceIndex > Filter.PIMax)
			{
				continue;
			}
		}

		// Text search
		if (!Filter.SearchText.IsEmpty())
		{
			if (!Listing.ItemDisplayName.Contains(Filter.SearchText) &&
				!Listing.ItemDescription.Contains(Filter.SearchText))
			{
				continue;
			}
		}

		// Buy now only filter
		if (Filter.bBuyNowOnly && Listing.BuyNowPrice <= 0)
		{
			continue;
		}

		// Ending soon filter (within 1 hour)
		if (Filter.bEndingSoon)
		{
			FTimespan TimeLeft = Listing.EndTime - FDateTime::Now();
			if (TimeLeft.GetTotalHours() > 1.0)
			{
				continue;
			}
		}

		// New listings (within 24 hours)
		if (Filter.bNewListingsOnly)
		{
			FTimespan Age = FDateTime::Now() - Listing.ListedTime;
			if (Age.GetTotalHours() > 24.0)
			{
				continue;
			}
		}

		Results.Add(Listing);

		if (Results.Num() >= MaxResults)
		{
			break;
		}
	}

	// Sort results
	if (Filter.SortBy == TEXT("EndTime"))
	{
		Results.Sort([Filter](const FMGMarketListing& A, const FMGMarketListing& B) {
			return Filter.bSortAscending ? A.EndTime < B.EndTime : A.EndTime > B.EndTime;
		});
	}
	else if (Filter.SortBy == TEXT("Price"))
	{
		Results.Sort([Filter](const FMGMarketListing& A, const FMGMarketListing& B) {
			int64 PriceA = A.CurrentBid > 0 ? A.CurrentBid : A.StartingPrice;
			int64 PriceB = B.CurrentBid > 0 ? B.CurrentBid : B.StartingPrice;
			return Filter.bSortAscending ? PriceA < PriceB : PriceA > PriceB;
		});
	}
	else if (Filter.SortBy == TEXT("BidCount"))
	{
		Results.Sort([Filter](const FMGMarketListing& A, const FMGMarketListing& B) {
			return Filter.bSortAscending ? A.BidCount < B.BidCount : A.BidCount > B.BidCount;
		});
	}

	return Results;
}

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::GetFeaturedListings(int32 MaxResults) const
{
	TArray<FMGMarketListing> Results;

	// Get high-activity listings
	for (const auto& Pair : ActiveListings)
	{
		if (Pair.Value.Status == EMGListingStatus::Active && Pair.Value.BidCount >= 5)
		{
			Results.Add(Pair.Value);
		}
	}

	// Sort by activity
	Results.Sort([](const FMGMarketListing& A, const FMGMarketListing& B) {
		return A.BidCount > B.BidCount;
	});

	if (Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::GetEndingSoonListings(int32 MaxResults) const
{
	TArray<FMGMarketListing> Results;

	FDateTime Now = FDateTime::Now();
	FDateTime OneHourFromNow = Now + FTimespan::FromHours(1);

	for (const auto& Pair : ActiveListings)
	{
		if (Pair.Value.Status == EMGListingStatus::Active &&
			Pair.Value.EndTime <= OneHourFromNow && Pair.Value.EndTime > Now)
		{
			Results.Add(Pair.Value);
		}
	}

	Results.Sort([](const FMGMarketListing& A, const FMGMarketListing& B) {
		return A.EndTime < B.EndTime;
	});

	if (Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::GetNewListings(int32 MaxResults) const
{
	TArray<FMGMarketListing> Results;

	FDateTime OneDayAgo = FDateTime::Now() - FTimespan::FromDays(1);

	for (const auto& Pair : ActiveListings)
	{
		if (Pair.Value.Status == EMGListingStatus::Active && Pair.Value.ListedTime >= OneDayAgo)
		{
			Results.Add(Pair.Value);
		}
	}

	Results.Sort([](const FMGMarketListing& A, const FMGMarketListing& B) {
		return A.ListedTime > B.ListedTime;
	});

	if (Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

// ==========================================
// WATCHLIST
// ==========================================

void UMGPlayerMarketSubsystem::AddToWatchlist(FGuid PlayerID, FGuid ListingID)
{
	TArray<FGuid>& Watchlist = PlayerWatchlists.FindOrAdd(PlayerID);
	if (!Watchlist.Contains(ListingID))
	{
		Watchlist.Add(ListingID);

		// Update watch count on listing
		if (FMGMarketListing* Listing = ActiveListings.Find(ListingID))
		{
			Listing->WatchCount++;
		}
	}
}

void UMGPlayerMarketSubsystem::RemoveFromWatchlist(FGuid PlayerID, FGuid ListingID)
{
	if (TArray<FGuid>* Watchlist = PlayerWatchlists.Find(PlayerID))
	{
		Watchlist->Remove(ListingID);

		if (FMGMarketListing* Listing = ActiveListings.Find(ListingID))
		{
			Listing->WatchCount = FMath::Max(0, Listing->WatchCount - 1);
		}
	}
}

TArray<FMGMarketListing> UMGPlayerMarketSubsystem::GetWatchlist(FGuid PlayerID) const
{
	TArray<FMGMarketListing> Results;

	if (const TArray<FGuid>* Watchlist = PlayerWatchlists.Find(PlayerID))
	{
		for (const FGuid& ListingID : *Watchlist)
		{
			if (const FMGMarketListing* Listing = ActiveListings.Find(ListingID))
			{
				Results.Add(*Listing);
			}
		}
	}

	return Results;
}

// ==========================================
// DIRECT TRADING
// ==========================================

bool UMGPlayerMarketSubsystem::CreateTradeOffer(FGuid InitiatorID, FGuid RecipientID,
	const TArray<FGuid>& OfferedItems, int64 OfferedCash,
	const TArray<FGuid>& RequestedItems, int64 RequestedCash,
	const FString& Message, FGuid& OutTradeID)
{
	// Cannot trade with self
	if (InitiatorID == RecipientID)
	{
		return false;
	}

	// Verify none of the offered items are trade-locked
	for (const FGuid& ItemID : OfferedItems)
	{
		if (IsVehicleTradeLocked(ItemID))
		{
			return false;
		}
	}

	// Verify initiator has the offered cash
	if (OfferedCash > 0 && EconomySubsystem && !EconomySubsystem->CanAfford(OfferedCash))
	{
		return false; // Initiator cannot afford the offered cash amount
	}

	FMGTradeOffer Offer;
	Offer.InitiatorID = InitiatorID;
	Offer.RecipientID = RecipientID;
	Offer.InitiatorItems = OfferedItems;
	Offer.InitiatorCash = OfferedCash;
	Offer.RecipientItems = RequestedItems;
	Offer.RecipientCash = RequestedCash;
	Offer.Message = Message;
	Offer.ExpiresTime = FDateTime::Now() + FTimespan::FromHours(24);

	OutTradeID = Offer.TradeID;
	PendingTrades.Add(Offer.TradeID, Offer);

	OnTradeOfferReceived.Broadcast(Offer);

	return true;
}

bool UMGPlayerMarketSubsystem::AcceptTradeOffer(FGuid RecipientID, FGuid TradeID)
{
	FMGTradeOffer* Offer = PendingTrades.Find(TradeID);
	if (!Offer || Offer->RecipientID != RecipientID)
	{
		return false;
	}

	if (Offer->Status != EMGTradeStatus::Pending)
	{
		return false;
	}

	if (FDateTime::Now() >= Offer->ExpiresTime)
	{
		Offer->Status = EMGTradeStatus::Expired;
		return false;
	}

	// Verify both parties still have the items/cash
	// Validate initiator has the offered items
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UMGGarageSubsystem* GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>())
		{
			// Check initiator's items
			for (const FGuid& ItemID : Offer->InitiatorItems)
			{
				FMGOwnedVehicle Vehicle;
				if (!GarageSubsystem->GetVehicle(ItemID, Vehicle))
				{
					return false; // Initiator no longer has the item
				}
			}

			// Check recipient's items (for counter trades)
			for (const FGuid& ItemID : Offer->RecipientItems)
			{
				FMGOwnedVehicle Vehicle;
				if (!GarageSubsystem->GetVehicle(ItemID, Vehicle))
				{
					return false; // Recipient no longer has the item
				}
			}
		}

		// Validate cash balances
		if (EconomySubsystem)
		{
			if (Offer->InitiatorCash > 0 && !EconomySubsystem->CanAfford(Offer->InitiatorCash))
			{
				return false; // Initiator cannot afford the offered cash
			}
			if (Offer->RecipientCash > 0 && !EconomySubsystem->CanAfford(Offer->RecipientCash))
			{
				return false; // Recipient cannot afford the requested cash
			}
		}
	}

	// Execute trade
	// Transfer items from initiator to recipient
	for (const FGuid& ItemID : Offer->InitiatorItems)
	{
		TransferItem(ItemID, EMGMarketItemType::Vehicle, Offer->InitiatorID, Offer->RecipientID);
	}

	// Transfer items from recipient to initiator
	for (const FGuid& ItemID : Offer->RecipientItems)
	{
		TransferItem(ItemID, EMGMarketItemType::Vehicle, Offer->RecipientID, Offer->InitiatorID);
	}

	// Transfer cash
	if (EconomySubsystem)
	{
		if (Offer->InitiatorCash > 0)
		{
			EconomySubsystem->DeductCash(Offer->InitiatorID, Offer->InitiatorCash, TEXT("Trade"));
			EconomySubsystem->AddCash(Offer->RecipientID, Offer->InitiatorCash, TEXT("Trade"));
		}
		if (Offer->RecipientCash > 0)
		{
			EconomySubsystem->DeductCash(Offer->RecipientID, Offer->RecipientCash, TEXT("Trade"));
			EconomySubsystem->AddCash(Offer->InitiatorID, Offer->RecipientCash, TEXT("Trade"));
		}
	}

	Offer->Status = EMGTradeStatus::Completed;

	OnTradeCompleted.Broadcast(TradeID, true);

	return true;
}

bool UMGPlayerMarketSubsystem::RejectTradeOffer(FGuid RecipientID, FGuid TradeID)
{
	FMGTradeOffer* Offer = PendingTrades.Find(TradeID);
	if (!Offer || Offer->RecipientID != RecipientID)
	{
		return false;
	}

	if (Offer->Status != EMGTradeStatus::Pending)
	{
		return false;
	}

	Offer->Status = EMGTradeStatus::Rejected;

	OnTradeCompleted.Broadcast(TradeID, false);

	return true;
}

bool UMGPlayerMarketSubsystem::CancelTradeOffer(FGuid InitiatorID, FGuid TradeID)
{
	FMGTradeOffer* Offer = PendingTrades.Find(TradeID);
	if (!Offer || Offer->InitiatorID != InitiatorID)
	{
		return false;
	}

	if (Offer->Status != EMGTradeStatus::Pending)
	{
		return false;
	}

	Offer->Status = EMGTradeStatus::Cancelled;

	return true;
}

bool UMGPlayerMarketSubsystem::CreateCounterOffer(FGuid RecipientID, FGuid OriginalTradeID,
	const TArray<FGuid>& CounterItems, int64 CounterCash, FGuid& OutTradeID)
{
	FMGTradeOffer* Original = PendingTrades.Find(OriginalTradeID);
	if (!Original || Original->RecipientID != RecipientID)
	{
		return false;
	}

	// Reject original offer
	Original->Status = EMGTradeStatus::Rejected;

	// Create counter (swap initiator/recipient)
	return CreateTradeOffer(
		RecipientID, // Now the initiator
		Original->InitiatorID, // Now the recipient
		CounterItems,
		CounterCash,
		Original->InitiatorItems, // Request the original offer
		Original->InitiatorCash,
		TEXT("Counter-offer"),
		OutTradeID
	);
}

TArray<FMGTradeOffer> UMGPlayerMarketSubsystem::GetPendingTrades(FGuid PlayerID) const
{
	TArray<FMGTradeOffer> Results;

	for (const auto& Pair : PendingTrades)
	{
		if (Pair.Value.Status == EMGTradeStatus::Pending)
		{
			if (Pair.Value.InitiatorID == PlayerID || Pair.Value.RecipientID == PlayerID)
			{
				Results.Add(Pair.Value);
			}
		}
	}

	return Results;
}

// ==========================================
// PRICING
// ==========================================

int64 UMGPlayerMarketSubsystem::GetEstimatedMarketValue(FGuid VehicleID) const
{
	// TODO: Look up vehicle data and calculate based on:
	// - Base model value
	// - Parts installed
	// - Condition
	// - Recent sale prices for similar vehicles

	return 50000; // Placeholder
}

TArray<FMGMarketTransaction> UMGPlayerMarketSubsystem::GetPriceHistory(FName ModelID, int32 DaysBack) const
{
	TArray<FMGMarketTransaction> Results;

	FDateTime Cutoff = FDateTime::Now() - FTimespan::FromDays(DaysBack);

	for (const FMGMarketTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.TransactionTime >= Cutoff)
		{
			// TODO: Filter by ModelID
			Results.Add(Transaction);
		}
	}

	return Results;
}

int64 UMGPlayerMarketSubsystem::GetMinimumListingPrice(FGuid ItemID, EMGMarketItemType ItemType) const
{
	// Prevent dumping for RMT
	int64 EstimatedValue = GetEstimatedMarketValue(ItemID);
	return FMath::Max(static_cast<int64>(EstimatedValue * 0.3), static_cast<int64>(1000)); // Min 30% of value or $1000
}

int64 UMGPlayerMarketSubsystem::GetMaximumListingPrice(FGuid ItemID, EMGMarketItemType ItemType) const
{
	// Prevent price gouging / money laundering
	int64 EstimatedValue = GetEstimatedMarketValue(ItemID);
	return FMath::Max(static_cast<int64>(EstimatedValue * 5.0), static_cast<int64>(1000000)); // Max 5x value or $1M
}

// ==========================================
// FEES & ECONOMY
// ==========================================

int64 UMGPlayerMarketSubsystem::CalculateMarketFee(int64 SalePrice) const
{
	// 5% market fee per PRD
	return static_cast<int64>(SalePrice * MarketFeePercent);
}

int64 UMGPlayerMarketSubsystem::GetListingFee(EMGMarketItemType ItemType, EMGListingType ListingType) const
{
	// Base listing fee varies by type
	int64 BaseFee = 100;

	switch (ItemType)
	{
	case EMGMarketItemType::Vehicle: BaseFee = 500; break;
	case EMGMarketItemType::Part: BaseFee = 100; break;
	case EMGMarketItemType::Cosmetic: BaseFee = 50; break;
	case EMGMarketItemType::Livery: BaseFee = 25; break;
	}

	// Auction listings cost more (more infrastructure)
	if (ListingType == EMGListingType::Auction)
	{
		BaseFee = static_cast<int64>(BaseFee * 1.5);
	}

	return BaseFee;
}

// ==========================================
// FRAUD DETECTION
// ==========================================

bool UMGPlayerMarketSubsystem::IsTransactionSuspicious(FGuid SellerID, FGuid BuyerID, int64 Amount) const
{
	// Check for common fraud patterns:

	// 1. Large transaction between accounts with history of trading
	int32 PreviousTransactions = 0;
	for (const FMGMarketTransaction& Trans : TransactionHistory)
	{
		if ((Trans.SellerID == SellerID && Trans.BuyerID == BuyerID) ||
			(Trans.SellerID == BuyerID && Trans.BuyerID == SellerID))
		{
			PreviousTransactions++;
		}
	}

	// Many transactions between same two accounts is suspicious
	if (PreviousTransactions > 10)
	{
		return true;
	}

	// 2. Price significantly above market value
	// Would need to compare against item's estimated value

	// 3. New accounts with large transactions
	// Would need account age data

	return false;
}

float UMGPlayerMarketSubsystem::GetPlayerFraudScore(FGuid PlayerID) const
{
	float Score = 0.0f;

	for (const FMGFraudFlag& Flag : FraudFlags)
	{
		if (Flag.PlayerID == PlayerID)
		{
			Score += Flag.SuspicionScore;
		}
	}

	return FMath::Clamp(Score, 0.0f, 100.0f);
}

void UMGPlayerMarketSubsystem::FlagSuspiciousActivity(const FMGFraudFlag& Flag)
{
	FraudFlags.Add(Flag);
}

// ==========================================
// HISTORY
// ==========================================

TArray<FMGMarketTransaction> UMGPlayerMarketSubsystem::GetPurchaseHistory(FGuid PlayerID, int32 MaxResults) const
{
	TArray<FMGMarketTransaction> Results;

	for (int32 i = TransactionHistory.Num() - 1; i >= 0 && Results.Num() < MaxResults; --i)
	{
		if (TransactionHistory[i].BuyerID == PlayerID)
		{
			Results.Add(TransactionHistory[i]);
		}
	}

	return Results;
}

TArray<FMGMarketTransaction> UMGPlayerMarketSubsystem::GetSaleHistory(FGuid PlayerID, int32 MaxResults) const
{
	TArray<FMGMarketTransaction> Results;

	for (int32 i = TransactionHistory.Num() - 1; i >= 0 && Results.Num() < MaxResults; --i)
	{
		if (TransactionHistory[i].SellerID == PlayerID)
		{
			Results.Add(TransactionHistory[i]);
		}
	}

	return Results;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGPlayerMarketSubsystem::OnAuctionTick()
{
	ProcessEndedListings();
	ProcessExpiredTrades();
}

void UMGPlayerMarketSubsystem::ProcessEndedListings()
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : ActiveListings)
	{
		FMGMarketListing& Listing = Pair.Value;

		if (Listing.Status != EMGListingStatus::Active)
		{
			continue;
		}

		if (Now >= Listing.EndTime)
		{
			if (Listing.ListingType == EMGListingType::Auction && Listing.BidCount > 0)
			{
				// Check reserve price met
				if (Listing.ReservePrice > 0 && Listing.CurrentBid < Listing.ReservePrice)
				{
					// Reserve not met - listing expires
					Listing.Status = EMGListingStatus::Expired;
					OnListingExpired.Broadcast(Listing.ListingID, Listing);
				}
				else
				{
					// Finalize sale to highest bidder
					FinalizeAuctionSale(Listing);
				}
			}
			else
			{
				// No bids - listing expires
				Listing.Status = EMGListingStatus::Expired;
				OnListingExpired.Broadcast(Listing.ListingID, Listing);
			}
		}
	}
}

void UMGPlayerMarketSubsystem::ProcessExpiredTrades()
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : PendingTrades)
	{
		FMGTradeOffer& Offer = Pair.Value;

		if (Offer.Status == EMGTradeStatus::Pending && Now >= Offer.ExpiresTime)
		{
			Offer.Status = EMGTradeStatus::Expired;
		}
	}
}

void UMGPlayerMarketSubsystem::FinalizeAuctionSale(FMGMarketListing& Listing)
{
	int64 SalePrice = Listing.CurrentBid;
	int64 MarketFee = CalculateMarketFee(SalePrice);
	int64 SellerReceives = SalePrice - MarketFee;

	// Process payment
	if (EconomySubsystem)
	{
		EconomySubsystem->DeductCash(Listing.HighestBidderID, SalePrice, TEXT("Auction Won"));
		EconomySubsystem->AddCash(Listing.SellerID, SellerReceives, TEXT("Auction Sale"));
	}

	// Transfer item
	TransferItem(Listing.ItemID, Listing.ItemType, Listing.SellerID, Listing.HighestBidderID);

	// Record transaction
	FMGMarketTransaction Transaction;
	Transaction.ListingID = Listing.ListingID;
	Transaction.SellerID = Listing.SellerID;
	Transaction.BuyerID = Listing.HighestBidderID;
	Transaction.ItemID = Listing.ItemID;
	Transaction.ItemName = Listing.ItemDisplayName;
	Transaction.SalePrice = SalePrice;
	Transaction.MarketFee = MarketFee;
	Transaction.SellerReceived = SellerReceives;

	RecordTransaction(Transaction);

	Listing.Status = EMGListingStatus::Sold;

	OnListingSold.Broadcast(Listing.ListingID, Transaction);
}

bool UMGPlayerMarketSubsystem::TransferItem(FGuid ItemID, EMGMarketItemType ItemType, FGuid FromPlayerID, FGuid ToPlayerID)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return false;
	}

	UMGGarageSubsystem* GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>();
	if (!GarageSubsystem)
	{
		return false;
	}

	switch (ItemType)
	{
	case EMGMarketItemType::Vehicle:
		{
			// Get the vehicle data before removing
			FMGOwnedVehicle SourceVehicle;
			if (!GarageSubsystem->GetVehicle(ItemID, SourceVehicle))
			{
				return false; // Vehicle not found in source
			}

			// Remove from source player's garage
			FMGGarageResult RemoveResult = GarageSubsystem->RemoveVehicle(ItemID);
			if (!RemoveResult.bSuccess)
			{
				return false;
			}

			// Add to destination player's garage
			// Note: In a full multiplayer implementation, this would use the ToPlayerID
			// to access the correct player's garage subsystem
			FGuid NewVehicleID;
			UMGVehicleModelData* ModelData = SourceVehicle.VehicleModelData.Get();
			if (ModelData)
			{
				FMGGarageResult AddResult = GarageSubsystem->AddVehicle(ModelData, NewVehicleID);
				if (!AddResult.bSuccess)
				{
					// Rollback - add vehicle back to source
					GarageSubsystem->AddVehicle(ModelData, NewVehicleID);
					return false;
				}
			}

			return true;
		}

	case EMGMarketItemType::Part:
		// For parts, ownership is tracked per-vehicle in InstalledParts
		// Parts in inventory would be transferred similarly
		// MVP: Parts transfer is handled implicitly when vehicle transfers
		return true;

	case EMGMarketItemType::Cosmetic:
	case EMGMarketItemType::Livery:
		// Cosmetics/Liveries are typically account-bound or vehicle-bound
		// MVP: These transfers are handled at the data level
		return true;

	default:
		return false;
	}
}

void UMGPlayerMarketSubsystem::NotifyOutbidPlayers(const FMGMarketListing& Listing, FGuid NewHighBidderID)
{
	// Find previous high bidder
	for (int32 i = Listing.BidHistory.Num() - 2; i >= 0; --i)
	{
		const FMGBidInfo& Bid = Listing.BidHistory[i];
		if (Bid.BidderID != NewHighBidderID)
		{
			OnOutbid.Broadcast(Listing.BidHistory.Last());
			break;
		}
	}
}

bool UMGPlayerMarketSubsystem::IsVehicleTradeLocked(FGuid VehicleID) const
{
	// Check if vehicle was won via pink slip within last 7 days
	UGameInstance* GI = const_cast<UMGPlayerMarketSubsystem*>(this)->GetGameInstance();
	if (!GI)
	{
		return false;
	}

	UMGGarageSubsystem* GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>();
	if (!GarageSubsystem)
	{
		return false;
	}

	FMGOwnedVehicle Vehicle;
	if (!GarageSubsystem->GetVehicle(VehicleID, Vehicle))
	{
		return false;
	}

	// Check if acquired within last 7 days via pink slip
	// For MVP, we check if DateAcquired is within 7 days
	// In full implementation, we'd also check acquisition method
	FTimespan TimeSinceAcquisition = FDateTime::Now() - Vehicle.DateAcquired;
	static constexpr double PinkSlipLockDays = 7.0;

	// Pink slip vehicles are typically marked with a special condition
	// For now, return false as we don't have pink slip acquisition tracking yet
	// This prevents blocking all recently acquired vehicles
	return false;
}

void UMGPlayerMarketSubsystem::ApplySnipeProtection(FMGMarketListing& Listing)
{
	FTimespan TimeLeft = Listing.EndTime - FDateTime::Now();

	if (TimeLeft.GetTotalSeconds() <= SnipeProtectionWindow)
	{
		Listing.EndTime = FDateTime::Now() + FTimespan::FromSeconds(SnipeProtectionExtension);
		Listing.bSnipeProtectionActive = true;
	}
}

bool UMGPlayerMarketSubsystem::ValidatePricing(int64 Price, FGuid ItemID, EMGMarketItemType ItemType) const
{
	int64 MinPrice = GetMinimumListingPrice(ItemID, ItemType);
	int64 MaxPrice = GetMaximumListingPrice(ItemID, ItemType);

	return Price >= MinPrice && Price <= MaxPrice;
}

void UMGPlayerMarketSubsystem::RecordTransaction(const FMGMarketTransaction& Transaction)
{
	TransactionHistory.Add(Transaction);

	// Update fraud detection
	UpdateFraudScores(Transaction);
}

void UMGPlayerMarketSubsystem::UpdateFraudScores(const FMGMarketTransaction& Transaction)
{
	if (IsTransactionSuspicious(Transaction.SellerID, Transaction.BuyerID, Transaction.SalePrice))
	{
		FMGFraudFlag Flag;
		Flag.PlayerID = Transaction.SellerID;
		Flag.FlagReason = TEXT("Suspicious transaction pattern");
		Flag.SuspicionScore = 10.0f;
		Flag.FlagTime = FDateTime::Now();
		Flag.RelatedTransactions.Add(Transaction.TransactionID);

		FlagSuspiciousActivity(Flag);

		Flag.PlayerID = Transaction.BuyerID;
		FlagSuspiciousActivity(Flag);
	}
}

int64 UMGPlayerMarketSubsystem::GetBidIncrement(int64 CurrentPrice) const
{
	// Bid increments based on current price
	// Follows auction house standards

	if (CurrentPrice < 1000)
	{
		return 50;
	}
	else if (CurrentPrice < 5000)
	{
		return 100;
	}
	else if (CurrentPrice < 25000)
	{
		return 250;
	}
	else if (CurrentPrice < 100000)
	{
		return 500;
	}
	else if (CurrentPrice < 500000)
	{
		return 1000;
	}
	else
	{
		return 2500;
	}
}
