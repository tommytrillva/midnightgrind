// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Companion Subsystem - Implementation

#include "Companion/MGCompanionSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGCompanionSubsystem::UMGCompanionSubsystem()
    : ActiveCompanionId(NAME_None)
{
}

void UMGCompanionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeSampleCompanions();

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGCompanionSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->TickCompanions(1.0f);
                }
            },
            1.0f,
            true
        );
    }
}

void UMGCompanionSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGCompanionSubsystem::TickCompanions(float DeltaTime)
{
    // Update ability cooldowns
    for (auto& CompanionPair : AbilityCooldowns)
    {
        for (auto& AbilityPair : CompanionPair.Value)
        {
            AbilityPair.Value = FMath::Max(0.0f, AbilityPair.Value - DeltaTime);
        }
    }

    // Update time spent with active companion
    if (ActiveCompanionId != NAME_None)
    {
        if (FMGCompanion* Companion = AllCompanions.Find(ActiveCompanionId))
        {
            Companion->Stats.TimeSpentTogether += DeltaTime / 60.0f;
            Companion->LastInteraction = FDateTime::Now();
        }
    }
}

// ===== Companion Management =====

TArray<FMGCompanion> UMGCompanionSubsystem::GetAllCompanions() const
{
    TArray<FMGCompanion> Companions;
    for (const auto& Pair : AllCompanions)
    {
        Companions.Add(Pair.Value);
    }
    return Companions;
}

TArray<FMGCompanion> UMGCompanionSubsystem::GetOwnedCompanions() const
{
    TArray<FMGCompanion> Owned;
    for (const FName& Id : OwnedCompanionIds)
    {
        if (const FMGCompanion* Companion = AllCompanions.Find(Id))
        {
            Owned.Add(*Companion);
        }
    }
    return Owned;
}

FMGCompanion UMGCompanionSubsystem::GetCompanion(FName CompanionId) const
{
    if (const FMGCompanion* Companion = AllCompanions.Find(CompanionId))
    {
        return *Companion;
    }
    return FMGCompanion();
}

FMGCompanion UMGCompanionSubsystem::GetActiveCompanion() const
{
    return GetCompanion(ActiveCompanionId);
}

bool UMGCompanionSubsystem::SetActiveCompanion(FName CompanionId)
{
    if (!OwnedCompanionIds.Contains(CompanionId))
    {
        return false;
    }

    if (ActiveCompanionId != NAME_None)
    {
        if (FMGCompanion* OldCompanion = AllCompanions.Find(ActiveCompanionId))
        {
            OldCompanion->bIsActive = false;
        }
    }

    ActiveCompanionId = CompanionId;

    if (FMGCompanion* NewCompanion = AllCompanions.Find(CompanionId))
    {
        NewCompanion->bIsActive = true;
        NewCompanion->LastInteraction = FDateTime::Now();
    }

    OnActiveCompanionChanged.Broadcast(CompanionId);
    return true;
}

bool UMGCompanionSubsystem::UnlockCompanion(FName CompanionId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    if (OwnedCompanionIds.Contains(CompanionId))
    {
        return false;
    }

    OwnedCompanionIds.Add(CompanionId);
    Companion->ObtainedDate = FDateTime::Now();

    OnCompanionUnlocked.Broadcast(*Companion);

    if (ActiveCompanionId == NAME_None)
    {
        SetActiveCompanion(CompanionId);
    }

    return true;
}

void UMGCompanionSubsystem::SetCompanionLocation(FName CompanionId, EMGCompanionLocation Location)
{
    if (FMGCompanion* Companion = AllCompanions.Find(CompanionId))
    {
        Companion->CurrentLocation = Location;
    }
}

// ===== Interaction =====

