# ITERATION 91 - Race Rewards Processor Fixes
## Midnight Grind - Personal Best & Track Record Detection

**Date:** 2026-01-27
**Phase:** Phase 3 - System Refinement
**Focus:** Fix placeholder implementations in MGRaceRewardsProcessor

---

## OVERVIEW

This iteration fixes the placeholder implementations in the Race Rewards Processor subsystem:
1. **CheckPersonalBest()** - Now properly queries LeaderboardSubsystem
2. **CheckTrackRecord()** - Now properly queries world records from LeaderboardSubsystem
3. **TrackID Bug Fix** - Fixed incorrect field reference (TrackID → TrackName)

---

## CHANGES MADE

### 1. Added LeaderboardSubsystem Include

**File:** `Private/Progression/MGRaceRewardsProcessor.cpp`

```cpp
#include "Social/MGLeaderboardSubsystem.h"
```

---

### 2. Fixed CheckPersonalBest() Implementation

**File:** `Private/Progression/MGRaceRewardsProcessor.cpp`

**Before:**
```cpp
bool UMGRaceRewardsProcessor::CheckPersonalBest(FName TrackID, float Time) const
{
    // Would check against saved personal bests
    // For now, return false - actual implementation would use save system
    return false;
}
```

**After:**
```cpp
bool UMGRaceRewardsProcessor::CheckPersonalBest(FName TrackID, float Time) const
{
    if (Time <= 0.0f)
    {
        return false;
    }

    // Get leaderboard subsystem to check personal best
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMGLeaderboardSubsystem* LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>())
            {
                const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

                // Convert time to milliseconds for comparison (scores stored as ms)
                const int64 TimeMs = static_cast<int64>(Time * 1000.0f);
                const int64 PersonalBestMs = TrackRecord.PersonalBest.Score;

                // New personal best if no previous record (Score == 0) or faster time
                return (PersonalBestMs == 0 || TimeMs < PersonalBestMs);
            }
        }
    }

    return false;
}
```

---

### 3. Fixed CheckTrackRecord() Implementation

**File:** `Private/Progression/MGRaceRewardsProcessor.cpp`

**Before:**
```cpp
bool UMGRaceRewardsProcessor::CheckTrackRecord(FName TrackID, float Time) const
{
    // Would check against leaderboard/track records
    // For now, return false - actual implementation would use leaderboard system
    return false;
}
```

**After:**
```cpp
bool UMGRaceRewardsProcessor::CheckTrackRecord(FName TrackID, float Time) const
{
    if (Time <= 0.0f)
    {
        return false;
    }

    // Get leaderboard subsystem to check track record
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMGLeaderboardSubsystem* LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>())
            {
                const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

                // Convert time to milliseconds for comparison (scores stored as ms)
                const int64 TimeMs = static_cast<int64>(Time * 1000.0f);
                const int64 WorldRecordMs = TrackRecord.WorldRecord.Score;

                // New track record if no existing record (Score == 0) or faster time
                return (WorldRecordMs == 0 || TimeMs < WorldRecordMs);
            }
        }
    }

    return false;
}
```

---

### 4. Fixed TrackID Bug

**File:** `Private/Progression/MGRaceRewardsProcessor.cpp`

**Before (Bug):**
```cpp
FName TrackID = RaceGameMode->GetRaceConfig().TrackID;  // TrackID doesn't exist!
```

**After (Fixed):**
```cpp
FName TrackID = RaceGameMode->GetRaceConfig().TrackName;  // Correct field name
```

This was a compilation bug - `FMGRaceConfig` has `TrackName` not `TrackID`.

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Private/Progression/MGRaceRewardsProcessor.cpp` | +LeaderboardSubsystem include, fixed CheckPersonalBest(), fixed CheckTrackRecord(), fixed TrackID→TrackName bug |

**Lines Changed:** ~55 lines

---

## INTEGRATION NOTES

### Time Format Convention

Both LeaderboardSubsystem (via FMGTrackRecord) and SaveSubsystem store lap/race times as **int64 milliseconds**:

```
Race Time (float seconds) × 1000 = Score (int64 milliseconds)
Example: 65.432 seconds → 65432 ms
```

### Record Comparison Logic

```cpp
// Personal best: Beat your own record
PersonalBestMs == 0  // No previous record (first time)
    || TimeMs < PersonalBestMs  // Faster than previous

