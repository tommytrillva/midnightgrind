// Copyright Midnight Grind. All Rights Reserved.

#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "Kismet/GameplayStatics.h"

UMGRaceDirectorSubsystem::UMGRaceDirectorSubsystem()
    : bRaceActive(false)
    , TotalLaps(3)
    , TrackLength(5000.0f)
    , RaceTime(0.0f)
    , CurrentPhase(EMGRacePhase::PreRace)
    , DirectorStyle(EMGDirectorStyle::Balanced)
    , TensionScore(0.0f)
    , TensionLevel(EMGRaceTension::Calm)
    , CurrentMoment(EMGDramaticMoment::None)
    , LeadChanges(0)
    , LastDramaTime(0.0f)
{
}

void UMGRaceDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeDifficultyPresets();

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Subsystem initialized"));
}

void UMGRaceDirectorSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UMGRaceDirectorSubsystem::InitializeRace(int32 InTotalLaps, float InTrackLength)
{
    TotalLaps = FMath::Max(1, InTotalLaps);
    TrackLength = FMath::Max(100.0f, InTrackLength);

    ResetRace();

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Initialized race - %d laps, %.0f meters"), TotalLaps, TrackLength);
}

void UMGRaceDirectorSubsystem::StartRace()
{
    bRaceActive = true;
    RaceTime = 0.0f;

    SetRacePhase(EMGRacePhase::Start);

    for (auto& Pair : RacerStates)
    {
        Pair.Value.bIsActive = true;
        Pair.Value.CurrentLap = 1;
        Pair.Value.RaceProgress = 0.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Race started with %d racers"), RacerStates.Num());
}

void UMGRaceDirectorSubsystem::EndRace()
{
    bRaceActive = false;

    SetRacePhase(EMGRacePhase::Finished);

    // Calculate final statistics
    UpdateStatistics();
    RaceStats.RaceTime = RaceTime;

    if (FinishOrder.Num() >= 2)
    {
        FMGRacerState* First = RacerStates.Find(FinishOrder[0]);
        FMGRacerState* Second = RacerStates.Find(FinishOrder[1]);

        if (First && Second)
        {
            RaceStats.WinningMargin = Second->FinishTime - First->FinishTime;
        }
    }

    OnRaceFinished.Broadcast(RaceStats);

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Race finished - %d lead changes, %.1f tension"),
           LeadChanges, TensionScore);
}

void UMGRaceDirectorSubsystem::ResetRace()
{
    RacerStates.Empty();
    FinishOrder.Empty();
    DramaticEvents.Empty();

    PlayerRacerId = FGuid();
    CurrentLeaderId = FGuid();

    RaceTime = 0.0f;
    TensionScore = 0.0f;
    LeadChanges = 0;
    LastDramaTime = 0.0f;

    CurrentPhase = EMGRacePhase::PreRace;
    TensionLevel = EMGRaceTension::Calm;
    CurrentMoment = EMGDramaticMoment::None;

    RaceStats = FMGRaceStatistics();

    bRaceActive = false;
}

bool UMGRaceDirectorSubsystem::IsRaceActive() const
{
    return bRaceActive;
}

FGuid UMGRaceDirectorSubsystem::RegisterRacer(const FString& RacerName, bool bIsPlayer, int32 StartPosition)
{
    FGuid RacerId = FGuid::NewGuid();

    FMGRacerState State;
    State.RacerId = RacerId;
    State.RacerName = RacerName;
    State.bIsPlayer = bIsPlayer;
    State.StartingPosition = StartPosition;
    State.CurrentPosition = StartPosition;
    State.BestPosition = StartPosition;
    State.SkillRating = bIsPlayer ? 0.5f : FMath::FRandRange(0.3f, 0.9f);
    State.AggresionLevel = bIsPlayer ? 0.5f : FMath::FRandRange(0.2f, 0.8f);

    // Apply difficulty modifiers to AI
    if (!bIsPlayer)
    {
        State.PerformanceRating = DifficultyConfig.SpeedMultiplier;
        State.SkillRating *= DifficultyConfig.RacingLineOptimality;
    }

    RacerStates.Add(RacerId, State);

    if (bIsPlayer)
    {
        PlayerRacerId = RacerId;
    }

    RaceStats.TotalRacers++;
    RaceStats.ActiveRacers++;

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Registered racer %s (P%d, Player: %d)"),
           *RacerName, StartPosition, bIsPlayer);

    return RacerId;
}

void UMGRaceDirectorSubsystem::UnregisterRacer(const FGuid& RacerId)
{
    if (RacerStates.Remove(RacerId) > 0)
    {
        if (RacerId == PlayerRacerId)
        {
            PlayerRacerId = FGuid();
        }
    }
}