FMGCompanionInteraction UMGCompanionSubsystem::InteractWithCompanion(FName CompanionId, FName InteractionType)
{
    FMGCompanionInteraction Result;
    Result.InteractionType = InteractionType;

    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion || !OwnedCompanionIds.Contains(CompanionId))
    {
        return Result;
    }

    if (InteractionType == FName("Pet"))
    {
        Result.AffectionChange = 5;
        Result.XPGained = 10;
        Result.Response = FText::FromString(TEXT("*purrs happily*"));
        Result.AnimationToPlay = FName("Anim_Happy");
    }
    else if (InteractionType == FName("Play"))
    {
        Result.AffectionChange = 10;
        Result.XPGained = 25;
        Result.Response = FText::FromString(TEXT("*bounces excitedly*"));
        Result.AnimationToPlay = FName("Anim_Play");
    }
    else if (InteractionType == FName("Talk"))
    {
        Result.AffectionChange = 3;
        Result.XPGained = 5;
        Result.Response = TriggerDialogue(CompanionId, FName("Chat"));
        Result.AnimationToPlay = FName("Anim_Listen");
    }

    AddCompanionAffection(CompanionId, Result.AffectionChange);
    AddCompanionXP(CompanionId, Result.XPGained);

    Companion->LastInteraction = FDateTime::Now();

    if (Companion->CurrentMood != EMGCompanionMood::Happy && Result.AffectionChange > 0)
    {
        Companion->CurrentMood = EMGCompanionMood::Happy;
        OnCompanionMoodChanged.Broadcast(CompanionId, EMGCompanionMood::Happy);
    }

    OnCompanionInteraction.Broadcast(CompanionId, Result);
    return Result;
}

void UMGCompanionSubsystem::FeedCompanion(FName CompanionId, FName FoodItemId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    int32 AffectionGain = 15;
    int32 XPGain = 30;

    AddCompanionAffection(CompanionId, AffectionGain);
    AddCompanionXP(CompanionId, XPGain);

    Companion->CurrentMood = EMGCompanionMood::Happy;
    OnCompanionMoodChanged.Broadcast(CompanionId, EMGCompanionMood::Happy);
}

void UMGCompanionSubsystem::GiftCompanion(FName CompanionId, FName GiftItemId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    int32 AffectionGain = 25;
    int32 XPGain = 50;

    AddCompanionAffection(CompanionId, AffectionGain);
    AddCompanionXP(CompanionId, XPGain);

    Companion->CurrentMood = EMGCompanionMood::Excited;
    OnCompanionMoodChanged.Broadcast(CompanionId, EMGCompanionMood::Excited);
}

TArray<FName> UMGCompanionSubsystem::GetAvailableInteractions(FName CompanionId) const
{
    return { FName("Pet"), FName("Play"), FName("Talk"), FName("Feed"), FName("Gift") };
}

// ===== Customization =====

bool UMGCompanionSubsystem::RenameCompanion(FName CompanionId, const FText& NewName)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion || !OwnedCompanionIds.Contains(CompanionId))
    {
        return false;
    }

    Companion->CustomName = NewName;
    return true;
}

bool UMGCompanionSubsystem::SetCompanionSkin(FName CompanionId, FName SkinId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    if (!Companion->UnlockedSkins.Contains(SkinId) && SkinId != FName("Default"))
    {
        return false;
    }

    Companion->Appearance.SkinId = SkinId;
    return true;
}

bool UMGCompanionSubsystem::SetCompanionAccessory(FName CompanionId, FName AccessoryId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    if (AccessoryId != NAME_None && !Companion->UnlockedAccessories.Contains(AccessoryId))
    {
        return false;
    }

    Companion->Appearance.AccessoryId = AccessoryId;
    return true;
}

bool UMGCompanionSubsystem::SetCompanionColors(FName CompanionId, FLinearColor Primary, FLinearColor Secondary)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    Companion->Appearance.PrimaryColor = Primary;
    Companion->Appearance.SecondaryColor = Secondary;
    return true;
}

bool UMGCompanionSubsystem::UnlockSkin(FName CompanionId, FName SkinId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    Companion->UnlockedSkins.AddUnique(SkinId);
    return true;
}

bool UMGCompanionSubsystem::UnlockAccessory(FName CompanionId, FName AccessoryId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    Companion->UnlockedAccessories.AddUnique(AccessoryId);
    return true;
}

// ===== Progression =====

void UMGCompanionSubsystem::AddCompanionXP(FName CompanionId, int32 Amount)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    Companion->Stats.CurrentXP += Amount;
    CheckLevelUp(CompanionId);
}

void UMGCompanionSubsystem::AddCompanionAffection(FName CompanionId, int32 Amount)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    Companion->Stats.Affection = FMath::Clamp(Companion->Stats.Affection + Amount, 0, Companion->Stats.MaxAffection);
}

