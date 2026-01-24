// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGEmoteSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGEmoteType : uint8
{
	None,
	Celebration,
	Taunt,
	Greeting,
	Reaction,
	Dance,
	Horn,
	Burnout,
	Drift,
	Sticker,
	Banner
};

UENUM(BlueprintType)
enum class EMGEmoteContext : uint8
{
	Any,
	PreRace,
	Victory,
	Defeat,
	Podium,
	Garage,
	Lobby,
	InRace,
	Spectating
};

UENUM(BlueprintType)
enum class EMGEmoteTrigger : uint8
{
	Manual,
	Automatic,
	WheelMenu,
	QuickSelect,
	Random
};

UENUM(BlueprintType)
enum class EMGEmoteRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Exclusive
};

USTRUCT(BlueprintType)
struct FMGEmoteDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteType EmoteType = EMGEmoteType::Celebration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteRarity Rarity = EMGEmoteRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGEmoteContext> ValidContexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInterruptible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> CharacterAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> VehicleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UParticleSystem> ParticleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> StickerTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchasePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;
};

USTRUCT(BlueprintType)
struct FMGActiveEmote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteContext Context = EMGEmoteContext::Any;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlaying = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator WorldRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FMGEmoteWheelSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteType FilterType = EMGEmoteType::None;
};

USTRUCT(BlueprintType)
struct FMGEmoteLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LoadoutName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEmoteWheelSlot> WheelSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VictoryEmote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefeatEmote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PodiumEmote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GreetingEmote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> QuickSelectEmotes;
};

USTRUCT(BlueprintType)
struct FMGEmoteUsageStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VictoryUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TauntUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;
};

USTRUCT(BlueprintType)
struct FMGEmoteCollection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UnlockedEmotes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FavoriteEmotes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FMGEmoteUsageStats> UsageStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEmotesUnlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostUsedEmote;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteStarted, FName, PlayerID, const FMGEmoteDefinition&, Emote);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteEnded, FName, PlayerID, FName, EmoteID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteInterrupted, FName, PlayerID, FName, EmoteID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmoteUnlocked, const FMGEmoteDefinition&, Emote);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteEquipped, int32, SlotIndex, FName, EmoteID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmoteWheelOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmoteWheelSelection, int32, SlotIndex);

