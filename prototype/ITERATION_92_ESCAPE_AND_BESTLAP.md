# ITERATION 92 - Escape Progress & Best Lap Detection
## Midnight Grind - Police Chase Visibility & Accurate Rewards

**Date:** 2026-01-27
**Phase:** Phase 3 - System Refinement
**Focus:** Improve escape progress calculation and best lap XP bonus accuracy

---

## OVERVIEW

This iteration improves two gameplay systems:
1. **Escape Progress** - Now uses actual police visibility tracking instead of constant rate
2. **Best Lap XP Bonus** - Now accurately compares against race's actual best lap time

---

## CHANGES MADE

### 1. Improved Escape Progress Calculation

**File:** `Private/WorldEvents/MGWorldEventsSubsystem.cpp`

**Before (Placeholder):**
```cpp
// Natural escape progress when out of sight (would check actual visibility)
// This is placeholder logic
float EscapeGain = 0.01f * DeltaTime; // 1% per second when hidden
CurrentPoliceEncounter.EscapeProgress = FMath::Min(
    1.0f, CurrentPoliceEncounter.EscapeProgress + EscapeGain);
```

**After (Visibility-Based):**
```cpp
// Check visibility via HeatLevelSubsystem for escape progress
bool bHasVisual = true; // Default to being spotted
if (UGameInstance* GI = GetGameInstance())
{
    if (UMGHeatLevelSubsystem* HeatSubsystem = GI->GetSubsystem<UMGHeatLevelSubsystem>())
    {
        bHasVisual = HeatSubsystem->AnyUnitHasVisual();
    }
}

// Calculate escape progress based on visibility
if (bHasVisual)
{
    // When visible, escape progress decreases (cops are catching up)
    float EscapeLoss = 0.03f * DeltaTime; // 3% per second when spotted
    CurrentPoliceEncounter.EscapeProgress = FMath::Max(
        0.0f, CurrentPoliceEncounter.EscapeProgress - EscapeLoss);
}
else
{
    // When hidden, escape progress increases (getting away)
    // Rate depends on heat level - higher heat = slower escape
    float BaseEscapeRate = 0.05f; // 5% per second base rate
    float HeatPenalty = FMath::Clamp(CurrentPoliceEncounter.HeatLevel * 0.01f, 0.0f, 0.03f);
    float EscapeGain = (BaseEscapeRate - HeatPenalty) * DeltaTime;
    CurrentPoliceEncounter.EscapeProgress = FMath::Min(
        1.0f, CurrentPoliceEncounter.EscapeProgress + EscapeGain);
}
```

**Key Improvements:**
- Integrates with `HeatLevelSubsystem::AnyUnitHasVisual()` for actual visibility tracking
- Escape progress **decreases** when spotted (3%/sec)
- Escape progress **increases** when hidden (5%/sec base)
- Higher heat levels slow down escape rate
- Creates tension: hiding is rewarded, being spotted is punished

---

### 2. Improved Best Lap XP Detection

**File:** `Private/Progression/MGRaceRewardsProcessor.cpp`

**Before (Heuristic):**
```cpp
// Best lap bonus - check if this racer had the best lap
// This would require comparing across all results
// For now, give bonus if position is 1st-3rd and they have a lap time
if (Result.Position <= 3 && Result.BestLap > 0.0f)
{
    XP.BestLapXP = BestLapXP;
}
```

**After (Accurate):**
```cpp
// Best lap bonus - check if this racer had the best lap
bool bHadBestLap = false;
if (RaceGameMode.IsValid() && Result.BestLap > 0.0f)
{
    FMGRaceResults RaceResults = RaceGameMode->GetRaceResults();
    // Check if player's best lap matches the race's best lap time
    // Use small epsilon for float comparison
    bHadBestLap = FMath::IsNearlyEqual(Result.BestLap, RaceResults.BestLapTime, 0.001f);
}
if (bHadBestLap)
{
    XP.BestLapXP = BestLapXP;
}
```

**Key Improvements:**
- Queries actual race results from `RaceGameMode->GetRaceResults()`
- Compares player's best lap against the race's `BestLapTime`
- Uses epsilon comparison for float precision (0.001 seconds)
- Best lap bonus now goes to actual best lap holder, not just top 3

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Private/WorldEvents/MGWorldEventsSubsystem.cpp` | +HeatLevelSubsystem include, improved escape progress |
| `Private/Progression/MGRaceRewardsProcessor.cpp` | Improved best lap detection logic |

**Lines Changed:** ~35 lines

---

## ESCAPE PROGRESS MECHANICS

### Visibility States

| State | Escape Progress | Rate |
|-------|-----------------|------|
| **Spotted** (bHasVisual = true) | Decreases | -3% per second |
| **Hidden** (bHasVisual = false) | Increases | +5% per second (base) |
| **High Heat** (Level 3+) | Increases slower | +2% to +3% per second |

### Escape Thresholds

```
0%   ████████████████████ 100%
     ↑ Cops catching up    ↑ Escaped!
