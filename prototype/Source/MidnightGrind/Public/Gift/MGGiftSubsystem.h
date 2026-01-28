// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGGiftSubsystem.h - Player-to-Player Gifting System
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Gift Subsystem, which allows players to send items,
 * currency, and gift bundles to other players in Midnight Grind. This is a
 * social feature that strengthens community bonds and enables generous
 * interactions between players.
 *
 * KEY CONCEPTS:
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    - This class inherits from UGameInstanceSubsystem
 *    - Subsystems are automatically created and managed by Unreal Engine
 *    - GameInstance subsystems persist for the entire game session
 *    - Access via: GameInstance->GetSubsystem<UMGGiftSubsystem>()
 *
 * 2. BLUEPRINTTYPE / BLUEPRINTCALLABLE:
 *    - UENUM(BlueprintType): Makes enum usable in Blueprints (visual scripting)
 *    - USTRUCT(BlueprintType): Makes struct usable in Blueprints
 *    - UFUNCTION(BlueprintCallable): Function can be called from Blueprints
 *    - UFUNCTION(BlueprintPure): Function has no side effects, only returns data
 *    - UPROPERTY(BlueprintAssignable): Delegate can be bound in Blueprints
 *
 * 3. DELEGATES:
 *    - DECLARE_DYNAMIC_MULTICAST_DELEGATE: Event system for notifying listeners
 *    - Multiple listeners can subscribe to one delegate
 *    - Used here for events like OnGiftSent, OnGiftReceived, etc.
 *    - UI widgets typically bind to these to update when gifts arrive
 *
 * 4. GENERATED_BODY():
 *    - Required macro in all UCLASS, USTRUCT, UENUM declarations
 *    - Unreal Header Tool (UHT) generates reflection code here
 *    - Never remove or modify this macro
 *
 * 5. SOFT OBJECT POINTERS (TSoftObjectPtr):
 *    - Reference to an asset that isn't loaded into memory immediately
 *    - Useful for textures/icons that may not always be needed
 *    - Prevents memory bloat from loading all gift icons at once
 *
 * GIFT LIFECYCLE (State Machine):
 *    Pending -> Sent -> Delivered -> Claimed (success)
 *                                 -> Expired (timeout)
 *                                 -> Returned (declined)
 *                    -> Cancelled (by sender)
 *
 * RELATED FILES:
 * - MGGiftSubsystem.cpp (implementation)
 * - MGSocialSubsystem.h (friends list integration)
 * - MGInventorySubsystem.h (item management)
 *
 * ============================================================================
 *
 * @file MGGiftSubsystem.h
 * @brief Player-to-player gifting system for Midnight Grind
 *
 * The Gift Subsystem enables players to send items, currency, and bundles to
 * friends and other players. This social feature strengthens player connections
 * and enables generous interactions within the community.
 *
 * @section gift_features_sec Core Features
 * - **Item Gifting**: Send vehicles, parts, cosmetics, and consumables
 * - **Currency Gifting**: Share in-game currency with friends
 * - **Gift Bundles**: Curated collections available for gifting
 * - **Wrap Styles**: Visual customization for gift presentation
 * - **Anonymous Gifting**: Option to send gifts without revealing identity
 * - **Gift History**: Track sent and received gifts over time
 *
 * @section gift_flow_sec Gift Lifecycle
 * 1. **Pending**: Gift created but not yet sent to server
 * 2. **Sent**: Gift transmitted, awaiting delivery
 * 3. **Delivered**: Gift arrived in recipient's inbox
 * 4. **Claimed**: Recipient accepted and received items
 * 5. **Expired**: Gift not claimed within time limit (returned to sender)
 * 6. **Returned**: Recipient declined or gift bounced back
 * 7. **Cancelled**: Sender cancelled before delivery
 *
 * @section gift_settings_sec Privacy Settings
 * Players can configure who can send them gifts:
 * - Friends only (default)
 * - Anyone in the game
 * - Accept/reject anonymous gifts
 * - Auto-claim or manual claim
 *
 * @section gift_usage_sec Example Usage
 * @code
 * UMGGiftSubsystem* GiftSub = GameInstance->GetSubsystem<UMGGiftSubsystem>();
 *
 * // Send currency to a friend
 * GiftSub->SendCurrencyGift(FriendID, 1000, FText::FromString("GG on that last race!"));
 *
 * // Check for pending gifts
 * if (GiftSub->GetPendingGiftCount() > 0)
 * {
 *     GiftSub->ClaimAllGifts();
 * }
 * @endcode
 *
 * @see UMGSocialSubsystem For friends list integration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGGiftSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGGiftType : uint8
{
	Item,
	Vehicle,
	Currency,
	SeasonPass,
	Bundle,
	Custom
};