void UMGRaceDirectorSubsystem::UpdateRacerState(const FGuid& RacerId, int32 Position, float Speed, float Progress)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        int32 OldPosition = State->CurrentPosition;

        State->CurrentPosition = Position;
        State->CurrentSpeed = Speed;
        State->RaceProgress = Progress;

        // Track position changes
        if (OldPosition != Position)
        {
            State->PositionChanges++;
            RaceStats.TotalPositionChanges++;

            if (Position < State->BestPosition)
            {
                State->BestPosition = Position;
            }
            if (Position > State->WorstPosition)
            {
                State->WorstPosition = Position;
            }

            // Check for lead change
            if (Position == 1 && OldPosition != 1)
            {
                FGuid OldLeader = CurrentLeaderId;
                CurrentLeaderId = RacerId;
                LeadChanges++;
                RaceStats.TotalLeadChanges++;

                OnLeadChange.Broadcast(RacerId, LeadChanges);

                if (DramaConfig.bEnableDramaticMoments)
                {
                    TriggerDramaticMoment(EMGDramaticMoment::LeadChange, RacerId, OldLeader);
                }
            }

            OnPositionChange.Broadcast(RacerId, Position);
        }

        // Update max speed tracking
        if (Speed > State->MaxSpeed)
        {
            State->MaxSpeed = Speed;
        }
    }
}

void UMGRaceDirectorSubsystem::SetRacerLap(const FGuid& RacerId, int32 Lap)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        State->CurrentLap = Lap;

        // Check for final lap
        if (Lap == TotalLaps && CurrentPhase != EMGRacePhase::FinalLap)
        {
            bool bAnyOnFinalLap = false;
            for (const auto& Pair : RacerStates)
            {
                if (Pair.Value.CurrentLap >= TotalLaps && !Pair.Value.bHasFinished)
                {
                    bAnyOnFinalLap = true;
                    break;
                }
            }

            if (bAnyOnFinalLap)
            {
                SetRacePhase(EMGRacePhase::FinalLap);
            }
        }
    }
}

void UMGRaceDirectorSubsystem::SetRacerFinished(const FGuid& RacerId, float FinishTime)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        State->bHasFinished = true;
        State->bIsActive = false;
        State->FinishTime = FinishTime;

        FinishOrder.Add(RacerId);
        RaceStats.FinishedRacers++;
        RaceStats.ActiveRacers--;

        int32 FinishPosition = FinishOrder.Num();

        OnRacerFinished.Broadcast(RacerId, FinishPosition);

        // Check for photo finish
        if (FinishOrder.Num() >= 2)
        {
            FMGRacerState* Previous = RacerStates.Find(FinishOrder[FinishOrder.Num() - 2]);
            if (Previous)
            {
                float Gap = FinishTime - Previous->FinishTime;
                if (Gap < DramaConfig.PhotoFinishWindow)
                {
                    TriggerDramaticMoment(EMGDramaticMoment::PhotoFinish, RacerId, Previous->RacerId);
                    RaceStats.ClosestGap = FMath::Min(RaceStats.ClosestGap, Gap);
                }
            }
        }

        // Check if race should end
        bool bAllFinishedOrWrecked = true;
        for (const auto& Pair : RacerStates)
        {
            if (Pair.Value.bIsActive && !Pair.Value.bHasFinished)
            {
                bAllFinishedOrWrecked = false;
                break;
            }
        }

        if (bAllFinishedOrWrecked)
        {
            EndRace();
        }

        UE_LOG(LogTemp, Log, TEXT("RaceDirector: %s finished in P%d (%.3fs)"),
               *State->RacerName, FinishPosition, FinishTime);
    }
}

void UMGRaceDirectorSubsystem::SetRacerWrecked(const FGuid& RacerId)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        State->bIsActive = false;
        State->TimesWrecked++;
        RaceStats.WreckedRacers++;
        RaceStats.ActiveRacers--;
    }
}

FMGRacerState UMGRaceDirectorSubsystem::GetRacerState(const FGuid& RacerId) const
{
    if (const FMGRacerState* State = RacerStates.Find(RacerId))
    {
        return *State;
    }
    return FMGRacerState();
}

TArray<FMGRacerState> UMGRaceDirectorSubsystem::GetAllRacerStates() const
{
    TArray<FMGRacerState> States;
    RacerStates.GenerateValueArray(States);

    // Sort by position
    States.Sort([](const FMGRacerState& A, const FMGRacerState& B)
    {
        return A.CurrentPosition < B.CurrentPosition;
    });

    return States;
}

FMGRacerState UMGRaceDirectorSubsystem::GetPlayerState() const
{
    return GetRacerState(PlayerRacerId);
}

FMGRacerState UMGRaceDirectorSubsystem::GetLeaderState() const
{
    for (const auto& Pair : RacerStates)
    {
        if (Pair.Value.CurrentPosition == 1)
        {
            return Pair.Value;
        }
    }
    return FMGRacerState();
}

void UMGRaceDirectorSubsystem::SetDirectorStyle(EMGDirectorStyle Style)
{
    DirectorStyle = Style;
    ApplyDirectorStyle();

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Director style set to %d"), (int32)Style);
}

void UMGRaceDirectorSubsystem::SetRubberBandConfig(const FMGRubberBandConfig& Config)
{
    RubberBandConfig = Config;
}

void UMGRaceDirectorSubsystem::SetDramaConfig(const FMGDramaConfig& Config)
{
    DramaConfig = Config;
}

void UMGRaceDirectorSubsystem::SetPacingConfig(const FMGRacePacingConfig& Config)
{
    PacingConfig = Config;
}

