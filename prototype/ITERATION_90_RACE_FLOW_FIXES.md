# ITERATION 90 - Race Flow Manager Fixes
## Midnight Grind - Personal Best & Clean Race Detection

**Date:** 2026-01-27
**Phase:** Phase 3 - System Refinement
**Focus:** Fix placeholder implementations in MGRaceFlowManager

---

## OVERVIEW

This iteration fixes two placeholder implementations in the Race Flow Manager:
1. **Personal Best Detection** - Now properly compares lap times against stored records
2. **Clean Race Detection** - Now uses actual collision tracking from DynamicDifficultySubsystem

Additionally fixed the race type ID determination which was hardcoded to "Circuit".

---

## CHANGES MADE

### 1. Added Collision Count Getter to DynamicDifficultySubsystem

**File:** `Public/DynamicDifficulty/MGDynamicDifficultySubsystem.h`

```cpp
/** Get the number of collisions recorded during the current race */
UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|RealTime")
int32 GetRaceCollisionCount() const { return RaceCollisionCount; }
```

This exposes the existing `RaceCollisionCount` member variable that was previously internal-only.

---

### 2. Fixed Clean Race Detection in CalculateRewards()

**File:** `Private/GameModes/MGRaceFlowManager.cpp`

**Before:**
```cpp
bool bCleanRace = true; // Assume clean unless collision data says otherwise
```

**After:**
```cpp
// Check collision count from DynamicDifficultySubsystem for clean race detection
bool bCleanRace = false;
if (UGameInstance* GI = GetGameInstance())
{
    if (UMGDynamicDifficultySubsystem* DifficultySubsystem = GI->GetSubsystem<UMGDynamicDifficultySubsystem>())
    {
        bCleanRace = (DifficultySubsystem->GetRaceCollisionCount() == 0);
    }
}
```

**Benefits:**
- Clean race bonus now accurately reflects player performance
- Integrates with existing collision tracking (OnPlayerCollision calls)
- No additional tracking systems needed

---

### 3. Fixed Personal Best Detection in BuildPostRaceSummary()

**File:** `Private/GameModes/MGRaceFlowManager.cpp`

**Before:**
```cpp
// Would check against stored personal best
PostRaceSummary.bNewPersonalBest = false; // Placeholder
```

**After:**
```cpp
// Get track records to compare against personal best
const FName TrackID = Results.Config.TrackName;
const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

// Convert lap time to milliseconds for comparison (scores stored as ms)
const int64 PlayerLapTimeMs = static_cast<int64>(Racer.BestLapTime * 1000.0f);
const int64 PersonalBestMs = TrackRecord.PersonalBest.Score;

// New personal best if no previous record (Score == 0) or faster time
PostRaceSummary.bNewPersonalBest = (PersonalBestMs == 0 || PlayerLapTimeMs < PersonalBestMs);
```

**Benefits:**
- Properly uses LeaderboardSubsystem's track records
- Handles first-time records (Score == 0)
- Converts float seconds to int64 milliseconds for accurate comparison

---

### 4. Fixed Race Type ID Determination

**File:** `Private/GameModes/MGRaceFlowManager.cpp`

**Before:**
```cpp
FName RaceTypeID = FName(TEXT("Circuit")); // Would come from CurrentConfig.RaceType
```

**After:**
```cpp
FName RaceTypeID;
switch (CurrentConfig.RaceType)
{
case EMGRaceType::Circuit:   RaceTypeID = FName(TEXT("Circuit")); break;
case EMGRaceType::Sprint:    RaceTypeID = FName(TEXT("Sprint")); break;
case EMGRaceType::Drift:     RaceTypeID = FName(TEXT("Drift")); break;
case EMGRaceType::TimeTrial: RaceTypeID = FName(TEXT("TimeTrial")); break;
case EMGRaceType::Drag:      RaceTypeID = FName(TEXT("Drag")); break;
case EMGRaceType::PinkSlip:  RaceTypeID = FName(TEXT("PinkSlip")); break;
default:                     RaceTypeID = FName(TEXT("Circuit")); break;
}
```

