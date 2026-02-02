// Copyright Midnight Grind. All Rights Reserved.

#include "Trade/MGTradeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGTradeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default configuration
	Config.TradeExpirationMinutes = 10.0f;
	Config.RequestExpirationMinutes = 5.0f;
	Config.MaxItemsPerTrade = 10;
	Config.MaxActiveTradeRequests = 5;
	Config.MinLevelToTrade = 5;
	Config.TradeTaxPercent = 0.0f;
	Config.MaxCurrencyPerTrade = 10000000;
	Config.bRequireBothLocked = true;
	Config.LockCooldownSeconds = 3.0f;
	Config.bShowValueWarning = true;
	Config.ValueWarningThreshold = 0.5f;

	LoadTradeData();

	// Start trade tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TradeTickHandle,
			this,
			&UMGTradeSubsystem::OnTradeTick,
			1.0f,
			true
		);
	}
}

void UMGTradeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TradeTickHandle);
	}

	// Cancel any active trade
	if (IsInTrade())
	{
		CancelTrade();
	}

	SaveTradeData();

	Super::Deinitialize();
}

bool UMGTradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FGuid UMGTradeSubsystem::SendTradeRequest(FName PlayerID, const FText& Message)
{
	if (!CanSendTradeRequest())
	{
		return FGuid();
	}

	if (!CanTradeWithPlayer(PlayerID))
	{
		return FGuid();
	}

	FMGTradeRequest Request;
	Request.RequestID = FGuid::NewGuid();
	Request.SenderID = LocalPlayerID;
	Request.SenderName = LocalPlayerName;
	Request.SenderLevel = LocalPlayerLevel;
	Request.SentAt = FDateTime::UtcNow();
	Request.ExpiresAt = Request.SentAt + FTimespan::FromMinutes(Config.RequestExpirationMinutes);
	Request.Message = Message;

	SentRequests.Add(Request);

	return Request.RequestID;
}

bool UMGTradeSubsystem::AcceptTradeRequest(FGuid RequestID)
{
	for (int32 i = PendingRequests.Num() - 1; i >= 0; i--)
	{
		if (PendingRequests[i].RequestID == RequestID)
		{
			FMGTradeRequest& Request = PendingRequests[i];

			if (FDateTime::UtcNow() > Request.ExpiresAt)
			{
				PendingRequests.RemoveAt(i);
				return false;
			}

			// Start the trade
			ActiveTrade.TradeID = FGuid::NewGuid();
			ActiveTrade.State = EMGTradeState::Negotiating;
			ActiveTrade.CreatedAt = FDateTime::UtcNow();
			ActiveTrade.ExpiresAt = ActiveTrade.CreatedAt + FTimespan::FromMinutes(Config.TradeExpirationMinutes);

			// Set up offers
			ActiveTrade.InitiatorOffer.PlayerID = Request.SenderID;
			ActiveTrade.InitiatorOffer.PlayerName = Request.SenderName;
			ActiveTrade.RecipientOffer.PlayerID = LocalPlayerID;
			ActiveTrade.RecipientOffer.PlayerName = LocalPlayerName;

			PendingRequests.RemoveAt(i);

			SetTradeState(EMGTradeState::Negotiating);
			OnTradeStarted.Broadcast(ActiveTrade);

			return true;
		}
	}
	return false;
}