void UMGRaceDirectorSubsystem::SetAIDifficulty(const FMGAIDifficultyConfig& Config)
{
    DifficultyConfig = Config;

    // Apply to existing AI racers
    for (auto& Pair : RacerStates)
    {
        if (!Pair.Value.bIsPlayer)
        {
            Pair.Value.PerformanceRating = Config.SpeedMultiplier;
        }
    }
}

EMGDirectorStyle UMGRaceDirectorSubsystem::GetDirectorStyle() const
{
    return DirectorStyle;
}

FMGRubberBandConfig UMGRaceDirectorSubsystem::GetRubberBandConfig() const
{
    return RubberBandConfig;
}

FMGAIDifficultyConfig UMGRaceDirectorSubsystem::GetAIDifficulty() const
{
    return DifficultyConfig;
}

void UMGRaceDirectorSubsystem::UpdateDirector(float DeltaTime)
{
    if (!bRaceActive)
    {
        return;
    }

    RaceTime += DeltaTime;

    UpdateRacePhase();
    CalculateGaps();
    UpdateTension();
    UpdateRubberBanding();
    UpdateAIBehaviors();
    CheckDramaticMoments();
    UpdateStatistics();
}

FMGDirectorState UMGRaceDirectorSubsystem::GetDirectorState() const
{
    FMGDirectorState State;
    State.CurrentPhase = CurrentPhase;
    State.TensionLevel = TensionLevel;
    State.CurrentMoment = CurrentMoment;
    State.TensionScore = TensionScore;
    State.LeadChanges = LeadChanges;
    State.AverageGap = RaceStats.ClosestGap;

    // Calculate race progress
    float TotalProgress = 0.0f;
    int32 ActiveCount = 0;

    for (const auto& Pair : RacerStates)
    {
        if (Pair.Value.bIsActive)
        {
            float Progress = ((Pair.Value.CurrentLap - 1) + Pair.Value.RaceProgress) / TotalLaps;
            TotalProgress += Progress;
            ActiveCount++;
        }
    }

    State.RaceProgress = ActiveCount > 0 ? TotalProgress / ActiveCount : 0.0f;

    // Player performance relative to expected position
    if (const FMGRacerState* PlayerState = RacerStates.Find(PlayerRacerId))
    {
        float ExpectedPosition = (PlayerState->StartingPosition + RacerStates.Num()) / 2.0f;
        State.PlayerPerformance = ExpectedPosition / FMath::Max(1, PlayerState->CurrentPosition);
    }

    State.bIsCloseRace = RaceStats.ClosestGap < DramaConfig.CloseRaceThreshold;
    State.bPhotoFinishPossible = CurrentPhase == EMGRacePhase::FinalLap && State.bIsCloseRace;

    return State;
}

EMGRacePhase UMGRaceDirectorSubsystem::GetCurrentPhase() const
{
    return CurrentPhase;
}

EMGRaceTension UMGRaceDirectorSubsystem::GetTensionLevel() const
{
    return TensionLevel;
}

float UMGRaceDirectorSubsystem::GetRaceProgress() const
{
    return GetDirectorState().RaceProgress;
}

float UMGRaceDirectorSubsystem::GetTensionScore() const
{
    return TensionScore;
}

int32 UMGRaceDirectorSubsystem::GetLeadChanges() const
{
    return LeadChanges;
}

float UMGRaceDirectorSubsystem::GetSpeedModifier(const FGuid& RacerId) const
{
    if (const FMGRacerState* State = RacerStates.Find(RacerId))
    {
        return State->SpeedModifier;
    }
    return 1.0f;
}

float UMGRaceDirectorSubsystem::GetHandlingModifier(const FGuid& RacerId) const
{
    if (const FMGRacerState* State = RacerStates.Find(RacerId))
    {
        return State->HandlingModifier;
    }
    return 1.0f;
}

float UMGRaceDirectorSubsystem::GetNitroRechargeModifier(const FGuid& RacerId) const
{
    if (const FMGRacerState* State = RacerStates.Find(RacerId))
    {
        // Give nitro bonus to trailing racers
        if (State->CurrentPosition > RacerStates.Num() / 2)
        {
            return RubberBandConfig.NitroRechargeBonus;
        }
    }
    return 1.0f;
}

EMGAIBehaviorState UMGRaceDirectorSubsystem::GetRecommendedBehavior(const FGuid& RacerId) const
{
    if (const FMGRacerState* State = RacerStates.Find(RacerId))
    {
        return State->BehaviorState;
    }
    return EMGAIBehaviorState::Normal;
}

void UMGRaceDirectorSubsystem::RequestMistake(const FGuid& RacerId, float Severity)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        if (!State->bIsPlayer)
        {
            // Check if mistake should happen based on difficulty
            float MistakeChance = DifficultyConfig.MistakeFrequency * Severity;

            if (FMath::FRand() < MistakeChance)
            {
                State->BehaviorState = EMGAIBehaviorState::Mistake;

                // Temporary speed reduction
                State->SpeedModifier *= (1.0f - (0.1f * Severity));
            }
        }
    }
}