// Track record: Beat the world record
WorldRecordMs == 0  // No existing record
    || TimeMs < WorldRecordMs  // Faster than existing
```

---

## REWARD FLOW

```
Race Ends
    ↓
CalculateRewards(Result, RaceCrew, bIsRanked)
    ↓
CheckPersonalBest(TrackID, TotalTime)
    ├─ Get FMGTrackRecord from LeaderboardSubsystem
    ├─ Compare TotalTime vs PersonalBest.Score
    └─ Set Rewards.bNewPersonalBest
    ↓
CheckTrackRecord(TrackID, TotalTime)
    ├─ Get FMGTrackRecord from LeaderboardSubsystem
    ├─ Compare TotalTime vs WorldRecord.Score
    └─ Set Rewards.bNewTrackRecord
    ↓
OnRaceRewardsCalculated.Broadcast(Rewards)
```

---

## TESTING RECOMMENDATIONS

### Personal Best Test

```cpp
void TestPersonalBestDetection()
{
    UMGRaceRewardsProcessor* Processor = GetWorld()->GetSubsystem<UMGRaceRewardsProcessor>();
    FName TestTrack = FName(TEXT("Track_Downtown"));

    // First run (no existing record)
    bool bFirstRun = Processor->CheckPersonalBest(TestTrack, 65.0f);
    ASSERT_TRUE(bFirstRun);  // Should be true - no prior record

    // After submitting: 65.0f becomes the personal best
    // Faster time should be a new PB
    bool bFaster = Processor->CheckPersonalBest(TestTrack, 63.5f);
    ASSERT_TRUE(bFaster);

    // Slower time should not be a new PB
    bool bSlower = Processor->CheckPersonalBest(TestTrack, 68.0f);
    ASSERT_FALSE(bSlower);
}
```

### Track Record Test

```cpp
void TestTrackRecordDetection()
{
    UMGRaceRewardsProcessor* Processor = GetWorld()->GetSubsystem<UMGRaceRewardsProcessor>();
    FName TestTrack = FName(TEXT("Track_Highway"));

    // Assume world record is 55.0 seconds (55000 ms)
    bool bBeatsRecord = Processor->CheckTrackRecord(TestTrack, 54.5f);
    ASSERT_TRUE(bBeatsRecord);

    bool bDoesntBeatRecord = Processor->CheckTrackRecord(TestTrack, 56.0f);
    ASSERT_FALSE(bDoesntBeatRecord);
}
```

---

## RELATIONSHIP TO ITERATION 90

Iteration 90 fixed similar personal best detection in **MGRaceFlowManager** (for post-race summary display).

Iteration 91 fixes the same issue in **MGRaceRewardsProcessor** (for reward calculation and XP bonuses).

Both systems now use the same approach:
- Query `LeaderboardSubsystem->GetTrackRecords(TrackID)`
- Compare times in milliseconds
- Handle first-run case (Score == 0)

---

## NEXT STEPS

### Iteration 92 Recommendations

1. **Submit Records to Leaderboard** - After detecting new PB/record, call `SubmitLapTime()` or `SubmitRaceTime()`
2. **Personal Best XP Bonus** - Award extra XP when setting a new personal best
3. **Track Record UI Celebration** - Add special UI effects when breaking track record
4. **Ghost Data Recording** - Save ghost replay data on new records

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P1 (Critical bug fixes - placeholder removal, compilation fix)
**Type:** Bug Fix / Placeholder Implementation

---

## MILESTONE: COMPLETE RECORD TRACKING

**Iteration 91 delivered:**
- Proper personal best detection in reward processor
- Proper track record detection in reward processor
- Fixed TrackID → TrackName bug that would have caused compilation failure
- Consistent time format handling (float seconds ↔ int64 milliseconds)

Combined with Iteration 90, all race record detection is now properly implemented across both subsystems.

---