bool UMGTradeSubsystem::DeclineTradeRequest(FGuid RequestID)
{
	for (int32 i = PendingRequests.Num() - 1; i >= 0; i--)
	{
		if (PendingRequests[i].RequestID == RequestID)
		{
			PendingRequests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGTradeSubsystem::CancelTradeRequest(FGuid RequestID)
{
	for (int32 i = SentRequests.Num() - 1; i >= 0; i--)
	{
		if (SentRequests[i].RequestID == RequestID)
		{
			SentRequests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

TArray<FMGTradeRequest> UMGTradeSubsystem::GetPendingRequests() const
{
	return PendingRequests;
}

TArray<FMGTradeRequest> UMGTradeSubsystem::GetSentRequests() const
{
	return SentRequests;
}

bool UMGTradeSubsystem::CanSendTradeRequest() const
{
	if (!CanTrade())
	{
		return false;
	}

	if (IsInTrade())
	{
		return false;
	}

	if (SentRequests.Num() >= Config.MaxActiveTradeRequests)
	{
		return false;
	}

	return true;
}

bool UMGTradeSubsystem::IsInTrade() const
{
	return ActiveTrade.TradeID.IsValid() &&
		   ActiveTrade.State != EMGTradeState::None &&
		   ActiveTrade.State != EMGTradeState::Completed &&
		   ActiveTrade.State != EMGTradeState::Cancelled &&
		   ActiveTrade.State != EMGTradeState::Declined;
}

FMGTradeOffer UMGTradeSubsystem::GetMyOffer() const
{
	if (IsInitiator())
	{
		return ActiveTrade.InitiatorOffer;
	}
	return ActiveTrade.RecipientOffer;
}

FMGTradeOffer UMGTradeSubsystem::GetPartnerOffer() const
{
	if (IsInitiator())
	{
		return ActiveTrade.RecipientOffer;
	}
	return ActiveTrade.InitiatorOffer;
}

bool UMGTradeSubsystem::AddItemToOffer(const FMGTradeItem& Item)
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsOfferLocked())
	{
		return false;
	}

	if (!IsItemTradeable(Item))
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	if (MyOffer->OfferedItems.Num() >= Config.MaxItemsPerTrade)
	{
		return false;
	}

	// Check if item is already in offer
	for (const FMGTradeItem& ExistingItem : MyOffer->OfferedItems)
	{
		if (ExistingItem.ItemInstanceID == Item.ItemInstanceID)
		{
			return false;
		}
	}

	MyOffer->OfferedItems.Add(Item);
	UpdateOfferValue(*MyOffer);

	// Unlock partner if they were locked (modification occurred)
	FMGTradeOffer* PartnerOffer = GetRemoteOffer();
	if (PartnerOffer && PartnerOffer->bIsLocked)
	{
		PartnerOffer->bIsLocked = false;
		PartnerOffer->bIsConfirmed = false;
	}

	ActiveTrade.ModificationCount++;
	OnTradeOfferUpdated.Broadcast(ActiveTrade);

	// Check for value warning
	if (Config.bShowValueWarning)
	{
		float Ratio = GetTradeValueRatio();
		if (Ratio < Config.ValueWarningThreshold || Ratio > (1.0f / Config.ValueWarningThreshold))
		{
			OnTradeValueWarning.Broadcast(ActiveTrade.TradeID, Ratio);
		}
	}

	return true;
}

bool UMGTradeSubsystem::RemoveItemFromOffer(FGuid ItemInstanceID)
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsOfferLocked())
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	for (int32 i = MyOffer->OfferedItems.Num() - 1; i >= 0; i--)
	{
		if (MyOffer->OfferedItems[i].ItemInstanceID == ItemInstanceID)
		{
			MyOffer->OfferedItems.RemoveAt(i);
			UpdateOfferValue(*MyOffer);

			// Unlock partner if they were locked
			FMGTradeOffer* PartnerOffer = GetRemoteOffer();
			if (PartnerOffer && PartnerOffer->bIsLocked)
			{
				PartnerOffer->bIsLocked = false;
				PartnerOffer->bIsConfirmed = false;
			}

			ActiveTrade.ModificationCount++;
			OnTradeOfferUpdated.Broadcast(ActiveTrade);
			return true;
		}
	}
	return false;
}

bool UMGTradeSubsystem::SetOfferedCurrency(int64 Amount)
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsOfferLocked())
	{
		return false;
	}

	if (Amount < 0 || Amount > Config.MaxCurrencyPerTrade)
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	MyOffer->OfferedCurrency = Amount;
	UpdateOfferValue(*MyOffer);

	// Unlock partner if they were locked
	FMGTradeOffer* PartnerOffer = GetRemoteOffer();
	if (PartnerOffer && PartnerOffer->bIsLocked)
	{
		PartnerOffer->bIsLocked = false;
		PartnerOffer->bIsConfirmed = false;
	}

	ActiveTrade.ModificationCount++;
	OnTradeOfferUpdated.Broadcast(ActiveTrade);

	return true;
}

bool UMGTradeSubsystem::ClearMyOffer()
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsOfferLocked())
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	MyOffer->OfferedItems.Empty();
	MyOffer->OfferedCurrency = 0;
	MyOffer->TotalValue = 0;

	// Unlock partner if they were locked
	FMGTradeOffer* PartnerOffer = GetRemoteOffer();
	if (PartnerOffer && PartnerOffer->bIsLocked)
	{
		PartnerOffer->bIsLocked = false;
		PartnerOffer->bIsConfirmed = false;
	}

	ActiveTrade.ModificationCount++;
	OnTradeOfferUpdated.Broadcast(ActiveTrade);

	return true;
}

int32 UMGTradeSubsystem::GetMyItemCount() const
{
	if (!IsInTrade())
	{
		return 0;
	}

	if (IsInitiator())
	{
		return ActiveTrade.InitiatorOffer.OfferedItems.Num();
	}
	return ActiveTrade.RecipientOffer.OfferedItems.Num();
}