void UMGRaceDirectorSubsystem::SetRacerAggression(const FGuid& RacerId, float Aggression)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        State->AggresionLevel = FMath::Clamp(Aggression, 0.0f, 1.0f);
    }
}

void UMGRaceDirectorSubsystem::DesignateRival(const FGuid& RacerId, bool bIsRival)
{
    if (FMGRacerState* State = RacerStates.Find(RacerId))
    {
        State->bIsRival = bIsRival;

        if (bIsRival)
        {
            // Rivals are more aggressive
            State->AggresionLevel = FMath::Max(State->AggresionLevel, 0.7f);
        }
    }
}

void UMGRaceDirectorSubsystem::RecordTakedown(const FGuid& AttackerId, const FGuid& VictimId)
{
    if (FMGRacerState* Attacker = RacerStates.Find(AttackerId))
    {
        Attacker->Takedowns++;
    }

    if (FMGRacerState* Victim = RacerStates.Find(VictimId))
    {
        Victim->TimesWrecked++;
    }

    RaceStats.TotalTakedowns++;
}

void UMGRaceDirectorSubsystem::RecordNearMiss(const FGuid& RacerId)
{
    // Near misses increase tension
    TensionScore += 0.05f;
}

void UMGRaceDirectorSubsystem::RecordPerfectLap(const FGuid& RacerId, float LapTime)
{
    if (LapTime < RaceStats.FastestLap)
    {
        RaceStats.FastestLap = LapTime;
    }

    if (LapTime > RaceStats.SlowestLap)
    {
        RaceStats.SlowestLap = LapTime;
    }

    TriggerDramaticMoment(EMGDramaticMoment::PerfectLap, RacerId);
}

TArray<FMGRaceEvent> UMGRaceDirectorSubsystem::GetDramaticEvents() const
{
    return DramaticEvents;
}

EMGDramaticMoment UMGRaceDirectorSubsystem::GetCurrentMoment() const
{
    return CurrentMoment;
}

FMGRaceStatistics UMGRaceDirectorSubsystem::GetRaceStatistics() const
{
    return RaceStats;
}

TArray<FGuid> UMGRaceDirectorSubsystem::GetFinishOrder() const
{
    return FinishOrder;
}

void UMGRaceDirectorSubsystem::SetDifficultyPreset(int32 Level)
{
    Level = FMath::Clamp(Level, 0, DifficultyPresets.Num() - 1);

    if (DifficultyPresets.IsValidIndex(Level))
    {
        SetAIDifficulty(DifficultyPresets[Level]);
        SetRubberBandConfig(FMGRubberBandConfig());
        RubberBandConfig.Level = DifficultyPresets[Level].RubberBandLevel;
    }
}

FMGAIDifficultyConfig UMGRaceDirectorSubsystem::GetDifficultyPreset(int32 Level) const
{
    Level = FMath::Clamp(Level, 0, DifficultyPresets.Num() - 1);

    if (DifficultyPresets.IsValidIndex(Level))
    {
        return DifficultyPresets[Level];
    }
    return FMGAIDifficultyConfig();
}

void UMGRaceDirectorSubsystem::UpdateRacePhase()
{
    float Progress = GetRaceProgress();

    EMGRacePhase NewPhase = CurrentPhase;

    if (RaceTime < PacingConfig.StartChaosWindow)
    {
        NewPhase = EMGRacePhase::Start;
    }
    else if (Progress < PacingConfig.EarlyRacePercent)
    {
        NewPhase = EMGRacePhase::EarlyRace;
    }
    else if (Progress < PacingConfig.MidRacePercent)
    {
        NewPhase = EMGRacePhase::MidRace;
    }
    else if (Progress < PacingConfig.LateRacePercent)
    {
        NewPhase = EMGRacePhase::LateRace;
    }
    else
    {
        // Check if anyone is on final lap
        bool bFinalLap = false;
        for (const auto& Pair : RacerStates)
        {
            if (Pair.Value.CurrentLap >= TotalLaps && !Pair.Value.bHasFinished)
            {
                bFinalLap = true;
                break;
            }
        }

        if (bFinalLap)
        {
            // Check for photo finish conditions
            if (TensionScore > 0.7f && RaceStats.ClosestGap < DramaConfig.PhotoFinishWindow * 2.0f)
            {
                NewPhase = EMGRacePhase::PhotoFinish;
            }
            else
            {
                NewPhase = EMGRacePhase::FinalLap;
            }
        }
    }

    if (NewPhase != CurrentPhase)
    {
        SetRacePhase(NewPhase);
    }
}

