# ITERATION 93 - Track Randomization & Vehicle Reliability
## Midnight Grind - Session Tracks & Part Quality Impact

**Date:** 2026-01-27
**Phase:** Phase 3 - System Refinement
**Focus:** Fix track selection randomization and implement vehicle reliability calculation

---

## OVERVIEW

This iteration fixes two placeholder implementations:
1. **Track Randomization** - Session subsystem now properly selects from available tracks
2. **Vehicle Reliability** - Stat calculator now computes reliability based on part quality tiers

---

## CHANGES MADE

### 1. Fixed Track Randomization

**File:** `Private/Session/MGSessionSubsystem.cpp`

**Before (Placeholder):**
```cpp
void UMGSessionSubsystem::RandomizeTrack()
{
    if (!bIsHost)
        return;

    // Would pick random track from available tracks
    // For now, just set a placeholder
    CurrentSession.CurrentTrackID = FName(TEXT("Track_Downtown"));
}
```

**After (Functional):**
```cpp
void UMGSessionSubsystem::RandomizeTrack()
{
    if (!bIsHost)
        return;

    // Get available tracks from RaceFlowSubsystem and pick a random one
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMGRaceFlowSubsystem* RaceFlowSubsystem = GI->GetSubsystem<UMGRaceFlowSubsystem>())
        {
            TArray<FName> AvailableTracks = RaceFlowSubsystem->GetAvailableTracks();
            if (AvailableTracks.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, AvailableTracks.Num() - 1);
                CurrentSession.CurrentTrackID = AvailableTracks[RandomIndex];
                return;
            }
        }
    }

    // Fallback to default track
    CurrentSession.CurrentTrackID = FName(TEXT("Track_Downtown"));
}
```

**Key Improvements:**
- Uses centralized track list from `MGRaceFlowSubsystem::GetAvailableTracks()`
- Random selection from all 6 available tracks
- Graceful fallback if subsystem unavailable

---

### 2. Added Vehicle Reliability Calculation

**File:** `Public/Vehicle/MGStatCalculator.h`

Added new function declaration:
```cpp
/**
 * @brief Calculate vehicle reliability rating based on part quality tiers.
 *
 * Higher performance parts (Race, Pro, Legendary) are less reliable than
 * OEM or Street tier parts. Based on the principle that more extreme
 * builds require more maintenance.
 *
 * @param Vehicle Complete vehicle configuration data.
 * @return Reliability rating from 0 (unreliable) to 100 (stock reliability).
 */
UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
static float CalculateReliability(const FMGVehicleData& Vehicle);
```

**File:** `Private/Vehicle/MGStatCalculator.cpp`

**Before (Placeholder):**
```cpp
// Reliability (placeholder)
Stats.ReliabilityRating = 100.0f;
```

**After (Calculated):**
```cpp
// Reliability based on part quality tiers
Stats.ReliabilityRating = CalculateReliability(Vehicle);
```