int32 UMGTradeSubsystem::GetRemainingSlots() const
{
	return Config.MaxItemsPerTrade - GetMyItemCount();
}

bool UMGTradeSubsystem::LockOffer()
{
	if (!IsInTrade())
	{
		return false;
	}

	if (!CanLockOffer())
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	MyOffer->bIsLocked = true;
	LockCooldownRemaining = Config.LockCooldownSeconds;

	bool bBothLocked = AreBothLocked();
	OnTradeLocked.Broadcast(ActiveTrade.TradeID, bBothLocked);

	if (bBothLocked)
	{
		SetTradeState(EMGTradeState::Locked);
	}

	OnTradeOfferUpdated.Broadcast(ActiveTrade);

	return true;
}

bool UMGTradeSubsystem::UnlockOffer()
{
	if (!IsInTrade())
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	if (!MyOffer->bIsLocked)
	{
		return false;
	}

	MyOffer->bIsLocked = false;
	MyOffer->bIsConfirmed = false;

	SetTradeState(EMGTradeState::Negotiating);
	OnTradeOfferUpdated.Broadcast(ActiveTrade);

	return true;
}

bool UMGTradeSubsystem::ConfirmTrade()
{
	if (!IsInTrade())
	{
		return false;
	}

	if (Config.bRequireBothLocked && !AreBothLocked())
	{
		return false;
	}

	FMGTradeOffer* MyOffer = GetLocalOffer();
	if (!MyOffer)
	{
		return false;
	}

	MyOffer->bIsConfirmed = true;

	// Check if both confirmed
	FMGTradeOffer* PartnerOffer = GetRemoteOffer();
	if (PartnerOffer && PartnerOffer->bIsConfirmed && MyOffer->bIsConfirmed)
	{
		ProcessTradeCompletion();
	}
	else
	{
		SetTradeState(EMGTradeState::Confirmed);
		OnTradeOfferUpdated.Broadcast(ActiveTrade);
	}

	return true;
}

bool UMGTradeSubsystem::CancelTrade()
{
	if (!IsInTrade())
	{
		return false;
	}

	FGuid TradeID = ActiveTrade.TradeID;
	SetTradeState(EMGTradeState::Cancelled);

	OnTradeCancelled.Broadcast(TradeID, LocalPlayerID);

	Stats.TotalTradesCancelled++;
	SaveTradeData();

	ActiveTrade = FMGTrade();

	return true;
}

bool UMGTradeSubsystem::IsOfferLocked() const
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsInitiator())
	{
		return ActiveTrade.InitiatorOffer.bIsLocked;
	}
	return ActiveTrade.RecipientOffer.bIsLocked;
}

bool UMGTradeSubsystem::IsPartnerLocked() const
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsInitiator())
	{
		return ActiveTrade.RecipientOffer.bIsLocked;
	}
	return ActiveTrade.InitiatorOffer.bIsLocked;
}

bool UMGTradeSubsystem::AreBothLocked() const
{
	return IsOfferLocked() && IsPartnerLocked();
}

bool UMGTradeSubsystem::CanLockOffer() const
{
	if (!IsInTrade())
	{
		return false;
	}

	if (IsOfferLocked())
	{
		return false;
	}

	if (LockCooldownRemaining > 0.0f)
	{
		return false;
	}

	return true;
}

float UMGTradeSubsystem::GetLockCooldownRemaining() const
{
	return LockCooldownRemaining;
}

bool UMGTradeSubsystem::IsItemTradeable(const FMGTradeItem& Item) const
{
	if (!Item.bIsTradeable)
	{
		return false;
	}

	if (Item.bIsLocked)
	{
		return false;
	}

	return true;
}

bool UMGTradeSubsystem::CanTradeWithPlayer(FName PlayerID) const
{
	if (PlayerID == LocalPlayerID)
	{
		return false;
	}

	// Could add blocked players check, level requirements, etc.
	return true;
}

float UMGTradeSubsystem::GetTradeValueRatio() const
{
	int64 MyValue = GetMyOfferTotalValue();
	int64 PartnerValue = GetPartnerOfferTotalValue();

	if (MyValue == 0 && PartnerValue == 0)
	{
		return 1.0f;
	}

	if (PartnerValue == 0)
	{
		return 0.0f;
	}

	return (float)MyValue / (float)PartnerValue;
}

bool UMGTradeSubsystem::IsFairTrade() const
{
	float Ratio = GetTradeValueRatio();
	return Ratio >= Config.ValueWarningThreshold && Ratio <= (1.0f / Config.ValueWarningThreshold);
}