int32 UMGCompanionSubsystem::GetCompanionLevel(FName CompanionId) const
{
    const FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    return Companion ? Companion->Stats.Level : 0;
}

float UMGCompanionSubsystem::GetCompanionAffection(FName CompanionId) const
{
    const FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    return Companion ? Companion->Stats.GetAffectionPercent() : 0.0f;
}

// ===== Abilities =====

TArray<FMGCompanionAbility> UMGCompanionSubsystem::GetCompanionAbilities(FName CompanionId) const
{
    const FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    return Companion ? Companion->Abilities : TArray<FMGCompanionAbility>();
}

TArray<FMGCompanionAbility> UMGCompanionSubsystem::GetActiveAbilities() const
{
    TArray<FMGCompanionAbility> Active;

    if (const FMGCompanion* Companion = AllCompanions.Find(ActiveCompanionId))
    {
        for (const FMGCompanionAbility& Ability : Companion->Abilities)
        {
            if (Ability.bIsUnlocked)
            {
                Active.Add(Ability);
            }
        }
    }

    return Active;
}

bool UMGCompanionSubsystem::UseCompanionAbility(FName CompanionId, FName AbilityId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return false;
    }

    for (const FMGCompanionAbility& Ability : Companion->Abilities)
    {
        if (Ability.AbilityId == AbilityId && Ability.bIsUnlocked && !Ability.bIsPassive)
        {
            float CurrentCooldown = GetAbilityCooldownRemaining(CompanionId, AbilityId);
            if (CurrentCooldown > 0)
            {
                return false;
            }

            if (!AbilityCooldowns.Contains(CompanionId))
            {
                AbilityCooldowns.Add(CompanionId, TMap<FName, float>());
            }
            AbilityCooldowns[CompanionId].Add(AbilityId, Ability.Cooldown);

            return true;
        }
    }

    return false;
}

float UMGCompanionSubsystem::GetAbilityCooldownRemaining(FName CompanionId, FName AbilityId) const
{
    if (const TMap<FName, float>* CompanionCooldowns = AbilityCooldowns.Find(CompanionId))
    {
        if (const float* Cooldown = CompanionCooldowns->Find(AbilityId))
        {
            return *Cooldown;
        }
    }
    return 0.0f;
}

// ===== Mood =====

EMGCompanionMood UMGCompanionSubsystem::GetCompanionMood(FName CompanionId) const
{
    const FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    return Companion ? Companion->CurrentMood : EMGCompanionMood::Neutral;
}

void UMGCompanionSubsystem::UpdateMoodFromEvent(FName EventType)
{
    if (ActiveCompanionId == NAME_None)
    {
        return;
    }

    FMGCompanion* Companion = AllCompanions.Find(ActiveCompanionId);
    if (!Companion)
    {
        return;
    }

    EMGCompanionMood OldMood = Companion->CurrentMood;
    EMGCompanionMood NewMood = OldMood;

    if (EventType == FName("RaceWin"))
    {
        NewMood = EMGCompanionMood::Excited;
        AddCompanionXP(ActiveCompanionId, 100);
        Companion->Stats.WinsWitnessed++;
    }
    else if (EventType == FName("RaceLose"))
    {
        NewMood = EMGCompanionMood::Sad;
    }
    else if (EventType == FName("BigDrift"))
    {
        NewMood = EMGCompanionMood::Excited;
        AddCompanionXP(ActiveCompanionId, 25);
    }
    else if (EventType == FName("Crash"))
    {
        NewMood = EMGCompanionMood::Angry;
    }
    else if (EventType == FName("RaceStart"))
    {
        NewMood = EMGCompanionMood::Happy;
        Companion->Stats.RacesParticipated++;
    }

    if (NewMood != OldMood)
    {
        Companion->CurrentMood = NewMood;
        OnCompanionMoodChanged.Broadcast(ActiveCompanionId, NewMood);
    }
}

// ===== Dialogue =====