UENUM(BlueprintType)
enum class EMGGiftStatus : uint8
{
	Pending,
	Sent,
	Delivered,
	Claimed,
	Expired,
	Returned,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGGiftWrapStyle : uint8
{
	Default,
	Birthday,
	Holiday,
	Victory,
	Special,
	Premium,
	Animated
};

USTRUCT(BlueprintType)
struct FMGGiftItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGiftType GiftType = EMGGiftType::Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGiftable = true;
};

USTRUCT(BlueprintType)
struct FMGGift
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid GiftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SenderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipientID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecipientName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGGiftItem> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PersonalMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGiftStatus Status = EMGGiftStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGiftWrapStyle WrapStyle = EMGGiftWrapStyle::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ClaimedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnonymous = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;
};

USTRUCT(BlueprintType)
struct FMGGiftBundle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BundleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BundleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGGiftItem> Contents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiscountPercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLimitedTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AvailableUntil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> BundleIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGiftWrapStyle DefaultWrap = EMGGiftWrapStyle::Special;
};

USTRUCT(BlueprintType)
struct FMGGiftHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid GiftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasSent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OtherPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OtherPlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime TransactionDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGiftStatus FinalStatus = EMGGiftStatus::Claimed;
};

USTRUCT(BlueprintType)
struct FMGGiftSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAcceptGiftsFromFriends = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAcceptGiftsFromAnyone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAcceptAnonymousGifts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNotifyOnGiftReceived = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoClaimGifts = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPendingGifts = 50;
};

USTRUCT(BlueprintType)
struct FMGGiftStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGiftsSent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGiftsReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValueSent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValueReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueRecipients = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueSenders = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostGenerousTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostGenerousFrom;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGiftSent, const FMGGift&, Gift);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGiftReceived, const FMGGift&, Gift);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGiftClaimed, FGuid, GiftID, const TArray<FMGGiftItem>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGiftExpired, FGuid, GiftID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGiftReturned, const FMGGift&, Gift);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGiftStatusChanged, FGuid, GiftID, EMGGiftStatus, NewStatus);