int64 UMGTradeSubsystem::GetMyOfferTotalValue() const
{
	if (IsInitiator())
	{
		return ActiveTrade.InitiatorOffer.TotalValue;
	}
	return ActiveTrade.RecipientOffer.TotalValue;
}

int64 UMGTradeSubsystem::GetPartnerOfferTotalValue() const
{
	if (IsInitiator())
	{
		return ActiveTrade.RecipientOffer.TotalValue;
	}
	return ActiveTrade.InitiatorOffer.TotalValue;
}

TArray<FMGTradeItem> UMGTradeSubsystem::GetTradeableItems() const
{
	TArray<FMGTradeItem> Result;
	for (const FMGTradeItem& Item : CachedInventory)
	{
		if (IsItemTradeable(Item))
		{
			Result.Add(Item);
		}
	}
	return Result;
}

TArray<FMGTradeItem> UMGTradeSubsystem::GetTradeableItemsByType(EMGTradeItemType Type) const
{
	TArray<FMGTradeItem> Result;
	for (const FMGTradeItem& Item : CachedInventory)
	{
		if (Item.ItemType == Type && IsItemTradeable(Item))
		{
			Result.Add(Item);
		}
	}
	return Result;
}

void UMGTradeSubsystem::RefreshInventory()
{
	// This would fetch inventory from the inventory/garage subsystems
	CachedInventory.Empty();
}

TArray<FMGTradeHistory> UMGTradeSubsystem::GetTradeHistory(int32 MaxEntries) const
{
	TArray<FMGTradeHistory> Result;
	int32 Count = FMath::Min(MaxEntries, TradeHistory.Num());
	for (int32 i = 0; i < Count; i++)
	{
		Result.Add(TradeHistory[i]);
	}
	return Result;
}

TArray<FMGTradeHistory> UMGTradeSubsystem::GetTradesWithPlayer(FName PlayerID) const
{
	TArray<FMGTradeHistory> Result;
	for (const FMGTradeHistory& History : TradeHistory)
	{
		if (History.PartnerID == PlayerID)
		{
			Result.Add(History);
		}
	}
	return Result;
}

void UMGTradeSubsystem::SetConfig(const FMGTradeConfig& NewConfig)
{
	Config = NewConfig;
}

void UMGTradeSubsystem::SetLocalPlayerInfo(FName PlayerID, const FString& PlayerName, int32 Level)
{
	LocalPlayerID = PlayerID;
	LocalPlayerName = PlayerName;
	LocalPlayerLevel = Level;
}

bool UMGTradeSubsystem::CanTrade() const
{
	return LocalPlayerLevel >= Config.MinLevelToTrade;
}

void UMGTradeSubsystem::ReceiveTradeRequest(const FMGTradeRequest& Request)
{
	// Check if we're already in a trade
	if (IsInTrade())
	{
		return;
	}

	// Check for duplicate
	for (const FMGTradeRequest& Existing : PendingRequests)
	{
		if (Existing.SenderID == Request.SenderID)
		{
			return;
		}
	}

	PendingRequests.Add(Request);
	OnTradeRequestReceived.Broadcast(Request);
}

void UMGTradeSubsystem::ReceiveTradeUpdate(const FMGTrade& Trade)
{
	if (!IsInTrade())
	{
		return;
	}

	if (Trade.TradeID != ActiveTrade.TradeID)
	{
		return;
	}

	EMGTradeState PreviousState = ActiveTrade.State;

	// Update partner's offer
	if (IsInitiator())
	{
		ActiveTrade.RecipientOffer = Trade.RecipientOffer;
	}
	else
	{
		ActiveTrade.InitiatorOffer = Trade.InitiatorOffer;
	}

	ActiveTrade.State = Trade.State;
	ActiveTrade.ModificationCount = Trade.ModificationCount;

	if (PreviousState != ActiveTrade.State)
	{
		OnTradeStateChanged.Broadcast(ActiveTrade.TradeID, ActiveTrade.State);
	}

	OnTradeOfferUpdated.Broadcast(ActiveTrade);
}

void UMGTradeSubsystem::ReceiveTradeCompletion(const FMGTradeHistory& History)
{
	// Add to history
	TradeHistory.Insert(History, 0);

	// Limit history size
	const int32 MaxHistorySize = 100;
	if (TradeHistory.Num() > MaxHistorySize)
	{
		TradeHistory.SetNum(MaxHistorySize);
	}

	// Update stats
	Stats.TotalTradesCompleted++;
	Stats.TotalValueTraded += History.CurrencyGiven + History.CurrencyReceived;

	for (const FMGTradeItem& Item : History.ItemsGiven)
	{
		switch (Item.ItemType)
		{
		case EMGTradeItemType::Vehicle:
			Stats.VehiclesTraded++;
			break;
		case EMGTradeItemType::Part:
			Stats.PartsTraded++;
			break;
		case EMGTradeItemType::Cosmetic:
			Stats.CosmeticsTraded++;
			break;
		default:
			break;
		}
	}

	SaveTradeData();

	OnTradeCompleted.Broadcast(History);

	// Clear active trade
	ActiveTrade = FMGTrade();
}

