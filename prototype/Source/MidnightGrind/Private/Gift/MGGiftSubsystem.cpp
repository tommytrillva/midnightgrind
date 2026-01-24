// Copyright Midnight Grind. All Rights Reserved.

#include "Gift/MGGiftSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGGiftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default settings
	Settings.bAcceptGiftsFromFriends = true;
	Settings.bAcceptGiftsFromAnyone = false;
	Settings.bAcceptAnonymousGifts = true;
	Settings.bNotifyOnGiftReceived = true;
	Settings.bAutoClaimGifts = false;
	Settings.MaxPendingGifts = 50;

	LoadGiftData();

	// Start gift tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GiftTickHandle,
			this,
			&UMGGiftSubsystem::OnGiftTick,
			60.0f, // Check every minute
			true
		);
	}
}

void UMGGiftSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GiftTickHandle);
	}

	SaveGiftData();
	Super::Deinitialize();
}

bool UMGGiftSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Sending Gifts
// ============================================================================

FGuid UMGGiftSubsystem::SendGift(FName RecipientID, const TArray<FMGGiftItem>& Items, const FText& Message, EMGGiftWrapStyle WrapStyle, bool bAnonymous)
{
	if (!CanSendGift(RecipientID) || Items.Num() == 0)
	{
		return FGuid();
	}

	// Verify all items are giftable
	for (const FMGGiftItem& Item : Items)
	{
		if (!CanGiftItem(Item.ItemID))
		{
			return FGuid();
		}
	}

	FMGGift NewGift;
	NewGift.GiftID = FGuid::NewGuid();
	NewGift.SenderID = LocalPlayerID;
	NewGift.RecipientID = RecipientID;
	NewGift.Items = Items;
	NewGift.PersonalMessage = Message;
	NewGift.WrapStyle = WrapStyle;
	NewGift.bIsAnonymous = bAnonymous;
	NewGift.Status = EMGGiftStatus::Sent;
	NewGift.SentAt = FDateTime::UtcNow();
	NewGift.ExpiresAt = NewGift.SentAt + FTimespan::FromDays(GiftExpirationDays);
	NewGift.TotalValue = CalculateGiftValue(Items);

	SentGifts.Add(NewGift);

	// Update stats
	UpdateStats(NewGift, true);
	AddToHistory(NewGift, true);

	OnGiftSent.Broadcast(NewGift);
	SaveGiftData();

	return NewGift.GiftID;
}

FGuid UMGGiftSubsystem::SendCurrencyGift(FName RecipientID, int32 Amount, const FText& Message, bool bAnonymous)
{
	if (Amount <= 0)
	{
		return FGuid();
	}

	FMGGiftItem CurrencyItem;
	CurrencyItem.ItemID = TEXT("Currency_Standard");
	CurrencyItem.DisplayName = FText::FromString(FString::Printf(TEXT("%d Credits"), Amount));
	CurrencyItem.GiftType = EMGGiftType::Currency;
	CurrencyItem.CurrencyValue = Amount;
	CurrencyItem.Quantity = 1;

	TArray<FMGGiftItem> Items;
	Items.Add(CurrencyItem);

	return SendGift(RecipientID, Items, Message, EMGGiftWrapStyle::Default, bAnonymous);
}

FGuid UMGGiftSubsystem::SendBundleGift(FName RecipientID, FName BundleID, const FText& Message, bool bAnonymous)
{
	const FMGGiftBundle* Bundle = AvailableBundles.Find(BundleID);
	if (!Bundle)
	{
		return FGuid();
	}

	return SendGift(RecipientID, Bundle->Contents, Message, Bundle->DefaultWrap, bAnonymous);
}

bool UMGGiftSubsystem::CanSendGift(FName RecipientID) const
{
	if (RecipientID.IsNone())
	{
		return false;
	}

	// Can't send to yourself
	if (RecipientID == LocalPlayerID)
	{
		return false;
	}

	// Would check friendship status, blocked status, etc. from other subsystems
	return true;
}

bool UMGGiftSubsystem::CanGiftItem(FName ItemID) const
{
	if (const FMGGiftItem* Item = GiftableItems.Find(ItemID))
	{
		return Item->bIsGiftable;
	}

	return false;
}