UCLASS()
class MIDNIGHTGRIND_API UMGGiftSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Sending Gifts
	UFUNCTION(BlueprintCallable, Category = "Gift|Send")
	FGuid SendGift(FName RecipientID, const TArray<FMGGiftItem>& Items, const FText& Message, EMGGiftWrapStyle WrapStyle = EMGGiftWrapStyle::Default, bool bAnonymous = false);

	UFUNCTION(BlueprintCallable, Category = "Gift|Send")
	FGuid SendCurrencyGift(FName RecipientID, int32 Amount, const FText& Message, bool bAnonymous = false);

	UFUNCTION(BlueprintCallable, Category = "Gift|Send")
	FGuid SendBundleGift(FName RecipientID, FName BundleID, const FText& Message, bool bAnonymous = false);

	UFUNCTION(BlueprintPure, Category = "Gift|Send")
	bool CanSendGift(FName RecipientID) const;

	UFUNCTION(BlueprintPure, Category = "Gift|Send")
	bool CanGiftItem(FName ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Gift|Send")
	bool CancelGift(FGuid GiftID);

	// Receiving Gifts
	UFUNCTION(BlueprintPure, Category = "Gift|Receive")
	TArray<FMGGift> GetPendingGifts() const;

	UFUNCTION(BlueprintPure, Category = "Gift|Receive")
	int32 GetPendingGiftCount() const;

	UFUNCTION(BlueprintPure, Category = "Gift|Receive")
	FMGGift GetGift(FGuid GiftID) const;

	UFUNCTION(BlueprintCallable, Category = "Gift|Receive")
	TArray<FMGGiftItem> ClaimGift(FGuid GiftID);

	UFUNCTION(BlueprintCallable, Category = "Gift|Receive")
	TArray<FMGGiftItem> ClaimAllGifts();

	UFUNCTION(BlueprintCallable, Category = "Gift|Receive")
	bool ReturnGift(FGuid GiftID);

	UFUNCTION(BlueprintPure, Category = "Gift|Receive")
	bool CanAcceptGiftFrom(FName SenderID) const;

	// Gift Bundles
	UFUNCTION(BlueprintPure, Category = "Gift|Bundles")
	TArray<FMGGiftBundle> GetAvailableBundles() const;

	UFUNCTION(BlueprintPure, Category = "Gift|Bundles")
	FMGGiftBundle GetBundle(FName BundleID) const;

	UFUNCTION(BlueprintCallable, Category = "Gift|Bundles")
	void RegisterBundle(const FMGGiftBundle& Bundle);

	UFUNCTION(BlueprintPure, Category = "Gift|Bundles")
	int32 GetBundlePrice(FName BundleID) const;

	// Giftable Items
	UFUNCTION(BlueprintPure, Category = "Gift|Items")
	TArray<FMGGiftItem> GetGiftableItems() const;

	UFUNCTION(BlueprintCallable, Category = "Gift|Items")
	void RegisterGiftableItem(const FMGGiftItem& Item);

	UFUNCTION(BlueprintPure, Category = "Gift|Items")
	FMGGiftItem GetGiftableItem(FName ItemID) const;

	// History
	UFUNCTION(BlueprintPure, Category = "Gift|History")
	TArray<FMGGiftHistory> GetSentHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintPure, Category = "Gift|History")
	TArray<FMGGiftHistory> GetReceivedHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintPure, Category = "Gift|History")
	TArray<FMGGiftHistory> GetHistoryWithPlayer(FName PlayerID) const;

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Gift|Settings")
	void SetGiftSettings(const FMGGiftSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Gift|Settings")
	FMGGiftSettings GetGiftSettings() const { return Settings; }

	// Stats
	UFUNCTION(BlueprintPure, Category = "Gift|Stats")
	FMGGiftStats GetGiftStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Gift|Stats")
	int32 GetTotalGiftsWithPlayer(FName PlayerID) const;

	// Network (receive from server)
	UFUNCTION(BlueprintCallable, Category = "Gift|Network")
	void ReceiveGift(const FMGGift& Gift);

	UFUNCTION(BlueprintCallable, Category = "Gift|Network")
	void UpdateGiftStatus(FGuid GiftID, EMGGiftStatus NewStatus);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftSent OnGiftSent;

	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftReceived OnGiftReceived;

	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftClaimed OnGiftClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftExpired OnGiftExpired;

	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftReturned OnGiftReturned;

	UPROPERTY(BlueprintAssignable, Category = "Gift|Events")
	FOnGiftStatusChanged OnGiftStatusChanged;

protected:
	void OnGiftTick();
	void CheckExpiredGifts();
	void AddToHistory(const FMGGift& Gift, bool bWasSent);
	void UpdateStats(const FMGGift& Gift, bool bWasSent);
	void SaveGiftData();
	void LoadGiftData();
	int32 CalculateGiftValue(const TArray<FMGGiftItem>& Items) const;

	UPROPERTY()
	TArray<FMGGift> PendingReceivedGifts;

	UPROPERTY()
	TArray<FMGGift> SentGifts;

	UPROPERTY()
	TArray<FMGGiftHistory> GiftHistory;

	UPROPERTY()
	TMap<FName, FMGGiftBundle> AvailableBundles;

	UPROPERTY()
	TMap<FName, FMGGiftItem> GiftableItems;

	UPROPERTY()
	FMGGiftSettings Settings;

	UPROPERTY()
	FMGGiftStats Stats;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	int32 GiftExpirationDays = 30;

	FTimerHandle GiftTickHandle;
};
