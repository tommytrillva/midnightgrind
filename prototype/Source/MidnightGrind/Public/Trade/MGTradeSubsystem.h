// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTradeSubsystem.h
 * @brief Secure Player-to-Player Trading System for Items, Vehicles, and Currency
 * @author Midnight Grind Team
 * @date 2024
 *
 * @section overview_sec Overview
 * This subsystem handles real-time item trading between two players in Midnight Grind.
 * Trading allows players to exchange vehicles, parts, cosmetics, and currency in a
 * secure, verified manner with built-in scam protection.
 *
 * @section quickstart_sec Quick Start Example
 * @code
 * // Get the subsystem
 * UMGTradeSubsystem* Trade = GetGameInstance()->GetSubsystem<UMGTradeSubsystem>();
 *
 * // Send a trade request to another player
 * FGuid RequestId = Trade->SendTradeRequest(OtherPlayerID, FText::FromString("Want to trade cars?"));
 *
 * // Add items to your offer once trade is accepted
 * FMGTradeItem MyItem;
 * MyItem.ItemInstanceID = MyVehicleGuid;
 * MyItem.ItemType = EMGTradeItemType::Vehicle;
 * MyItem.DisplayName = FText::FromString("1999 Skyline GT-R");
 * Trade->AddItemToOffer(MyItem);
 *
 * // Add currency to the offer
 * Trade->SetOfferedCurrency(50000);
 *
 * // Lock your offer when ready (prevents further changes)
 * Trade->LockOffer();
 *
 * // Confirm the trade once both parties have locked
 * if (Trade->AreBothLocked())
 * {
 *     Trade->ConfirmTrade();
 * }
 *
 * // Listen for completion
 * Trade->OnTradeCompleted.AddDynamic(this, &UMyClass::OnTradeFinished);
 * @endcode
 *
 * @section concepts_sec Key Concepts for Beginners
 *
 * @subsection flow_subsec Trade Flow
 * The complete trading process follows these steps:
 * @verbatim
 * Step 1: Player A sends trade request to Player B
 * Step 2: Player B accepts, opening the trade window
 * Step 3: Both players add items/currency to their offers
 * Step 4: Both players "Lock" their offers (can no longer modify)
 * Step 5: Both players "Confirm" the trade
 * Step 6: Items are exchanged atomically (all or nothing)
 * @endverbatim
 *
 * @subsection state_subsec Trade State Machine (EMGTradeState)
 * | State       | Description                                    |
 * |-------------|------------------------------------------------|
 * | Proposed    | Request sent, waiting for acceptance           |
 * | Negotiating | Trade window open, players modifying offers    |
 * | Locked      | Both offers locked, awaiting confirmation      |
 * | Confirmed   | Both confirmed, executing transfer             |
 * | Completed   | Trade successful                               |
 * | Cancelled   | Trade was cancelled by a player                |
 * | Declined    | Trade request was declined                     |
 * | Expired     | Trade timed out                                |
 *
 * @subsection lockconfirm_subsec Lock-Confirm Pattern (Scam Protection)
 * This safety mechanism prevents last-second scams:
 * 1. Players must LOCK their offer before they can confirm
 * 2. Once locked, items cannot be added or removed
 * 3. A cooldown period (LockCooldownSeconds) prevents rapid lock/unlock
 * 4. This gives both players time to review the final offer
 *
 * @subsection fairness_subsec Value Fairness Warning
 * The system calculates total value of each offer:
 * - If one offer is worth significantly less than ValueWarningThreshold
 * - OnTradeValueWarning fires to alert the potentially disadvantaged player
 * - Helps prevent accidental unfair trades
 *
 * @subsection rarity_subsec Item Rarity (EMGTradeItemRarity)
 * @verbatim
 * Common < Uncommon < Rare < Epic < Legendary < Mythic < Unique
 * @endverbatim
 * Rarity affects item value calculations and may impose trade restrictions.
 *
 * @section security_sec Security Features
 * - **Atomic Transfers**: All items move or none do (no partial trades)
 * - **Item Locking**: Items in active trades cannot be sold or used elsewhere
 * - **Server Validation**: All operations validated server-side in online mode
 * - **Trade History**: Complete history saved for dispute resolution
 *
 * @section events_subsec Delegates/Events
 * | Event                   | Description                              |
 * |-------------------------|------------------------------------------|
 * | OnTradeRequestReceived  | Someone wants to trade with you          |
 * | OnTradeStarted          | Trade window opened                      |
 * | OnTradeStateChanged     | State machine transition                 |
 * | OnTradeOfferUpdated     | Partner added/removed items              |
 * | OnTradeLocked           | Lock status changed                      |
 * | OnTradeCompleted        | Trade finished successfully              |
 * | OnTradeCancelled        | Trade was cancelled                      |
 * | OnTradeValueWarning     | Unfair trade value detected              |
 *
 * @section related_sec Related Files
 * - MGTradeSubsystem.cpp: Implementation
 * - MGInventorySubsystem.h: Item management and ownership
 * - MGGarageSubsystem.h: Vehicle management for vehicle trades
 *
 * @see EMGTradeState, EMGTradeItemType, EMGTradeItemRarity
 * @see FMGTrade, FMGTradeOffer, FMGTradeItem, FMGTradeConfig
 */