void UMGTradeSubsystem::OnTradeTick()
{
	// Update cooldowns
	if (LockCooldownRemaining > 0.0f)
	{
		LockCooldownRemaining -= 1.0f;
		if (LockCooldownRemaining < 0.0f)
		{
			LockCooldownRemaining = 0.0f;
		}
	}

	CheckExpiredTrades();
}

void UMGTradeSubsystem::CheckExpiredTrades()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check pending requests
	for (int32 i = PendingRequests.Num() - 1; i >= 0; i--)
	{
		if (Now > PendingRequests[i].ExpiresAt)
		{
			PendingRequests.RemoveAt(i);
		}
	}

	// Check sent requests
	for (int32 i = SentRequests.Num() - 1; i >= 0; i--)
	{
		if (Now > SentRequests[i].ExpiresAt)
		{
			SentRequests.RemoveAt(i);
		}
	}

	// Check active trade
	if (IsInTrade() && Now > ActiveTrade.ExpiresAt)
	{
		SetTradeState(EMGTradeState::Expired);
		OnTradeCancelled.Broadcast(ActiveTrade.TradeID, FName());
		ActiveTrade = FMGTrade();
	}
}

void UMGTradeSubsystem::ProcessTradeCompletion()
{
	// Create history entry
	FMGTradeHistory History;
	History.TradeID = ActiveTrade.TradeID;
	History.CompletedAt = FDateTime::UtcNow();
	History.bWasFairTrade = IsFairTrade();

	FMGTradeOffer* MyOffer = GetLocalOffer();
	FMGTradeOffer* PartnerOffer = GetRemoteOffer();

	if (MyOffer && PartnerOffer)
	{
		History.PartnerID = PartnerOffer->PlayerID;
		History.PartnerName = PartnerOffer->PlayerName;
		History.ItemsGiven = MyOffer->OfferedItems;
		History.ItemsReceived = PartnerOffer->OfferedItems;
		History.CurrencyGiven = MyOffer->OfferedCurrency;
		History.CurrencyReceived = PartnerOffer->OfferedCurrency;
	}

	SetTradeState(EMGTradeState::Completed);
	ActiveTrade.CompletedAt = FDateTime::UtcNow();

	// Transfer items
	TransferItems();

	ReceiveTradeCompletion(History);
}

void UMGTradeSubsystem::TransferItems()
{
	// This would integrate with inventory/economy subsystems to actually transfer items
	// For now, do nothing - the receiving/sending is handled by those systems
}

void UMGTradeSubsystem::UpdateOfferValue(FMGTradeOffer& Offer)
{
	int64 TotalValue = Offer.OfferedCurrency;
	for (const FMGTradeItem& Item : Offer.OfferedItems)
	{
		TotalValue += Item.MarketValue > 0 ? Item.MarketValue : Item.EstimatedValue;
	}
	Offer.TotalValue = TotalValue;
}

void UMGTradeSubsystem::SetTradeState(EMGTradeState NewState)
{
	if (ActiveTrade.State != NewState)
	{
		ActiveTrade.State = NewState;
		OnTradeStateChanged.Broadcast(ActiveTrade.TradeID, NewState);
	}
}

void UMGTradeSubsystem::SaveTradeData()
{
	// This would integrate with save game system
}

void UMGTradeSubsystem::LoadTradeData()
{
	// This would integrate with save game system
}

FMGTradeOffer* UMGTradeSubsystem::GetLocalOffer()
{
	if (!IsInTrade())
	{
		return nullptr;
	}

	if (IsInitiator())
	{
		return &ActiveTrade.InitiatorOffer;
	}
	return &ActiveTrade.RecipientOffer;
}

FMGTradeOffer* UMGTradeSubsystem::GetRemoteOffer()
{
	if (!IsInTrade())
	{
		return nullptr;
	}

	if (IsInitiator())
	{
		return &ActiveTrade.RecipientOffer;
	}
	return &ActiveTrade.InitiatorOffer;
}

bool UMGTradeSubsystem::IsInitiator() const
{
	return ActiveTrade.InitiatorOffer.PlayerID == LocalPlayerID;
}