**Benefits:**
- Statistics now properly categorized by race type
- Enables race-type-specific progression tracking
- Supports all 6 race types defined in EMGRaceType

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Public/DynamicDifficulty/MGDynamicDifficultySubsystem.h` | +GetRaceCollisionCount() getter |
| `Private/GameModes/MGRaceFlowManager.cpp` | +DynamicDifficulty include, fixed 3 placeholders |

**Lines Changed:** ~35 lines

---

## INTEGRATION NOTES

### Collision Tracking Flow

```
Player Collision Event
        ↓
OnPlayerCollision() [DynamicDifficultySubsystem]
        ↓
RaceCollisionCount++ [internal tracking]
        ↓
Race Ends → CalculateRewards()
        ↓
GetRaceCollisionCount() == 0 ?
        ↓
bCleanRace = true/false
        ↓
CleanRaceBonus applied (1500 credits if clean)
```

### Personal Best Comparison Flow

```
Race Ends → BuildPostRaceSummary()
        ↓
Get player's BestLapTime from results
        ↓
LeaderboardSubsystem->GetTrackRecords(TrackID)
        ↓
Compare PlayerLapTimeMs < PersonalBest.Score
        ↓
PostRaceSummary.bNewPersonalBest = true/false
```

---

## TESTING RECOMMENDATIONS

### Clean Race Detection Test

```cpp
void TestCleanRaceDetection()
{
    UMGDynamicDifficultySubsystem* Difficulty = GetDifficultySubsystem();
    UMGRaceFlowManager* FlowManager = GetRaceFlowManager();

    // Reset collision count
    Difficulty->ResetRaceTimeAdjustments();
    ASSERT_EQ(Difficulty->GetRaceCollisionCount(), 0);

    // Simulate race with no collisions
    FMGRaceResults CleanResults = CreateMockRaceResults();
    FMGRaceRewardBreakdown Rewards = FlowManager->CalculateRewards(CleanResults);
    ASSERT_EQ(Rewards.CleanRaceBonus, 1500);

    // Simulate collisions
    Difficulty->OnPlayerCollision(0.5f);
    Difficulty->OnPlayerCollision(0.3f);
    ASSERT_EQ(Difficulty->GetRaceCollisionCount(), 2);

    // Verify dirty race
    Rewards = FlowManager->CalculateRewards(CleanResults);
    ASSERT_EQ(Rewards.CleanRaceBonus, 0);
}
```

### Personal Best Detection Test

```cpp
void TestPersonalBestDetection()
{
    UMGLeaderboardSubsystem* Leaderboard = GetLeaderboardSubsystem();
    UMGRaceFlowManager* FlowManager = GetRaceFlowManager();

    // Set up track with known personal best
    FName TestTrack = FName(TEXT("Track_Downtown"));
    FMGTrackRecord Record = Leaderboard->GetTrackRecords(TestTrack);
    // Assume PersonalBest.Score = 65000 (65 seconds in ms)

    // Test: Beat personal best
    FMGRaceResults Results = CreateMockRaceResults();
    Results.RacerResults[0].BestLapTime = 63.5f; // 63.5 seconds
    FlowManager->BuildPostRaceSummary(Results);
    ASSERT_TRUE(FlowManager->GetPostRaceSummary().bNewPersonalBest);

    // Test: Did not beat personal best
    Results.RacerResults[0].BestLapTime = 68.0f; // 68 seconds
    FlowManager->BuildPostRaceSummary(Results);
    ASSERT_FALSE(FlowManager->GetPostRaceSummary().bNewPersonalBest);
}
```

---

## NEXT STEPS

### Iteration 91 Recommendations

1. **Submit Personal Best to Leaderboard** - After detecting new PB, call `SubmitLapTime()`
2. **Clean Race Streak Tracking** - Track consecutive clean races for achievements
3. **Ghost Data Recording** - Record ghost data on new personal bests
4. **Race Type Statistics UI** - Display per-race-type stats in career menu

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P1 (Critical bug fixes - placeholder removal)
**Type:** Bug Fix / Placeholder Implementation

---

## MILESTONE: ACCURATE RACE TRACKING

**Iteration 90 delivered:**
- Proper clean race detection using collision tracking
- Accurate personal best comparison against stored records
- Correct race type categorization for statistics
- Exposed collision count API for Blueprint access

Player achievements and rewards now accurately reflect their actual performance.

---