void UMGRaceDirectorSubsystem::UpdateTension()
{
    // Calculate tension based on multiple factors
    float NewTension = 0.0f;

    // Gap closeness contributes to tension
    if (RaceStats.ClosestGap < DramaConfig.CloseRaceThreshold)
    {
        NewTension += (DramaConfig.CloseRaceThreshold - RaceStats.ClosestGap) / DramaConfig.CloseRaceThreshold * 0.4f;
    }

    // Lead changes contribute to tension
    float LeadChangeFactor = FMath::Min(LeadChanges / 10.0f, 1.0f) * 0.2f;
    NewTension += LeadChangeFactor;

    // Race phase contributes to tension
    switch (CurrentPhase)
    {
        case EMGRacePhase::LateRace:
            NewTension += 0.2f;
            break;
        case EMGRacePhase::FinalLap:
            NewTension += 0.3f;
            break;
        case EMGRacePhase::PhotoFinish:
            NewTension += 0.5f;
            break;
        default:
            break;
    }

    // Player position affects tension
    if (const FMGRacerState* PlayerState = RacerStates.Find(PlayerRacerId))
    {
        // Tension is higher when player is in contention
        if (PlayerState->CurrentPosition <= 3)
        {
            NewTension += 0.2f;
        }

        // Comeback attempts are exciting
        if (PlayerState->CurrentPosition > PlayerState->StartingPosition + DramaConfig.ComebackThreshold)
        {
            NewTension += 0.15f;
        }
    }

    // Smooth tension changes
    UWorld* World = GetWorld();
    float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
    TensionScore = FMath::FInterpTo(TensionScore, FMath::Clamp(NewTension, 0.0f, 1.0f),
                                     DeltaSeconds, DramaConfig.TensionBuildupRate * 10.0f);

    // Update tension level category
    EMGRaceTension NewLevel;
    if (TensionScore < 0.2f)
    {
        NewLevel = EMGRaceTension::Calm;
    }
    else if (TensionScore < 0.4f)
    {
        NewLevel = EMGRaceTension::Mild;
    }
    else if (TensionScore < 0.6f)
    {
        NewLevel = EMGRaceTension::Moderate;
    }
    else if (TensionScore < 0.8f)
    {
        NewLevel = EMGRaceTension::Intense;
    }
    else
    {
        NewLevel = EMGRaceTension::Extreme;
    }

    if (NewLevel != TensionLevel)
    {
        SetTensionLevel(NewLevel);
    }
}

void UMGRaceDirectorSubsystem::UpdateRubberBanding()
{
    if (RubberBandConfig.Level == EMGRubberBandLevel::None)
    {
        return;
    }

    for (auto& Pair : RacerStates)
    {
        FMGRacerState& State = Pair.Value;

        if (!State.bIsActive || State.bHasFinished)
        {
            continue;
        }

        // Skip if this type of racer shouldn't be affected
        if (State.bIsPlayer && !RubberBandConfig.bAffectsPlayer)
        {
            continue;
        }
        if (!State.bIsPlayer && !RubberBandConfig.bAffectsAI)
        {
            continue;
        }

        float Modifier = CalculateRubberBandModifier(State);

        // Smooth modifier changes
        UWorld* ModWorld = GetWorld();
        float ModDeltaSeconds = ModWorld ? ModWorld->GetDeltaSeconds() : 0.016f;
        State.SpeedModifier = FMath::FInterpTo(State.SpeedModifier, Modifier,
                                                ModDeltaSeconds,
                                                1.0f / RubberBandConfig.RampUpTime);

        // Apply handling modifier if trailing
        if (State.CurrentPosition > RacerStates.Num() / 2)
        {
            State.HandlingModifier = RubberBandConfig.HandlingBoost;
        }
        else
        {
            State.HandlingModifier = 1.0f;
        }

        // Update adjustment type
        if (Modifier > 1.0f)
        {
            State.CurrentAdjustment = EMGPositionAdjustment::SpeedBoost;
        }
        else if (Modifier < 1.0f)
        {
            State.CurrentAdjustment = EMGPositionAdjustment::SpeedReduction;
        }
        else
        {
            State.CurrentAdjustment = EMGPositionAdjustment::None;
        }

        if (State.CurrentAdjustment != EMGPositionAdjustment::None)
        {
            OnRubberBandApplied.Broadcast(Pair.Key, State.SpeedModifier);
        }
    }
}

void UMGRaceDirectorSubsystem::UpdateAIBehaviors()
{
    for (auto& Pair : RacerStates)
    {
        FMGRacerState& State = Pair.Value;

        if (State.bIsPlayer || !State.bIsActive || State.bHasFinished)
        {
            continue;
        }

        State.BehaviorState = DetermineAIBehavior(State);
    }
}

