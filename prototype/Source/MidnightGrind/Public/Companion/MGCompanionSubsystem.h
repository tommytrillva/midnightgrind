// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Companion Subsystem - Mascots, pets, and companion characters

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCompanionSubsystem.generated.h"

// Forward declarations
class UMGCompanionSubsystem;

/**
 * EMGCompanionType - Types of companions
 */
UENUM(BlueprintType)
enum class EMGCompanionType : uint8
{
    Mascot          UMETA(DisplayName = "Mascot"),
    Pet             UMETA(DisplayName = "Pet"),
    Robot           UMETA(DisplayName = "Robot"),
    Spirit          UMETA(DisplayName = "Spirit"),
    Hologram        UMETA(DisplayName = "Hologram"),
    Crew            UMETA(DisplayName = "Crew Member")
};

/**
 * EMGCompanionRarity - Companion rarity tiers
 */
UENUM(BlueprintType)
enum class EMGCompanionRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary"),
    Mythic          UMETA(DisplayName = "Mythic")
};

/**
 * EMGCompanionMood - Companion emotional states
 */
UENUM(BlueprintType)
enum class EMGCompanionMood : uint8
{
    Happy           UMETA(DisplayName = "Happy"),
    Excited         UMETA(DisplayName = "Excited"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Tired           UMETA(DisplayName = "Tired"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

/**
 * EMGCompanionLocation - Where companion appears
 */
UENUM(BlueprintType)
enum class EMGCompanionLocation : uint8
{
    Dashboard       UMETA(DisplayName = "Dashboard"),
    Passenger       UMETA(DisplayName = "Passenger Seat"),
    Hood            UMETA(DisplayName = "Hood Ornament"),
    Floating        UMETA(DisplayName = "Floating Nearby"),
    Garage          UMETA(DisplayName = "Garage Only"),
    Hidden          UMETA(DisplayName = "Hidden")
};

/**
 * FMGCompanionAbility - A companion's special ability
 */
USTRUCT(BlueprintType)
struct FMGCompanionAbility
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AbilityId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AbilityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPassive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;

    FMGCompanionAbility()
        : AbilityId(NAME_None)
        , AbilityType(FName("Boost"))
        , EffectValue(0.0f)
        , Cooldown(60.0f)
        , RequiredLevel(1)
        , bIsPassive(true)
        , bIsUnlocked(false)
    {}
};

/**
 * FMGCompanionStats - Companion statistics
 */
USTRUCT(BlueprintType)
struct FMGCompanionStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPToNextLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Affection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAffection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RacesParticipated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WinsWitnessed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DriftDistanceWitnessed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSpentTogether;

    FMGCompanionStats()
        : Level(1)
        , CurrentXP(0)
        , XPToNextLevel(1000)
        , Affection(50)
        , MaxAffection(100)
        , RacesParticipated(0)
        , WinsWitnessed(0)
        , DriftDistanceWitnessed(0)
        , TimeSpentTogether(0.0f)
    {}

    float GetLevelProgress() const
    {
        return XPToNextLevel > 0 ? static_cast<float>(CurrentXP) / XPToNextLevel : 0.0f;
    }

    float GetAffectionPercent() const
    {
        return MaxAffection > 0 ? static_cast<float>(Affection) / MaxAffection * 100.0f : 0.0f;
    }
};

/**
 * FMGCompanionAppearance - Visual customization
 */
USTRUCT(BlueprintType)
struct FMGCompanionAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SkinId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AccessoryId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowTrail;

    FMGCompanionAppearance()
        : SkinId(FName("Default"))
        , AccessoryId(NAME_None)
        , EffectId(NAME_None)
        , PrimaryColor(FLinearColor::White)
        , SecondaryColor(FLinearColor::Gray)
        , Scale(1.0f)
        , bShowTrail(true)
    {}
};

/**
 * FMGCompanion - A companion character
 */
USTRUCT(BlueprintType)
struct FMGCompanion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CompanionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText CustomName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCompanionType CompanionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCompanionRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCompanionMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCompanionLocation CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGCompanionStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGCompanionAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGCompanionAbility> Abilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedSkins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedAccessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ObtainedDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFavorite;

    FMGCompanion()
        : CompanionId(NAME_None)
        , CompanionType(EMGCompanionType::Mascot)
        , Rarity(EMGCompanionRarity::Common)
        , CurrentMood(EMGCompanionMood::Happy)
        , CurrentLocation(EMGCompanionLocation::Dashboard)
        , bIsActive(false)
        , bIsFavorite(false)
    {}
};

/**
 * FMGCompanionInteraction - An interaction with a companion
 */
USTRUCT(BlueprintType)
struct FMGCompanionInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AffectionChange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPGained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Response;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AnimationToPlay;

    FMGCompanionInteraction()
        : InteractionType(FName("Pet"))
        , AffectionChange(5)
        , XPGained(10)
        , AnimationToPlay(NAME_None)
    {}
};

/**
 * FMGCompanionDialogue - Companion voice lines/dialogue
 */
USTRUCT(BlueprintType)
struct FMGCompanionDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName VoiceAssetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Probability;

    FMGCompanionDialogue()
        : TriggerEvent(FName("RaceStart"))
        , VoiceAssetId(NAME_None)
        , Probability(1.0f)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCompanionUnlocked, const FMGCompanion&, Companion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCompanionLevelUp, const FMGCompanion&, Companion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCompanionMoodChanged, FName, CompanionId, EMGCompanionMood, NewMood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCompanionAbilityUnlocked, FName, CompanionId, const FMGCompanionAbility&, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCompanionInteraction, FName, CompanionId, const FMGCompanionInteraction&, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCompanionDialogue, FName, CompanionId, const FText&, Dialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnActiveCompanionChanged, FName, CompanionId);