```

- **0%**: About to be busted
- **100%**: Successfully escaped

### Heat Level Penalty

| Heat Level | Escape Rate When Hidden |
|------------|------------------------|
| 1 | 4% per second |
| 2 | 3% per second |
| 3+ | 2% per second |

---

## BEST LAP DETECTION FLOW

```
Race Ends
    ↓
CalculateXPBreakdown(PlayerResult)
    ↓
Query RaceGameMode->GetRaceResults()
    ↓
Compare: PlayerResult.BestLap ≈ RaceResults.BestLapTime?
    ↓
├── Match (within 0.001s) → Award BestLapXP (150 XP)
└── No Match → No Best Lap Bonus
```

---

## TESTING RECOMMENDATIONS

### Escape Progress Test

```cpp
void TestEscapeProgress()
{
    UMGWorldEventsSubsystem* WorldEvents = GetWorldEventsSubsystem();
    UMGHeatLevelSubsystem* HeatLevel = GetHeatLevelSubsystem();

    // Start a police chase
    WorldEvents->StartPoliceChase();

    // Simulate being spotted
    HeatLevel->UpdatePoliceUnit("Unit1", PlayerLocation, true, true); // bHasVisual = true
    WorldEvents->UpdatePoliceChase(1.0f);
    float ProgressAfterSpotted = WorldEvents->GetPoliceEncounter().EscapeProgress;
    // Progress should have decreased

    // Simulate losing them
    HeatLevel->UpdatePoliceUnit("Unit1", FarAwayLocation, false, true); // bHasVisual = false
    WorldEvents->UpdatePoliceChase(1.0f);
    float ProgressAfterHidden = WorldEvents->GetPoliceEncounter().EscapeProgress;
    // Progress should have increased

    ASSERT_GT(ProgressAfterHidden, ProgressAfterSpotted);
}
```

### Best Lap XP Test

```cpp
void TestBestLapXP()
{
    UMGRaceRewardsProcessor* Processor = GetRewardsProcessor();
    AMGRaceGameMode* RaceMode = GetRaceGameMode();

    // Simulate race where player got best lap of 45.5 seconds
    FMGRaceResults Results;
    Results.BestLapTime = 45.5f;
    Results.BestLapRacerIndex = 0; // Player

    FMGFinalRaceResult PlayerResult;
    PlayerResult.BestLap = 45.5f;
    PlayerResult.Position = 2; // Even 2nd place can get best lap!

    FMGXPBreakdown XP = Processor->CalculateXPBreakdown(PlayerResult, false, 8);
    ASSERT_EQ(XP.BestLapXP, 150); // Should get best lap bonus

    // Test case: Player didn't get best lap
    PlayerResult.BestLap = 47.2f; // Slower
    XP = Processor->CalculateXPBreakdown(PlayerResult, false, 8);
    ASSERT_EQ(XP.BestLapXP, 0); // Should NOT get best lap bonus
}
```

---

## GAMEPLAY IMPACT

### Escape Progress Changes

**Before:**
- Escape progress always increased at same rate
- No incentive to break line of sight
- Escaping felt passive/automatic

**After:**
- Must actively avoid detection to escape
- Getting spotted reverses escape progress
- Higher heat = harder to escape
- Creates tension and skill expression

### Best Lap XP Changes

**Before:**
- Top 3 finishers always got best lap bonus
- Unfair: 1st place without best lap got bonus
- Unfair: 4th place with best lap got nothing

**After:**
- Only actual best lap holder gets bonus
- Rewards skilled driving regardless of position
- Fair: You can finish 5th but still get best lap XP

---

## NEXT STEPS

### Iteration 93 Recommendations

1. **Pursuit AI Integration** - Connect visibility tracking to actual AI pursuit behavior
2. **Cooldown Spot Bonus** - Faster escape progress when in designated hiding spots
3. **Best Lap Notification** - UI feedback when player sets new race best lap
4. **Heat Level Decay** - Reduce heat when successfully evading for extended time

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P2 (Gameplay improvement - escape mechanics, reward accuracy)
**Type:** Feature Enhancement / Placeholder Implementation

---

## MILESTONE: DYNAMIC CHASE MECHANICS

**Iteration 92 delivered:**
- Visibility-based escape progress (integrates with HeatLevelSubsystem)
- Escape progress now decreases when spotted
- Heat level affects escape difficulty
- Accurate best lap XP detection using actual race results
- Fair best lap bonus regardless of finishing position

Police chases now require active evasion, and race rewards accurately reflect performance.

---