**Implementation:**
```cpp
float UMGStatCalculator::CalculateReliability(const FMGVehicleData& Vehicle)
{
    // Helper to convert part tier to reliability impact
    auto GetTierReliability = [](EMGPartTier Tier) -> float
    {
        switch (Tier)
        {
        case EMGPartTier::OEM:
        case EMGPartTier::Stock:
            return 100.0f;
        case EMGPartTier::Street:
            return 98.0f;
        case EMGPartTier::Sport:
            return 95.0f;
        case EMGPartTier::Race:
            return 90.0f;
        case EMGPartTier::Pro:
            return 85.0f;
        case EMGPartTier::Legendary:
            return 80.0f;
        default:
            return 100.0f;
        }
    };

    // Collect reliability from major engine components
    TArray<float> PartReliabilities;
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.AirFilterTier));
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.ExhaustTier));
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.CamshaftTier));
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.ValvesTier));
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.PistonsTier));
    PartReliabilities.Add(GetTierReliability(Vehicle.Engine.FuelSystemTier));

    // Forced induction penalty based on boost level
    if (Vehicle.Engine.ForcedInduction.Type != EMGForcedInductionType::None)
    {
        float BoostPenalty = FMath::GetMappedRangeValueClamped(
            FVector2D(5.0f, 25.0f), FVector2D(0.0f, 10.0f),
            Vehicle.Engine.ForcedInduction.MaxBoostPSI);
        PartReliabilities.Add(90.0f - BoostPenalty);
    }

    // Average all part reliabilities
    return FMath::Clamp(Average, 0.0f, 100.0f);
}
```

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Private/Session/MGSessionSubsystem.cpp` | +RaceFlowSubsystem include, fixed track randomization |
| `Public/Vehicle/MGStatCalculator.h` | +CalculateReliability declaration |
| `Private/Vehicle/MGStatCalculator.cpp` | +CalculateReliability implementation, updated CalculateAllStats |

**Lines Changed:** ~75 lines

---

## RELIABILITY TIERS

| Part Tier | Reliability | Notes |
|-----------|-------------|-------|
| OEM/Stock | 100% | Factory parts, maximum reliability |
| Street | 98% | Mild upgrades, minimal impact |
| Sport | 95% | Enthusiast parts, slight reduction |
| Race | 90% | Track parts, needs maintenance |
| Pro | 85% | Competition parts, frequent maintenance |
| Legendary | 80% | Exotic parts, high maintenance |

### Forced Induction Impact

| Boost PSI | Additional Penalty |
|-----------|-------------------|
| 5 PSI | 0% |
| 15 PSI | 5% |
| 25+ PSI | 10% |

### Example Calculations

**Stock Vehicle:**
- All OEM parts = 100% reliability
- No forced induction
- **Result: 100% reliability**

**Street Build:**
- Street exhaust + Street intake + Stock everything else
- Average: (98 + 98 + 100 + 100 + 100 + 100) / 6 = 99.3%
- **Result: 99% reliability**

**Full Race Build:**
- All Race tier parts + 15 PSI turbo
- Engine parts average: 90%
- Turbo penalty: -5%
- Average: (90×6 + 85) / 7 = 89.3%
- **Result: 89% reliability**

---

## AVAILABLE TRACKS

The track randomization now selects from:

| Track ID | Display Name |
|----------|--------------|
| Track_Downtown | Downtown Circuit |
| Track_Highway | Highway Sprint |
| Track_Industrial | Industrial Zone |
| Track_Mountain | Mountain Pass |
| Track_Airport | Airport Runway |
| Track_Docks | Dockside Drift |

---

## TESTING RECOMMENDATIONS

### Track Randomization Test

```cpp
void TestTrackRandomization()
{
    UMGSessionSubsystem* Session = GetSessionSubsystem();

    // Track frequency over 100 randomizations
    TMap<FName, int32> TrackCounts;
    for (int32 i = 0; i < 100; ++i)
    {
        Session->RandomizeTrack();
        FName Track = Session->GetCurrentSession().CurrentTrackID;
        TrackCounts.FindOrAdd(Track)++;
    }

    // Each track should appear roughly 16-17 times (100/6)
    // Allow variance of ±10
    for (const auto& Pair : TrackCounts)
    {
        ASSERT_GE(Pair.Value, 6);
        ASSERT_LE(Pair.Value, 27);
    }
}
```

### Reliability Calculation Test

```cpp
void TestReliabilityCalculation()
{
    // Stock vehicle
    FMGVehicleData StockVehicle = CreateStockVehicle();
    float StockReliability = UMGStatCalculator::CalculateReliability(StockVehicle);
    ASSERT_EQ(StockReliability, 100.0f);

    // Full race build
    FMGVehicleData RaceVehicle = CreateRaceVehicle();
    RaceVehicle.Engine.AirFilterTier = EMGPartTier::Race;
    RaceVehicle.Engine.ExhaustTier = EMGPartTier::Race;
    RaceVehicle.Engine.CamshaftTier = EMGPartTier::Race;
    // ... all Race parts

    float RaceReliability = UMGStatCalculator::CalculateReliability(RaceVehicle);
    ASSERT_LT(RaceReliability, 95.0f);
    ASSERT_GT(RaceReliability, 80.0f);
}
```

---

## GAMEPLAY IMPACT

### Track Randomization

**Before:**
- Multiplayer sessions always defaulted to Downtown
- No variety in session tracks

**After:**
- Random track from 6 available options
- Each session gets a different experience

### Vehicle Reliability

**Before:**
- All vehicles had 100% reliability regardless of parts
- No trade-off for extreme builds

**After:**
- Higher performance = lower reliability
- Encourages balanced builds for endurance
- Extreme builds have maintenance consequences
- Stock vehicles rewarded with reliability

---

## NEXT STEPS

### Iteration 94 Recommendations

1. **Reliability Effects** - Make low reliability affect in-race performance (random failures, overheating)
2. **Maintenance System** - Allow players to repair/maintain vehicles to restore reliability
3. **Track Voting** - Let session players vote on track instead of pure random
4. **Track Difficulty Rating** - Weight random selection by player skill level

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P2 (Gameplay improvement - session variety, vehicle balancing)
**Type:** Feature Enhancement / Placeholder Implementation

---

## MILESTONE: VEHICLE TRADE-OFFS

**Iteration 93 delivered:**
- Proper track randomization using centralized track list
- Reliability calculation based on part quality tiers
- Forced induction boost penalty system
- Trade-off gameplay: more power = less reliability

Sessions now offer variety, and vehicle builds now have meaningful trade-offs.

---