FText UMGCompanionSubsystem::TriggerDialogue(FName CompanionId, FName EventType)
{
    TArray<FMGCompanionDialogue> Dialogues = GetDialoguesForEvent(CompanionId, EventType);

    if (Dialogues.Num() == 0)
    {
        return FText::GetEmpty();
    }

    int32 RandomIndex = FMath::RandRange(0, Dialogues.Num() - 1);
    const FMGCompanionDialogue& Dialogue = Dialogues[RandomIndex];

    if (Dialogue.DialogueLines.Num() > 0)
    {
        int32 LineIndex = FMath::RandRange(0, Dialogue.DialogueLines.Num() - 1);
        FText SelectedLine = Dialogue.DialogueLines[LineIndex];
        OnCompanionDialogue.Broadcast(CompanionId, SelectedLine);
        return SelectedLine;
    }

    return FText::GetEmpty();
}

TArray<FMGCompanionDialogue> UMGCompanionSubsystem::GetDialoguesForEvent(FName CompanionId, FName EventType) const
{
    TArray<FMGCompanionDialogue> Result;

    if (const TArray<FMGCompanionDialogue>* Dialogues = CompanionDialogues.Find(CompanionId))
    {
        for (const FMGCompanionDialogue& Dialogue : *Dialogues)
        {
            if (Dialogue.TriggerEvent == EventType)
            {
                Result.Add(Dialogue);
            }
        }
    }

    return Result;
}

// ===== Favorites =====

void UMGCompanionSubsystem::SetFavorite(FName CompanionId, bool bFavorite)
{
    if (FMGCompanion* Companion = AllCompanions.Find(CompanionId))
    {
        Companion->bIsFavorite = bFavorite;
    }
}

TArray<FMGCompanion> UMGCompanionSubsystem::GetFavoriteCompanions() const
{
    TArray<FMGCompanion> Favorites;
    for (const FName& Id : OwnedCompanionIds)
    {
        if (const FMGCompanion* Companion = AllCompanions.Find(Id))
        {
            if (Companion->bIsFavorite)
            {
                Favorites.Add(*Companion);
            }
        }
    }
    return Favorites;
}

// ===== Protected =====