bool UMGGiftSubsystem::CancelGift(FGuid GiftID)
{
	for (int32 i = 0; i < SentGifts.Num(); ++i)
	{
		if (SentGifts[i].GiftID == GiftID && SentGifts[i].Status == EMGGiftStatus::Sent)
		{
			SentGifts[i].Status = EMGGiftStatus::Cancelled;
			OnGiftStatusChanged.Broadcast(GiftID, EMGGiftStatus::Cancelled);
			SaveGiftData();
			return true;
		}
	}

	return false;
}

// ============================================================================
// Receiving Gifts
// ============================================================================

TArray<FMGGift> UMGGiftSubsystem::GetPendingGifts() const
{
	TArray<FMGGift> Pending;

	for (const FMGGift& Gift : PendingReceivedGifts)
	{
		if (Gift.Status == EMGGiftStatus::Delivered)
		{
			Pending.Add(Gift);
		}
	}

	return Pending;
}

int32 UMGGiftSubsystem::GetPendingGiftCount() const
{
	int32 Count = 0;

	for (const FMGGift& Gift : PendingReceivedGifts)
	{
		if (Gift.Status == EMGGiftStatus::Delivered)
		{
			Count++;
		}
	}

	return Count;
}

FMGGift UMGGiftSubsystem::GetGift(FGuid GiftID) const
{
	for (const FMGGift& Gift : PendingReceivedGifts)
	{
		if (Gift.GiftID == GiftID)
		{
			return Gift;
		}
	}

	for (const FMGGift& Gift : SentGifts)
	{
		if (Gift.GiftID == GiftID)
		{
			return Gift;
		}
	}

	return FMGGift();
}

TArray<FMGGiftItem> UMGGiftSubsystem::ClaimGift(FGuid GiftID)
{
	for (int32 i = 0; i < PendingReceivedGifts.Num(); ++i)
	{
		if (PendingReceivedGifts[i].GiftID == GiftID && PendingReceivedGifts[i].Status == EMGGiftStatus::Delivered)
		{
			FMGGift& Gift = PendingReceivedGifts[i];
			Gift.Status = EMGGiftStatus::Claimed;
			Gift.ClaimedAt = FDateTime::UtcNow();

			TArray<FMGGiftItem> ClaimedItems = Gift.Items;

			// Move to history
			AddToHistory(Gift, false);
			UpdateStats(Gift, false);

			OnGiftClaimed.Broadcast(GiftID, ClaimedItems);
			OnGiftStatusChanged.Broadcast(GiftID, EMGGiftStatus::Claimed);

			// Remove from pending
			PendingReceivedGifts.RemoveAt(i);
			SaveGiftData();

			return ClaimedItems;
		}
	}

	return TArray<FMGGiftItem>();
}

TArray<FMGGiftItem> UMGGiftSubsystem::ClaimAllGifts()
{
	TArray<FMGGiftItem> AllClaimedItems;
	TArray<FGuid> GiftsToClaim;

	for (const FMGGift& Gift : PendingReceivedGifts)
	{
		if (Gift.Status == EMGGiftStatus::Delivered)
		{
			GiftsToClaim.Add(Gift.GiftID);
		}
	}

	for (const FGuid& GiftID : GiftsToClaim)
	{
		TArray<FMGGiftItem> Items = ClaimGift(GiftID);
		AllClaimedItems.Append(Items);
	}

	return AllClaimedItems;
}

bool UMGGiftSubsystem::ReturnGift(FGuid GiftID)
{
	for (int32 i = 0; i < PendingReceivedGifts.Num(); ++i)
	{
		if (PendingReceivedGifts[i].GiftID == GiftID && PendingReceivedGifts[i].Status == EMGGiftStatus::Delivered)
		{
			FMGGift Gift = PendingReceivedGifts[i];
			Gift.Status = EMGGiftStatus::Returned;

			OnGiftReturned.Broadcast(Gift);
			OnGiftStatusChanged.Broadcast(GiftID, EMGGiftStatus::Returned);

			PendingReceivedGifts.RemoveAt(i);
			SaveGiftData();

			return true;
		}
	}

	return false;
}