UCLASS()
class MIDNIGHTGRIND_API UMGEmoteSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Emote Playback
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayEmote(FName EmoteID, EMGEmoteContext Context = EMGEmoteContext::Any);

	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayEmoteAtIndex(int32 WheelSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayRandomEmote(EMGEmoteType TypeFilter = EMGEmoteType::None, EMGEmoteContext ContextFilter = EMGEmoteContext::Any);

	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	void StopCurrentEmote();

	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	void InterruptEmote(FName PlayerID);

	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	bool IsPlayingEmote() const;

	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	bool IsEmoteOnCooldown(FName EmoteID) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	float GetEmoteCooldownRemaining(FName EmoteID) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	FMGActiveEmote GetActiveEmote() const { return ActiveEmote; }

	// Remote Emotes (from other players)
	UFUNCTION(BlueprintCallable, Category = "Emote|Remote")
	void PlayRemoteEmote(FName PlayerID, FName EmoteID, const FVector& Position, const FRotator& Rotation);

	UFUNCTION(BlueprintPure, Category = "Emote|Remote")
	TArray<FMGActiveEmote> GetActiveRemoteEmotes() const { return ActiveRemoteEmotes; }

	// Contextual Emotes
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayVictoryEmote();

	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayDefeatEmote();

	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayPodiumEmote(int32 Position);

	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayGreetingEmote();

	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void SetCurrentContext(EMGEmoteContext Context);

	UFUNCTION(BlueprintPure, Category = "Emote|Context")
	EMGEmoteContext GetCurrentContext() const { return CurrentContext; }

	// Emote Wheel
	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void OpenEmoteWheel();

	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void CloseEmoteWheel();

	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void SelectWheelSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Emote|Wheel")
	bool IsEmoteWheelOpen() const { return bWheelOpen; }

	UFUNCTION(BlueprintPure, Category = "Emote|Wheel")
	TArray<FMGEmoteWheelSlot> GetWheelSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void SetWheelSlot(int32 SlotIndex, FName EmoteID);

	// Quick Select
	UFUNCTION(BlueprintCallable, Category = "Emote|QuickSelect")
	void PlayQuickSelectEmote(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Emote|QuickSelect")
	void SetQuickSelectEmote(int32 Index, FName EmoteID);

	UFUNCTION(BlueprintPure, Category = "Emote|QuickSelect")
	TArray<FName> GetQuickSelectEmotes() const;

	// Loadouts
	UFUNCTION(BlueprintCallable, Category = "Emote|Loadout")
	void SetActiveLoadout(const FMGEmoteLoadout& Loadout);

	UFUNCTION(BlueprintPure, Category = "Emote|Loadout")
	FMGEmoteLoadout GetActiveLoadout() const { return ActiveLoadout; }

	UFUNCTION(BlueprintCallable, Category = "Emote|Loadout")
	void SaveLoadout(const FMGEmoteLoadout& Loadout, int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Emote|Loadout")
	FMGEmoteLoadout GetLoadout(int32 SlotIndex) const;

	// Collection
	UFUNCTION(BlueprintCallable, Category = "Emote|Collection")
	bool UnlockEmote(FName EmoteID);

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	bool IsEmoteUnlocked(FName EmoteID) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetUnlockedEmotes() const;

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesByType(EMGEmoteType Type) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesByRarity(EMGEmoteRarity Rarity) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesForContext(EMGEmoteContext Context) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	FMGEmoteCollection GetCollection() const { return Collection; }

	UFUNCTION(BlueprintCallable, Category = "Emote|Collection")
	void SetEmoteFavorite(FName EmoteID, bool bFavorite);

	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetFavoriteEmotes() const;

	// Emote Database
	UFUNCTION(BlueprintCallable, Category = "Emote|Database")
	void RegisterEmote(const FMGEmoteDefinition& Emote);

	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	FMGEmoteDefinition GetEmoteDefinition(FName EmoteID) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	TArray<FMGEmoteDefinition> GetAllEmotes() const;

	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	int32 GetTotalEmoteCount() const { return EmoteDatabase.Num(); }

	// Stats
	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	FMGEmoteUsageStats GetEmoteStats(FName EmoteID) const;

	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	FName GetMostUsedEmote() const;

	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	TArray<FMGEmoteUsageStats> GetEmoteLeaderboard(int32 MaxEntries = 10) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteStarted OnEmoteStarted;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteEnded OnEmoteEnded;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteInterrupted OnEmoteInterrupted;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteUnlocked OnEmoteUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteEquipped OnEmoteEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteWheelOpened OnEmoteWheelOpened;

	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteWheelSelection OnEmoteWheelSelection;

protected:
	void OnEmoteTick();
	void UpdateActiveEmotes(float DeltaTime);
	void TrackEmoteUsage(FName EmoteID, EMGEmoteContext Context);
	bool CanPlayEmoteInContext(const FMGEmoteDefinition& Emote, EMGEmoteContext Context) const;
	void SaveEmoteData();
	void LoadEmoteData();
	void InitializeDefaultLoadout();

	UPROPERTY()
	TMap<FName, FMGEmoteDefinition> EmoteDatabase;

	UPROPERTY()
	FMGEmoteCollection Collection;

	UPROPERTY()
	FMGEmoteLoadout ActiveLoadout;

	UPROPERTY()
	TArray<FMGEmoteLoadout> SavedLoadouts;

	UPROPERTY()
	FMGActiveEmote ActiveEmote;

	UPROPERTY()
	TArray<FMGActiveEmote> ActiveRemoteEmotes;

	UPROPERTY()
	TMap<FName, float> EmoteCooldowns;

	UPROPERTY()
	EMGEmoteContext CurrentContext = EMGEmoteContext::Any;

	UPROPERTY()
	bool bWheelOpen = false;

	UPROPERTY()
	FName LocalPlayerID;

	FTimerHandle EmoteTickHandle;
};