void UMGCompanionSubsystem::InitializeSampleCompanions()
{
    // Turbo the Racing Cat
    {
        FMGCompanion Turbo;
        Turbo.CompanionId = FName("companion_turbo_cat");
        Turbo.DisplayName = FText::FromString(TEXT("Turbo"));
        Turbo.Description = FText::FromString(TEXT("A speed-obsessed cat who loves the smell of burnt rubber."));
        Turbo.Personality = FText::FromString(TEXT("Energetic and always ready to race!"));
        Turbo.CompanionType = EMGCompanionType::Pet;
        Turbo.Rarity = EMGCompanionRarity::Rare;
        Turbo.CurrentMood = EMGCompanionMood::Happy;
        Turbo.CurrentLocation = EMGCompanionLocation::Dashboard;
        Turbo.UnlockedSkins = { FName("Default"), FName("Neon") };

        FMGCompanionAbility BoostAbility;
        BoostAbility.AbilityId = FName("ability_nitro_boost");
        BoostAbility.AbilityName = FText::FromString(TEXT("Turbo Boost"));
        BoostAbility.Description = FText::FromString(TEXT("Grants +5% nitro efficiency"));
        BoostAbility.AbilityType = FName("NitroBoost");
        BoostAbility.EffectValue = 5.0f;
        BoostAbility.bIsPassive = true;
        BoostAbility.bIsUnlocked = true;
        Turbo.Abilities.Add(BoostAbility);

        AllCompanions.Add(Turbo.CompanionId, Turbo);

        TArray<FMGCompanionDialogue> TurboDialogues;
        FMGCompanionDialogue RaceStart;
        RaceStart.TriggerEvent = FName("RaceStart");
        RaceStart.DialogueLines = {
            FText::FromString(TEXT("Let's go fast!")),
            FText::FromString(TEXT("Meow! Time to race!")),
            FText::FromString(TEXT("*revs engine excitedly*"))
        };
        TurboDialogues.Add(RaceStart);

        FMGCompanionDialogue Win;
        Win.TriggerEvent = FName("RaceWin");
        Win.DialogueLines = {
            FText::FromString(TEXT("We did it! Purr-fect victory!")),
            FText::FromString(TEXT("*does happy dance*"))
        };
        TurboDialogues.Add(Win);

        CompanionDialogues.Add(Turbo.CompanionId, TurboDialogues);
    }

    // Neon the Spirit Fox
    {
        FMGCompanion Neon;
        Neon.CompanionId = FName("companion_neon_fox");
        Neon.DisplayName = FText::FromString(TEXT("Neon"));
        Neon.Description = FText::FromString(TEXT("A mystical fox spirit that glows with the colors of the night."));
        Neon.Personality = FText::FromString(TEXT("Calm and wise, with a mischievous side."));
        Neon.CompanionType = EMGCompanionType::Spirit;
        Neon.Rarity = EMGCompanionRarity::Legendary;
        Neon.CurrentMood = EMGCompanionMood::Neutral;
        Neon.CurrentLocation = EMGCompanionLocation::Floating;

        FMGCompanionAbility DriftAbility;
        DriftAbility.AbilityId = FName("ability_drift_bonus");
        DriftAbility.AbilityName = FText::FromString(TEXT("Spirit Drift"));
        DriftAbility.Description = FText::FromString(TEXT("Grants +10% drift score multiplier"));
        DriftAbility.AbilityType = FName("DriftBonus");
        DriftAbility.EffectValue = 10.0f;
        DriftAbility.bIsPassive = true;
        DriftAbility.bIsUnlocked = true;
        Neon.Abilities.Add(DriftAbility);

        AllCompanions.Add(Neon.CompanionId, Neon);
    }

    // Bolt the Robot Buddy
    {
        FMGCompanion Bolt;
        Bolt.CompanionId = FName("companion_bolt_robot");
        Bolt.DisplayName = FText::FromString(TEXT("Bolt"));
        Bolt.Description = FText::FromString(TEXT("A small maintenance robot that loves analyzing racing data."));
        Bolt.Personality = FText::FromString(TEXT("Analytical and helpful, always optimizing."));
        Bolt.CompanionType = EMGCompanionType::Robot;
        Bolt.Rarity = EMGCompanionRarity::Epic;
        Bolt.CurrentMood = EMGCompanionMood::Happy;
        Bolt.CurrentLocation = EMGCompanionLocation::Dashboard;

        FMGCompanionAbility XPAbility;
        XPAbility.AbilityId = FName("ability_xp_boost");
        XPAbility.AbilityName = FText::FromString(TEXT("Data Analysis"));
        XPAbility.Description = FText::FromString(TEXT("Grants +5% XP from races"));
        XPAbility.AbilityType = FName("XPBoost");
        XPAbility.EffectValue = 5.0f;
        XPAbility.bIsPassive = true;
        XPAbility.bIsUnlocked = true;
        Bolt.Abilities.Add(XPAbility);

        AllCompanions.Add(Bolt.CompanionId, Bolt);
    }

    // Unlock starter companion
    UnlockCompanion(FName("companion_turbo_cat"));
}

void UMGCompanionSubsystem::CheckLevelUp(FName CompanionId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    while (Companion->Stats.CurrentXP >= Companion->Stats.XPToNextLevel && Companion->Stats.Level < 50)
    {
        Companion->Stats.CurrentXP -= Companion->Stats.XPToNextLevel;
        Companion->Stats.Level++;
        Companion->Stats.XPToNextLevel = CalculateXPForLevel(Companion->Stats.Level);

        for (FMGCompanionAbility& Ability : Companion->Abilities)
        {
            if (!Ability.bIsUnlocked && Ability.RequiredLevel <= Companion->Stats.Level)
            {
                Ability.bIsUnlocked = true;
                OnCompanionAbilityUnlocked.Broadcast(CompanionId, Ability);
            }
        }

        OnCompanionLevelUp.Broadcast(*Companion);
    }
}

void UMGCompanionSubsystem::UpdateMood(FName CompanionId)
{
    FMGCompanion* Companion = AllCompanions.Find(CompanionId);
    if (!Companion)
    {
        return;
    }

    FTimespan TimeSinceInteraction = FDateTime::Now() - Companion->LastInteraction;

    if (TimeSinceInteraction.GetTotalHours() > 24)
    {
        Companion->CurrentMood = EMGCompanionMood::Sad;
    }
    else if (TimeSinceInteraction.GetTotalHours() > 12)
    {
        Companion->CurrentMood = EMGCompanionMood::Tired;
    }
}

int32 UMGCompanionSubsystem::CalculateXPForLevel(int32 Level) const
{
    return 1000 + (Level * 250);
}