void UMGRaceDirectorSubsystem::CheckDramaticMoments()
{
    if (!DramaConfig.bEnableDramaticMoments)
    {
        return;
    }

    // Cooldown check
    if (RaceTime - LastDramaTime < DramaConfig.MinDramaCooldown)
    {
        return;
    }

    // Check for comeback
    if (const FMGRacerState* PlayerState = RacerStates.Find(PlayerRacerId))
    {
        int32 PositionsGained = PlayerState->StartingPosition - PlayerState->CurrentPosition;

        if (PositionsGained >= DramaConfig.ComebackThreshold && PlayerState->CurrentPosition <= 3)
        {
            TriggerDramaticMoment(EMGDramaticMoment::Comeback, PlayerRacerId);
        }

        // Check for underdog scenario
        if (DramaConfig.bEnableUnderdogBonus)
        {
            if (PlayerState->StartingPosition >= RacerStates.Num() - 2 &&
                PlayerState->CurrentPosition == 1)
            {
                TriggerDramaticMoment(EMGDramaticMoment::Underdog, PlayerRacerId);
            }
        }

        // Check for dominance
        if (PlayerState->CurrentPosition == 1 &&
            PlayerState->DistanceToBehind > RubberBandConfig.ActivationDistance * 2.0f &&
            CurrentPhase == EMGRacePhase::LateRace)
        {
            TriggerDramaticMoment(EMGDramaticMoment::Dominance, PlayerRacerId);
        }
    }

    // Check for close race
    if (RaceStats.ClosestGap < DramaConfig.CloseRaceThreshold &&
        CurrentPhase >= EMGRacePhase::MidRace &&
        CurrentMoment != EMGDramaticMoment::CloseRace)
    {
        TriggerDramaticMoment(EMGDramaticMoment::CloseRace, CurrentLeaderId);
    }

    // Check for rivalry moments
    if (DramaConfig.bEnableRivalrySystem)
    {
        if (const FMGRacerState* PlayerState = RacerStates.Find(PlayerRacerId))
        {
            for (const auto& Pair : RacerStates)
            {
                if (Pair.Value.bIsRival &&
                    FMath::Abs(Pair.Value.CurrentPosition - PlayerState->CurrentPosition) <= 1)
                {
                    TriggerDramaticMoment(EMGDramaticMoment::Rivalry, PlayerRacerId, Pair.Key);
                    break;
                }
            }
        }
    }
}

void UMGRaceDirectorSubsystem::CalculateGaps()
{
    TArray<FMGRacerState*> SortedRacers;

    for (auto& Pair : RacerStates)
    {
        if (Pair.Value.bIsActive && !Pair.Value.bHasFinished)
        {
            SortedRacers.Add(&Pair.Value);
        }
    }

    // Sort by position
    SortedRacers.Sort([](const FMGRacerState& A, const FMGRacerState& B)
    {
        return A.CurrentPosition < B.CurrentPosition;
    });

    // Calculate gaps
    float SmallestGap = 999999.0f;
    float TotalGap = 0.0f;

    for (int32 i = 0; i < SortedRacers.Num(); ++i)
    {
        FMGRacerState* Current = SortedRacers[i];

        // Distance to leader
        if (i > 0)
        {
            Current->DistanceToLeader = SortedRacers[0]->RaceProgress - Current->RaceProgress;
            Current->DistanceToLeader *= TrackLength;
        }
        else
        {
            Current->DistanceToLeader = 0.0f;
        }

        // Distance to car ahead
        if (i > 0)
        {
            FMGRacerState* Ahead = SortedRacers[i - 1];
            float Gap = (Ahead->RaceProgress - Current->RaceProgress) * TrackLength;
            Current->DistanceToAhead = Gap;

            TotalGap += Gap;

            if (Gap < SmallestGap && Gap > 0.0f)
            {
                SmallestGap = Gap;
            }
        }
        else
        {
            Current->DistanceToAhead = 0.0f;
        }

        // Distance to car behind
        if (i < SortedRacers.Num() - 1)
        {
            FMGRacerState* Behind = SortedRacers[i + 1];
            Current->DistanceToBehind = (Current->RaceProgress - Behind->RaceProgress) * TrackLength;
        }
        else
        {
            Current->DistanceToBehind = 999999.0f;
        }
    }

    if (SmallestGap < 999999.0f)
    {
        RaceStats.ClosestGap = SmallestGap;
    }

    if (SortedRacers.Num() > 1)
    {
        RaceStats.AverageSpeed = TotalGap / (SortedRacers.Num() - 1);
    }
}

void UMGRaceDirectorSubsystem::UpdateStatistics()
{
    float TotalSpeed = 0.0f;
    int32 SpeedCount = 0;

    for (const auto& Pair : RacerStates)
    {
        if (Pair.Value.bIsActive && !Pair.Value.bHasFinished)
        {
            TotalSpeed += Pair.Value.CurrentSpeed;
            SpeedCount++;
        }
    }

    if (SpeedCount > 0)
    {
        RaceStats.AverageSpeed = TotalSpeed / SpeedCount;
    }

    RaceStats.TotalDramaticMoments = DramaticEvents.Num();
}

void UMGRaceDirectorSubsystem::SetRacePhase(EMGRacePhase NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        OnRacePhaseChanged.Broadcast(NewPhase);

        UE_LOG(LogTemp, Log, TEXT("RaceDirector: Phase changed to %d"), (int32)NewPhase);
    }
}

void UMGRaceDirectorSubsystem::SetTensionLevel(EMGRaceTension NewLevel)
{
    if (TensionLevel != NewLevel)
    {
        TensionLevel = NewLevel;
        OnTensionChanged.Broadcast(NewLevel);
    }
}