bool UMGGiftSubsystem::CanAcceptGiftFrom(FName SenderID) const
{
	if (SenderID.IsNone())
	{
		return Settings.bAcceptAnonymousGifts;
	}

	// Check if we accept from anyone
	if (Settings.bAcceptGiftsFromAnyone)
	{
		return true;
	}

	// Check if we accept from friends (would check friendship from social subsystem)
	if (Settings.bAcceptGiftsFromFriends)
	{
		// Placeholder - would check friendship status
		return true;
	}

	return false;
}

// ============================================================================
// Gift Bundles
// ============================================================================

TArray<FMGGiftBundle> UMGGiftSubsystem::GetAvailableBundles() const
{
	TArray<FMGGiftBundle> Bundles;
	AvailableBundles.GenerateValueArray(Bundles);

	// Filter out expired bundles
	FDateTime Now = FDateTime::UtcNow();
	Bundles.RemoveAll([&Now](const FMGGiftBundle& Bundle)
	{
		return Bundle.bLimitedTime && Bundle.AvailableUntil < Now;
	});

	return Bundles;
}

FMGGiftBundle UMGGiftSubsystem::GetBundle(FName BundleID) const
{
	if (const FMGGiftBundle* Bundle = AvailableBundles.Find(BundleID))
	{
		return *Bundle;
	}

	return FMGGiftBundle();
}

void UMGGiftSubsystem::RegisterBundle(const FMGGiftBundle& Bundle)
{
	AvailableBundles.Add(Bundle.BundleID, Bundle);
}

int32 UMGGiftSubsystem::GetBundlePrice(FName BundleID) const
{
	if (const FMGGiftBundle* Bundle = AvailableBundles.Find(BundleID))
	{
		if (Bundle->DiscountPercent > 0)
		{
			return FMath::RoundToInt(Bundle->Price * (1.0f - Bundle->DiscountPercent / 100.0f));
		}
		return Bundle->Price;
	}

	return 0;
}

// ============================================================================
// Giftable Items
// ============================================================================

TArray<FMGGiftItem> UMGGiftSubsystem::GetGiftableItems() const
{
	TArray<FMGGiftItem> Items;

	for (const auto& ItemPair : GiftableItems)
	{
		if (ItemPair.Value.bIsGiftable)
		{
			Items.Add(ItemPair.Value);
		}
	}

	return Items;
}

void UMGGiftSubsystem::RegisterGiftableItem(const FMGGiftItem& Item)
{
	GiftableItems.Add(Item.ItemID, Item);
}

FMGGiftItem UMGGiftSubsystem::GetGiftableItem(FName ItemID) const
{
	if (const FMGGiftItem* Item = GiftableItems.Find(ItemID))
	{
		return *Item;
	}

	return FMGGiftItem();
}

// ============================================================================
// History
// ============================================================================

TArray<FMGGiftHistory> UMGGiftSubsystem::GetSentHistory(int32 MaxEntries) const
{
	TArray<FMGGiftHistory> SentHistory;

	for (const FMGGiftHistory& Entry : GiftHistory)
	{
		if (Entry.bWasSent)
		{
			SentHistory.Add(Entry);
		}
	}

	// Sort by date descending
	SentHistory.Sort([](const FMGGiftHistory& A, const FMGGiftHistory& B)
	{
		return A.TransactionDate > B.TransactionDate;
	});

	if (MaxEntries > 0 && SentHistory.Num() > MaxEntries)
	{
		SentHistory.SetNum(MaxEntries);
	}

	return SentHistory;
}

TArray<FMGGiftHistory> UMGGiftSubsystem::GetReceivedHistory(int32 MaxEntries) const
{
	TArray<FMGGiftHistory> ReceivedHistory;

	for (const FMGGiftHistory& Entry : GiftHistory)
	{
		if (!Entry.bWasSent)
		{
			ReceivedHistory.Add(Entry);
		}
	}

	// Sort by date descending
	ReceivedHistory.Sort([](const FMGGiftHistory& A, const FMGGiftHistory& B)
	{
		return A.TransactionDate > B.TransactionDate;
	});

	if (MaxEntries > 0 && ReceivedHistory.Num() > MaxEntries)
	{
		ReceivedHistory.SetNum(MaxEntries);
	}

	return ReceivedHistory;
}