/**
 * UMGCompanionSubsystem
 *
 * Manages companion characters for Midnight Grind.
 * Features include:
 * - Collectible companions with different types
 * - Companion leveling and affection
 * - Special abilities and bonuses
 * - Customization options
 * - Mood system based on gameplay
 * - Dynamic dialogue/reactions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCompanionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGCompanionSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void TickCompanions(float DeltaTime);

    // ===== Companion Management =====

    UFUNCTION(BlueprintPure, Category = "Companion|Management")
    TArray<FMGCompanion> GetAllCompanions() const;

    UFUNCTION(BlueprintPure, Category = "Companion|Management")
    TArray<FMGCompanion> GetOwnedCompanions() const;

    UFUNCTION(BlueprintPure, Category = "Companion|Management")
    FMGCompanion GetCompanion(FName CompanionId) const;

    UFUNCTION(BlueprintPure, Category = "Companion|Management")
    FMGCompanion GetActiveCompanion() const;

    UFUNCTION(BlueprintCallable, Category = "Companion|Management")
    bool SetActiveCompanion(FName CompanionId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Management")
    bool UnlockCompanion(FName CompanionId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Management")
    void SetCompanionLocation(FName CompanionId, EMGCompanionLocation Location);

    // ===== Interaction =====

    UFUNCTION(BlueprintCallable, Category = "Companion|Interact")
    FMGCompanionInteraction InteractWithCompanion(FName CompanionId, FName InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Companion|Interact")
    void FeedCompanion(FName CompanionId, FName FoodItemId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Interact")
    void GiftCompanion(FName CompanionId, FName GiftItemId);

    UFUNCTION(BlueprintPure, Category = "Companion|Interact")
    TArray<FName> GetAvailableInteractions(FName CompanionId) const;

    // ===== Customization =====

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool RenameCompanion(FName CompanionId, const FText& NewName);

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool SetCompanionSkin(FName CompanionId, FName SkinId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool SetCompanionAccessory(FName CompanionId, FName AccessoryId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool SetCompanionColors(FName CompanionId, FLinearColor Primary, FLinearColor Secondary);

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool UnlockSkin(FName CompanionId, FName SkinId);

    UFUNCTION(BlueprintCallable, Category = "Companion|Customize")
    bool UnlockAccessory(FName CompanionId, FName AccessoryId);

    // ===== Progression =====

    UFUNCTION(BlueprintCallable, Category = "Companion|Progress")
    void AddCompanionXP(FName CompanionId, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Companion|Progress")
    void AddCompanionAffection(FName CompanionId, int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Companion|Progress")
    int32 GetCompanionLevel(FName CompanionId) const;

    UFUNCTION(BlueprintPure, Category = "Companion|Progress")
    float GetCompanionAffection(FName CompanionId) const;

    // ===== Abilities =====

    UFUNCTION(BlueprintPure, Category = "Companion|Abilities")
    TArray<FMGCompanionAbility> GetCompanionAbilities(FName CompanionId) const;

    UFUNCTION(BlueprintPure, Category = "Companion|Abilities")
    TArray<FMGCompanionAbility> GetActiveAbilities() const;

    UFUNCTION(BlueprintCallable, Category = "Companion|Abilities")
    bool UseCompanionAbility(FName CompanionId, FName AbilityId);

    UFUNCTION(BlueprintPure, Category = "Companion|Abilities")
    float GetAbilityCooldownRemaining(FName CompanionId, FName AbilityId) const;

    // ===== Mood =====

    UFUNCTION(BlueprintPure, Category = "Companion|Mood")
    EMGCompanionMood GetCompanionMood(FName CompanionId) const;

    UFUNCTION(BlueprintCallable, Category = "Companion|Mood")
    void UpdateMoodFromEvent(FName EventType);

    // ===== Dialogue =====

    UFUNCTION(BlueprintCallable, Category = "Companion|Dialogue")
    FText TriggerDialogue(FName CompanionId, FName EventType);

    UFUNCTION(BlueprintPure, Category = "Companion|Dialogue")
    TArray<FMGCompanionDialogue> GetDialoguesForEvent(FName CompanionId, FName EventType) const;

    // ===== Favorites =====

    UFUNCTION(BlueprintCallable, Category = "Companion|Favorite")
    void SetFavorite(FName CompanionId, bool bFavorite);

    UFUNCTION(BlueprintPure, Category = "Companion|Favorite")
    TArray<FMGCompanion> GetFavoriteCompanions() const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionUnlocked OnCompanionUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionLevelUp OnCompanionLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionMoodChanged OnCompanionMoodChanged;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionAbilityUnlocked OnCompanionAbilityUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionInteraction OnCompanionInteraction;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnCompanionDialogue OnCompanionDialogue;

    UPROPERTY(BlueprintAssignable, Category = "Companion|Events")
    FMGOnActiveCompanionChanged OnActiveCompanionChanged;

protected:
    void InitializeSampleCompanions();
    void CheckLevelUp(FName CompanionId);
    void UpdateMood(FName CompanionId);
    int32 CalculateXPForLevel(int32 Level) const;

private:
    UPROPERTY()
    TMap<FName, FMGCompanion> AllCompanions;

    UPROPERTY()
    TArray<FName> OwnedCompanionIds;

    UPROPERTY()
    FName ActiveCompanionId;

    UPROPERTY()
    TMap<FName, TMap<FName, float>> AbilityCooldowns;

    UPROPERTY()
    TMap<FName, TArray<FMGCompanionDialogue>> CompanionDialogues;

    FTimerHandle TickTimerHandle;
};