void UMGRaceDirectorSubsystem::TriggerDramaticMoment(EMGDramaticMoment Moment, const FGuid& PrimaryRacer, const FGuid& SecondaryRacer)
{
    CurrentMoment = Moment;
    LastDramaTime = RaceTime;

    FMGRaceEvent Event;
    Event.EventId = FGuid::NewGuid();
    Event.MomentType = Moment;
    Event.Timestamp = RaceTime;
    Event.PrimaryRacerId = PrimaryRacer;
    Event.SecondaryRacerId = SecondaryRacer;
    Event.Intensity = TensionScore;

    // Find current lap
    if (const FMGRacerState* State = RacerStates.Find(PrimaryRacer))
    {
        Event.Lap = State->CurrentLap;
    }

    DramaticEvents.Add(Event);
    RaceStats.TotalDramaticMoments++;

    OnDramaticMoment.Broadcast(Event);

    UE_LOG(LogTemp, Log, TEXT("RaceDirector: Dramatic moment triggered - %d"), (int32)Moment);
}

float UMGRaceDirectorSubsystem::CalculateRubberBandModifier(const FMGRacerState& Racer) const
{
    float Modifier = 1.0f;

    float LevelMultiplier = 1.0f;
    switch (RubberBandConfig.Level)
    {
        case EMGRubberBandLevel::VeryLight:
            LevelMultiplier = 0.25f;
            break;
        case EMGRubberBandLevel::Light:
            LevelMultiplier = 0.5f;
            break;
        case EMGRubberBandLevel::Moderate:
            LevelMultiplier = 1.0f;
            break;
        case EMGRubberBandLevel::Strong:
            LevelMultiplier = 1.5f;
            break;
        case EMGRubberBandLevel::VeryStrong:
            LevelMultiplier = 2.0f;
            break;
        default:
            return 1.0f;
    }

    int32 TotalRacers = RacerStates.Num();
    float PositionFactor = 0.0f;

    if (RubberBandConfig.bScaleWithPosition && TotalRacers > 1)
    {
        // Normalize position: -1 (last) to +1 (first)
        PositionFactor = 1.0f - (2.0f * (Racer.CurrentPosition - 1) / (TotalRacers - 1));
    }

    // Trailing racers get boost, leading racers get reduction
    if (PositionFactor < 0.0f) // Trailing
    {
        float BoostAmount = FMath::Abs(PositionFactor) * (RubberBandConfig.MaxSpeedBoost - 1.0f) * LevelMultiplier;
        Modifier = 1.0f + FMath::Clamp(BoostAmount, 0.0f, RubberBandConfig.MaxSpeedBoost - 1.0f);
    }
    else if (PositionFactor > 0.5f) // Well ahead
    {
        float ReductionAmount = PositionFactor * (1.0f - RubberBandConfig.MaxSpeedReduction) * LevelMultiplier;
        Modifier = 1.0f - FMath::Clamp(ReductionAmount, 0.0f, 1.0f - RubberBandConfig.MaxSpeedReduction);
    }

    // Distance-based activation
    if (Racer.DistanceToLeader > RubberBandConfig.ActivationDistance)
    {
        float DistanceFactor = (Racer.DistanceToLeader - RubberBandConfig.ActivationDistance) /
                               RubberBandConfig.ActivationDistance;
        DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

        if (Modifier > 1.0f)
        {
            Modifier = 1.0f + (Modifier - 1.0f) * (1.0f + DistanceFactor * 0.5f);
        }
    }

    // Phase-based adjustments
    if (CurrentPhase == EMGRacePhase::FinalLap || CurrentPhase == EMGRacePhase::PhotoFinish)
    {
        // Increase rubber banding intensity for exciting finish
        if (Modifier > 1.0f)
        {
            Modifier = 1.0f + (Modifier - 1.0f) * PacingConfig.FinalLapIntensity;
        }
    }

    return Modifier;
}

EMGAIBehaviorState UMGRaceDirectorSubsystem::DetermineAIBehavior(const FMGRacerState& Racer) const
{
    // Recovery from mistake
    if (Racer.BehaviorState == EMGAIBehaviorState::Mistake)
    {
        return EMGAIBehaviorState::Recovery;
    }

    // Determine based on race situation
    if (Racer.CurrentPosition == 1)
    {
        // Leader behavior
        if (Racer.DistanceToBehind < DramaConfig.CloseRaceThreshold)
        {
            return Racer.AggresionLevel > 0.6f ? EMGAIBehaviorState::Blocking : EMGAIBehaviorState::Defensive;
        }
        return EMGAIBehaviorState::Normal;
    }
    else if (Racer.DistanceToAhead < DramaConfig.CloseRaceThreshold)
    {
        // Close to car ahead - attacking
        return Racer.AggresionLevel > 0.5f ? EMGAIBehaviorState::Aggressive : EMGAIBehaviorState::Hunting;
    }
    else if (Racer.DistanceToLeader > RubberBandConfig.ActivationDistance)
    {
        // Far behind - catch up mode
        return EMGAIBehaviorState::CatchUp;
    }
    else if (Racer.CurrentPosition <= 3 && CurrentPhase >= EMGRacePhase::LateRace)
    {
        // In podium position during late race
        return Racer.AggresionLevel > 0.7f ? EMGAIBehaviorState::Aggressive : EMGAIBehaviorState::Hunting;
    }

    // Rival behavior
    if (Racer.bIsRival)
    {
        return EMGAIBehaviorState::Aggressive;
    }

    return EMGAIBehaviorState::Normal;
}