TArray<FMGGiftHistory> UMGGiftSubsystem::GetHistoryWithPlayer(FName PlayerID) const
{
	TArray<FMGGiftHistory> PlayerHistory;

	for (const FMGGiftHistory& Entry : GiftHistory)
	{
		if (Entry.OtherPlayerID == PlayerID)
		{
			PlayerHistory.Add(Entry);
		}
	}

	return PlayerHistory;
}

// ============================================================================
// Settings
// ============================================================================

void UMGGiftSubsystem::SetGiftSettings(const FMGGiftSettings& NewSettings)
{
	Settings = NewSettings;
	SaveGiftData();
}

// ============================================================================
// Stats
// ============================================================================

int32 UMGGiftSubsystem::GetTotalGiftsWithPlayer(FName PlayerID) const
{
	int32 Count = 0;

	for (const FMGGiftHistory& Entry : GiftHistory)
	{
		if (Entry.OtherPlayerID == PlayerID)
		{
			Count++;
		}
	}

	return Count;
}

// ============================================================================
// Network
// ============================================================================

void UMGGiftSubsystem::ReceiveGift(const FMGGift& Gift)
{
	// Check if we can accept gifts from this sender
	if (!CanAcceptGiftFrom(Gift.bIsAnonymous ? NAME_None : Gift.SenderID))
	{
		// Automatically return the gift
		FMGGift ReturnedGift = Gift;
		ReturnedGift.Status = EMGGiftStatus::Returned;
		OnGiftReturned.Broadcast(ReturnedGift);
		return;
	}

	// Check pending limit
	if (GetPendingGiftCount() >= Settings.MaxPendingGifts)
	{
		// Would need to handle this - maybe return oldest gift
		return;
	}

	FMGGift ReceivedGift = Gift;
	ReceivedGift.Status = EMGGiftStatus::Delivered;

	PendingReceivedGifts.Add(ReceivedGift);

	OnGiftReceived.Broadcast(ReceivedGift);

	// Auto-claim if enabled
	if (Settings.bAutoClaimGifts)
	{
		ClaimGift(ReceivedGift.GiftID);
	}

	SaveGiftData();
}

void UMGGiftSubsystem::UpdateGiftStatus(FGuid GiftID, EMGGiftStatus NewStatus)
{
	// Update sent gifts
	for (FMGGift& Gift : SentGifts)
	{
		if (Gift.GiftID == GiftID)
		{
			Gift.Status = NewStatus;
			OnGiftStatusChanged.Broadcast(GiftID, NewStatus);
			SaveGiftData();
			return;
		}
	}

	// Update received gifts
	for (FMGGift& Gift : PendingReceivedGifts)
	{
		if (Gift.GiftID == GiftID)
		{
			Gift.Status = NewStatus;
			OnGiftStatusChanged.Broadcast(GiftID, NewStatus);
			SaveGiftData();
			return;
		}
	}
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGGiftSubsystem::OnGiftTick()
{
	CheckExpiredGifts();
}

void UMGGiftSubsystem::CheckExpiredGifts()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check pending received gifts
	for (int32 i = PendingReceivedGifts.Num() - 1; i >= 0; --i)
	{
		if (PendingReceivedGifts[i].ExpiresAt < Now && PendingReceivedGifts[i].Status == EMGGiftStatus::Delivered)
		{
			FGuid ExpiredGiftID = PendingReceivedGifts[i].GiftID;
			PendingReceivedGifts[i].Status = EMGGiftStatus::Expired;
			OnGiftExpired.Broadcast(ExpiredGiftID);
			PendingReceivedGifts.RemoveAt(i);
		}
	}

	// Check sent gifts
	for (int32 i = SentGifts.Num() - 1; i >= 0; --i)
	{
		if (SentGifts[i].ExpiresAt < Now && SentGifts[i].Status == EMGGiftStatus::Sent)
		{
			SentGifts[i].Status = EMGGiftStatus::Expired;
			OnGiftExpired.Broadcast(SentGifts[i].GiftID);
		}
	}
}