/**
 * ============================================================================
 * MGTradeSubsystem.h - Player-to-Player Trading System
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Trade Subsystem, which handles real-time item trading
 * between two players in Midnight Grind. Trading allows players to exchange
 * vehicles, parts, cosmetics, and currency in a secure, verified manner.
 *
 * KEY CONCEPTS:
 *
 * 1. TRADE FLOW (How Trading Works):
 *    Step 1: Player A sends trade request to Player B
 *    Step 2: Player B accepts, opening the trade window
 *    Step 3: Both players add items/currency to their offers
 *    Step 4: Both players "Lock" their offers (can no longer modify)
 *    Step 5: Both players "Confirm" the trade
 *    Step 6: Items are exchanged atomically (all or nothing)
 *
 * 2. TRADE STATE MACHINE (EMGTradeState):
 *    - Proposed: Request sent, waiting for acceptance
 *    - Negotiating: Trade window open, players modifying offers
 *    - Locked: Both offers locked, awaiting confirmation
 *    - Confirmed: Both confirmed, executing transfer
 *    - Completed: Trade successful
 *    - Cancelled/Declined/Expired: Trade failed
 *
 * 3. LOCK-CONFIRM PATTERN:
 *    This is a safety mechanism to prevent last-second scams:
 *    - Players must LOCK their offer before confirming
 *    - Once locked, you cannot add/remove items
 *    - There's a cooldown (LockCooldownSeconds) before you can lock again
 *    - This gives players time to review the final offer
 *
 * 4. VALUE FAIRNESS WARNING:
 *    - The system calculates total value of each offer
 *    - If one offer is worth significantly less (< ValueWarningThreshold)
 *    - OnTradeValueWarning fires to alert the player
 *    - Helps prevent accidental unfair trades
 *
 * 5. ITEM RARITY SYSTEM (EMGTradeItemRarity):
 *    Common < Uncommon < Rare < Epic < Legendary < Mythic < Unique
 *    - Rarity affects item value calculations
 *    - Some items may have trade restrictions based on rarity
 *
 * 6. DELEGATES (Events):
 *    - OnTradeRequestReceived: Someone wants to trade with you
 *    - OnTradeStarted: Trade window opened
 *    - OnTradeStateChanged: State machine transition
 *    - OnTradeOfferUpdated: Partner added/removed items
 *    - OnTradeLocked: Lock status changed
 *    - OnTradeCompleted: Trade finished successfully
 *    - OnTradeCancelled: Trade was cancelled
 *    - OnTradeValueWarning: Unfair trade detected
 *
 * 7. FGUID (Globally Unique Identifier):
 *    - Used to uniquely identify trades, items, requests
 *    - 128-bit unique ID that won't collide
 *    - FGuid::NewGuid() creates a new one
 *
 * SECURITY CONSIDERATIONS:
 * - All transfers are atomic (all items move or none do)
 * - Items are "locked" during active trades (can't be sold/used)
 * - Server validates all trade operations in online mode
 * - Trade history is saved for dispute resolution
 *
 * RELATED FILES:
 * - MGTradeSubsystem.cpp (implementation)
 * - MGInventorySubsystem.h (item management)
 * - MGGarageSubsystem.h (vehicle management for vehicle trades)
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTradeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTradeState : uint8
{
	None,
	Proposed,
	Negotiating,
	Locked,
	Confirmed,
	Completed,
	Cancelled,
	Declined,
	Expired
};

UENUM(BlueprintType)
enum class EMGTradeItemType : uint8
{
	Vehicle,
	Part,
	Cosmetic,
	Currency,
	Experience,
	Consumable,
	Blueprint,
	Crate
};

UENUM(BlueprintType)
enum class EMGTradeItemRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Mythic,
	Unique
};

USTRUCT(BlueprintType)
struct FMGTradeItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemInstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTradeItemType ItemType = EMGTradeItemType::Part;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTradeItemRarity Rarity = EMGTradeItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 EstimatedValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MarketValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTradeable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> ItemStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 0;
};

USTRUCT(BlueprintType)
struct FMGTradeOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTradeItem> OfferedItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 OfferedCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsConfirmed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalValue = 0;
};

USTRUCT(BlueprintType)
struct FMGTrade
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TradeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTradeState State = EMGTradeState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTradeOffer InitiatorOffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTradeOffer RecipientOffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ModificationCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TradeNote;
};

USTRUCT(BlueprintType)
struct FMGTradeRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SenderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SenderLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SenderAvatar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTradeItem> RequestedItems;
};

USTRUCT(BlueprintType)
struct FMGTradeHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TradeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PartnerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTradeItem> ItemsGiven;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTradeItem> ItemsReceived;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyGiven = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasFairTrade = true;
};

USTRUCT(BlueprintType)
struct FMGTradeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TradeExpirationMinutes = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RequestExpirationMinutes = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxItemsPerTrade = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveTradeRequests = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLevelToTrade = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TradeTaxPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MaxCurrencyPerTrade = 10000000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireBothLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockCooldownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowValueWarning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ValueWarningThreshold = 0.5f;
};

USTRUCT(BlueprintType)
struct FMGTradeStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTradesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTradesCancelled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalValueTraded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclesTraded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PartsTraded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CosmeticsTraded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostTradedWith;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TradesWithMostTraded = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeRequestReceived, const FMGTradeRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeStarted, const FMGTrade&, Trade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeStateChanged, FGuid, TradeID, EMGTradeState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeOfferUpdated, const FMGTrade&, Trade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeLocked, FGuid, TradeID, bool, bBothLocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeCompleted, const FMGTradeHistory&, History);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCancelled, FGuid, TradeID, FName, CancelledBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeValueWarning, FGuid, TradeID, float, ValueRatio);

UCLASS()
class MIDNIGHTGRIND_API UMGTradeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Trade Requests
	UFUNCTION(BlueprintCallable, Category = "Trade|Request")
	FGuid SendTradeRequest(FName PlayerID, const FText& Message = FText::GetEmpty());

	UFUNCTION(BlueprintCallable, Category = "Trade|Request")
	bool AcceptTradeRequest(FGuid RequestID);

	UFUNCTION(BlueprintCallable, Category = "Trade|Request")
	bool DeclineTradeRequest(FGuid RequestID);

	UFUNCTION(BlueprintCallable, Category = "Trade|Request")
	bool CancelTradeRequest(FGuid RequestID);

	UFUNCTION(BlueprintPure, Category = "Trade|Request")
	TArray<FMGTradeRequest> GetPendingRequests() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Request")
	TArray<FMGTradeRequest> GetSentRequests() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Request")
	bool CanSendTradeRequest() const;

	// Active Trade
	UFUNCTION(BlueprintPure, Category = "Trade|Active")
	bool IsInTrade() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Active")
	FMGTrade GetActiveTrade() const { return ActiveTrade; }

	UFUNCTION(BlueprintPure, Category = "Trade|Active")
	FMGTradeOffer GetMyOffer() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Active")
	FMGTradeOffer GetPartnerOffer() const;

	// Item Management
	UFUNCTION(BlueprintCallable, Category = "Trade|Items")
	bool AddItemToOffer(const FMGTradeItem& Item);

	UFUNCTION(BlueprintCallable, Category = "Trade|Items")
	bool RemoveItemFromOffer(FGuid ItemInstanceID);

	UFUNCTION(BlueprintCallable, Category = "Trade|Items")
	bool SetOfferedCurrency(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Trade|Items")
	bool ClearMyOffer();

	UFUNCTION(BlueprintPure, Category = "Trade|Items")
	int32 GetMyItemCount() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Items")
	int32 GetRemainingSlots() const;

	// Trade Actions
	UFUNCTION(BlueprintCallable, Category = "Trade|Actions")
	bool LockOffer();

	UFUNCTION(BlueprintCallable, Category = "Trade|Actions")
	bool UnlockOffer();

	UFUNCTION(BlueprintCallable, Category = "Trade|Actions")
	bool ConfirmTrade();

	UFUNCTION(BlueprintCallable, Category = "Trade|Actions")
	bool CancelTrade();

	UFUNCTION(BlueprintPure, Category = "Trade|Actions")
	bool IsOfferLocked() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Actions")
	bool IsPartnerLocked() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Actions")
	bool AreBothLocked() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Actions")
	bool CanLockOffer() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Actions")
	float GetLockCooldownRemaining() const;

	// Validation
	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	bool IsItemTradeable(const FMGTradeItem& Item) const;

	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	bool CanTradeWithPlayer(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	float GetTradeValueRatio() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	bool IsFairTrade() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	int64 GetMyOfferTotalValue() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Validation")
	int64 GetPartnerOfferTotalValue() const;

	// Inventory
	UFUNCTION(BlueprintPure, Category = "Trade|Inventory")
	TArray<FMGTradeItem> GetTradeableItems() const;

	UFUNCTION(BlueprintPure, Category = "Trade|Inventory")
	TArray<FMGTradeItem> GetTradeableItemsByType(EMGTradeItemType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Trade|Inventory")
	void RefreshInventory();

	// History
	UFUNCTION(BlueprintPure, Category = "Trade|History")
	TArray<FMGTradeHistory> GetTradeHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintPure, Category = "Trade|History")
	TArray<FMGTradeHistory> GetTradesWithPlayer(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Trade|History")
	FMGTradeStats GetTradeStats() const { return Stats; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Trade|Config")
	void SetConfig(const FMGTradeConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Trade|Config")
	FMGTradeConfig GetConfig() const { return Config; }

	// Player Info
	UFUNCTION(BlueprintCallable, Category = "Trade|Player")
	void SetLocalPlayerInfo(FName PlayerID, const FString& PlayerName, int32 Level);

	UFUNCTION(BlueprintPure, Category = "Trade|Player")
	bool CanTrade() const;

	// Network
	UFUNCTION(BlueprintCallable, Category = "Trade|Network")
	void ReceiveTradeRequest(const FMGTradeRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Trade|Network")
	void ReceiveTradeUpdate(const FMGTrade& Trade);

	UFUNCTION(BlueprintCallable, Category = "Trade|Network")
	void ReceiveTradeCompletion(const FMGTradeHistory& History);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeRequestReceived OnTradeRequestReceived;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeStarted OnTradeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeStateChanged OnTradeStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeOfferUpdated OnTradeOfferUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeLocked OnTradeLocked;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeCancelled OnTradeCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Trade|Events")
	FOnTradeValueWarning OnTradeValueWarning;

protected:
	void OnTradeTick();
	void CheckExpiredTrades();
	void ProcessTradeCompletion();
	void TransferItems();
	void UpdateOfferValue(FMGTradeOffer& Offer);
	void SetTradeState(EMGTradeState NewState);
	void SaveTradeData();
	void LoadTradeData();
	FMGTradeOffer* GetLocalOffer();
	FMGTradeOffer* GetRemoteOffer();
	bool IsInitiator() const;

	UPROPERTY()
	FMGTrade ActiveTrade;

	UPROPERTY()
	TArray<FMGTradeRequest> PendingRequests;

	UPROPERTY()
	TArray<FMGTradeRequest> SentRequests;

	UPROPERTY()
	TArray<FMGTradeHistory> TradeHistory;

	UPROPERTY()
	TArray<FMGTradeItem> CachedInventory;

	UPROPERTY()
	FMGTradeConfig Config;

	UPROPERTY()
	FMGTradeStats Stats;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;

	UPROPERTY()
	int32 LocalPlayerLevel = 1;

	UPROPERTY()
	float LockCooldownRemaining = 0.0f;

	FTimerHandle TradeTickHandle;
};