void UMGRaceDirectorSubsystem::InitializeDifficultyPresets()
{
    // Easy
    FMGAIDifficultyConfig Easy;
    Easy.DifficultyName = TEXT("Easy");
    Easy.SpeedMultiplier = 0.85f;
    Easy.ReactionTime = 0.5f;
    Easy.MistakeFrequency = 0.2f;
    Easy.AggressionBase = 0.3f;
    Easy.RacingLineOptimality = 0.6f;
    Easy.NitroUsageEfficiency = 0.5f;
    Easy.DriftProficiency = 0.5f;
    Easy.TrafficAvoidance = 0.6f;
    Easy.RecoverySpeed = 0.6f;
    Easy.RubberBandLevel = EMGRubberBandLevel::VeryStrong;
    DifficultyPresets.Add(Easy);

    // Normal
    FMGAIDifficultyConfig Normal;
    Normal.DifficultyName = TEXT("Normal");
    Normal.SpeedMultiplier = 0.95f;
    Normal.ReactionTime = 0.35f;
    Normal.MistakeFrequency = 0.1f;
    Normal.AggressionBase = 0.5f;
    Normal.RacingLineOptimality = 0.75f;
    Normal.NitroUsageEfficiency = 0.7f;
    Normal.DriftProficiency = 0.7f;
    Normal.TrafficAvoidance = 0.75f;
    Normal.RecoverySpeed = 0.75f;
    Normal.RubberBandLevel = EMGRubberBandLevel::Moderate;
    DifficultyPresets.Add(Normal);

    // Hard
    FMGAIDifficultyConfig Hard;
    Hard.DifficultyName = TEXT("Hard");
    Hard.SpeedMultiplier = 1.0f;
    Hard.ReactionTime = 0.25f;
    Hard.MistakeFrequency = 0.05f;
    Hard.AggressionBase = 0.65f;
    Hard.RacingLineOptimality = 0.85f;
    Hard.NitroUsageEfficiency = 0.85f;
    Hard.DriftProficiency = 0.85f;
    Hard.TrafficAvoidance = 0.85f;
    Hard.RecoverySpeed = 0.85f;
    Hard.RubberBandLevel = EMGRubberBandLevel::Light;
    DifficultyPresets.Add(Hard);

    // Expert
    FMGAIDifficultyConfig Expert;
    Expert.DifficultyName = TEXT("Expert");
    Expert.SpeedMultiplier = 1.05f;
    Expert.ReactionTime = 0.15f;
    Expert.MistakeFrequency = 0.02f;
    Expert.AggressionBase = 0.75f;
    Expert.RacingLineOptimality = 0.95f;
    Expert.NitroUsageEfficiency = 0.95f;
    Expert.DriftProficiency = 0.95f;
    Expert.TrafficAvoidance = 0.95f;
    Expert.RecoverySpeed = 0.95f;
    Expert.RubberBandLevel = EMGRubberBandLevel::VeryLight;
    DifficultyPresets.Add(Expert);

    // Legendary
    FMGAIDifficultyConfig Legendary;
    Legendary.DifficultyName = TEXT("Legendary");
    Legendary.SpeedMultiplier = 1.1f;
    Legendary.ReactionTime = 0.1f;
    Legendary.MistakeFrequency = 0.0f;
    Legendary.AggressionBase = 0.85f;
    Legendary.RacingLineOptimality = 1.0f;
    Legendary.NitroUsageEfficiency = 1.0f;
    Legendary.DriftProficiency = 1.0f;
    Legendary.TrafficAvoidance = 1.0f;
    Legendary.RecoverySpeed = 1.0f;
    Legendary.RubberBandLevel = EMGRubberBandLevel::None;
    DifficultyPresets.Add(Legendary);

    // Set default difficulty
    DifficultyConfig = Normal;
}

void UMGRaceDirectorSubsystem::ApplyDirectorStyle()
{
    switch (DirectorStyle)
    {
        case EMGDirectorStyle::Authentic:
            RubberBandConfig.Level = EMGRubberBandLevel::VeryLight;
            DramaConfig.bEnableDramaticMoments = false;
            break;

        case EMGDirectorStyle::Competitive:
            RubberBandConfig.Level = EMGRubberBandLevel::Moderate;
            DramaConfig.bEnableDramaticMoments = true;
            break;

        case EMGDirectorStyle::Dramatic:
            RubberBandConfig.Level = EMGRubberBandLevel::Strong;
            DramaConfig.bEnableDramaticMoments = true;
            DramaConfig.bEnableRivalrySystem = true;
            DramaConfig.bEnableUnderdogBonus = true;
            break;

        case EMGDirectorStyle::Arcade:
            RubberBandConfig.Level = EMGRubberBandLevel::VeryStrong;
            DramaConfig.bEnableDramaticMoments = true;
            break;

        case EMGDirectorStyle::Simulation:
            RubberBandConfig.Level = EMGRubberBandLevel::None;
            DramaConfig.bEnableDramaticMoments = false;
            break;

        case EMGDirectorStyle::Balanced:
        default:
            RubberBandConfig.Level = EMGRubberBandLevel::Moderate;
            DramaConfig.bEnableDramaticMoments = true;
            break;
    }
}