void UMGGiftSubsystem::AddToHistory(const FMGGift& Gift, bool bWasSent)
{
	FMGGiftHistory HistoryEntry;
	HistoryEntry.GiftID = Gift.GiftID;
	HistoryEntry.bWasSent = bWasSent;
	HistoryEntry.OtherPlayerID = bWasSent ? Gift.RecipientID : Gift.SenderID;
	HistoryEntry.OtherPlayerName = bWasSent ? Gift.RecipientName : Gift.SenderName;
	HistoryEntry.TotalValue = Gift.TotalValue;
	HistoryEntry.TransactionDate = bWasSent ? Gift.SentAt : FDateTime::UtcNow();
	HistoryEntry.FinalStatus = Gift.Status;

	GiftHistory.Add(HistoryEntry);

	// Limit history size
	const int32 MaxHistoryEntries = 200;
	while (GiftHistory.Num() > MaxHistoryEntries)
	{
		GiftHistory.RemoveAt(0);
	}
}

void UMGGiftSubsystem::UpdateStats(const FMGGift& Gift, bool bWasSent)
{
	if (bWasSent)
	{
		Stats.TotalGiftsSent++;
		Stats.TotalValueSent += Gift.TotalValue;

		// Track unique recipients
		TSet<FName> UniqueRecipients;
		for (const FMGGiftHistory& Entry : GiftHistory)
		{
			if (Entry.bWasSent)
			{
				UniqueRecipients.Add(Entry.OtherPlayerID);
			}
		}
		Stats.UniqueRecipients = UniqueRecipients.Num();

		// Track most generous to
		TMap<FName, int32> ValueByRecipient;
		for (const FMGGiftHistory& Entry : GiftHistory)
		{
			if (Entry.bWasSent)
			{
				int32& TotalValue = ValueByRecipient.FindOrAdd(Entry.OtherPlayerID);
				TotalValue += Entry.TotalValue;
			}
		}

		int32 MaxValue = 0;
		for (const auto& Pair : ValueByRecipient)
		{
			if (Pair.Value > MaxValue)
			{
				MaxValue = Pair.Value;
				Stats.MostGenerousTo = Pair.Key;
			}
		}
	}
	else
	{
		Stats.TotalGiftsReceived++;
		Stats.TotalValueReceived += Gift.TotalValue;

		// Track unique senders
		TSet<FName> UniqueSenders;
		for (const FMGGiftHistory& Entry : GiftHistory)
		{
			if (!Entry.bWasSent && !Entry.OtherPlayerID.IsNone())
			{
				UniqueSenders.Add(Entry.OtherPlayerID);
			}
		}
		Stats.UniqueSenders = UniqueSenders.Num();

		// Track most generous from
		TMap<FName, int32> ValueBySender;
		for (const FMGGiftHistory& Entry : GiftHistory)
		{
			if (!Entry.bWasSent && !Entry.OtherPlayerID.IsNone())
			{
				int32& TotalValue = ValueBySender.FindOrAdd(Entry.OtherPlayerID);
				TotalValue += Entry.TotalValue;
			}
		}

		int32 MaxValue = 0;
		for (const auto& Pair : ValueBySender)
		{
			if (Pair.Value > MaxValue)
			{
				MaxValue = Pair.Value;
				Stats.MostGenerousFrom = Pair.Key;
			}
		}
	}
}

void UMGGiftSubsystem::SaveGiftData()
{
	// Persist gift data
	// Implementation would use USaveGame or cloud save
}

void UMGGiftSubsystem::LoadGiftData()
{
	// Load persisted gift data
	// Implementation would use USaveGame or cloud save
}

int32 UMGGiftSubsystem::CalculateGiftValue(const TArray<FMGGiftItem>& Items) const
{
	int32 TotalValue = 0;

	for (const FMGGiftItem& Item : Items)
	{
		if (Item.GiftType == EMGGiftType::Currency)
		{
			TotalValue += Item.CurrencyValue * Item.Quantity;
		}
		else
		{
			// Would look up item value from economy system
			TotalValue += Item.CurrencyValue > 0 ? Item.CurrencyValue * Item.Quantity : 100 * Item.Quantity;
		}
	}

	return TotalValue;
}
