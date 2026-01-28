# ITERATION 95 - Tactic Cooldowns, Surface Integration & AI Prediction
## Midnight Grind - Enhanced Pursuit AI & Physics Integration

**Date:** 2026-01-28
**Phase:** Phase 3 - System Refinement
**Focus:** Pursuit tactic cooldowns, tire-surface physics integration, AI prediction, intelligent roadblocks

---

## OVERVIEW

This iteration enhances the pursuit and physics systems with four key features:
1. **Tactic Cooldowns** - RAM, PIT, and EMP tactics now have cooldown periods
2. **Surface Physics Integration** - Tire grip now dynamically responds to track surface type
3. **AI Prediction** - Police units use player velocity to predict and intercept
4. **Intelligent Roadblocks** - Roadblocks placed using track spline data

---

## CHANGES MADE

### 1. Pursuit Tactic Cooldowns

**File:** `Public/Pursuit/MGPursuitSubsystem.h`

Added cooldown tracking to `FMGPursuitUnit`:
```cpp
/// Time remaining before unit can attempt RAM tactic again
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float RamCooldownRemaining = 0.0f;

/// Time remaining before unit can attempt PIT maneuver again
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float PITCooldownRemaining = 0.0f;

/// Time remaining before unit can attempt EMP again
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float EMPCooldownRemaining = 0.0f;
```

Added cooldown configuration to `FMGPursuitConfig`:
```cpp
/// Cooldown time after executing RAM tactic (seconds)
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float RamCooldown = 8.0f;

/// Cooldown time after attempting PIT maneuver (seconds)
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float PITCooldown = 12.0f;

/// Cooldown time after EMP deployment attempt (seconds)
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float EMPCooldown = 30.0f;
```

**Default Cooldowns:**
| Tactic | Cooldown | Rationale |
|--------|----------|-----------|
| RAM | 8 seconds | Allows recovery, prevents spam |
| PIT | 12 seconds | High-skill maneuver, longer setup |
| EMP | 30 seconds | Powerful effect, significant delay |

---

### 2. AI Prediction System

**File:** `Public/Pursuit/MGPursuitSubsystem.h`

Added prediction data to `FMGPursuitUnit`:
```cpp
/// Last known player location for prediction
UPROPERTY(EditAnywhere, BlueprintReadWrite)
FVector LastKnownPlayerLocation = FVector::ZeroVector;

/// Last known player velocity for prediction
UPROPERTY(EditAnywhere, BlueprintReadWrite)
FVector LastKnownPlayerVelocity = FVector::ZeroVector;

/// Time since last visual contact
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float TimeSinceLastVisual = 0.0f;
```

**File:** `Private/Pursuit/MGPursuitSubsystem.cpp`

**Prediction Logic:**
```cpp
// Calculate predicted player position (Iteration 95)
FVector PredictedPlayerLocation = bHasPlayerLocation ?
    EstimatedPlayerLocation + (EstimatedPlayerVelocity * PursuitConfig.PredictionTime) :
    EstimatedPlayerLocation;

// Use intercept vector for aggressive tactics
FVector ToIntercept = bHasPlayerLocation ?
    (PredictedPlayerLocation - Unit.Location).GetSafeNormal() : FVector::ZeroVector;
```

**Prediction Behavior:**
- Units store last known player location and velocity
- When visual is lost, prediction continues for `PredictionTime` seconds (default: 2s)
- Intercept tactics (RAM, PIT, Spike Strip) use predicted position
- Follow tactics use current position for smoother pursuit

---

### 3. Surface Physics Integration

**File:** `Public/Tire/MGTireSubsystem.h`

Added new surface integration functions:
```cpp
/**
 * Calculate complete tire grip at a world position including surface modifier
 */
UFUNCTION(BlueprintPure, Category = "Tire|Grip|Surface")
float CalculateGripAtPosition(FName VehicleID, EMGTirePosition Position, FVector WorldPosition) const;

/**
 * Get the current surface type at a world position via track subsystem
 */
UFUNCTION(BlueprintPure, Category = "Tire|Grip|Surface")
EMGTrackSurface GetSurfaceTypeAtPosition(FVector WorldPosition) const;

/**
 * Update all tire grips for a vehicle based on current surface
 */
UFUNCTION(BlueprintCallable, Category = "Tire|Grip|Surface")
void UpdateSurfaceGrip(FName VehicleID, FVector FLPosition, FVector FRPosition,
                       FVector RLPosition, FVector RRPosition);
```

**File:** `Private/Tire/MGTireSubsystem.cpp`

**Integration with Track Surface Detection:**
```cpp
EMGTrackSurface UMGTireSubsystem::GetSurfaceTypeAtPosition(FVector WorldPosition) const
{
    UWorld* World = GetWorld();
    if (!World)
        return EMGTrackSurface::Asphalt;

    // Get track subsystem for surface detection
    if (UMGTrackSubsystem* TrackSubsystem = World->GetSubsystem<UMGTrackSubsystem>())
    {
        // Use track subsystem's surface detection (implemented in Iteration 94)
        ::EMGTrackSurface TrackSurface = TrackSubsystem->GetSurfaceAtPosition(WorldPosition);
        // Map to tire surface enum...
    }
    return EMGTrackSurface::Asphalt;
}
```

**Surface Grip Modifiers (from existing system):**
| Surface | Grip Multiplier |
|---------|-----------------|
| Asphalt | 1.0 |
| Concrete | 1.0 |
| Gravel | 0.6 |
| Dirt | 0.5 |
| Grass | 0.4 |
| Sand | 0.3 |
| Snow | 0.2 (0.6 with studs) |
| Ice | 0.1 (0.4 with studs) |
| Wet | Compound-dependent |
| Puddle | Wet * 0.7 |
| Oil | 0.15 |

---

### 4. Intelligent Roadblock Positioning

**File:** `Public/Pursuit/MGPursuitSubsystem.h`

Added intelligent roadblock functions:
```cpp
/**
 * Calculate optimal roadblock position ahead of player using track data
 */
UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics|Roadblock")
bool CalculateOptimalRoadblockPosition(
    const FString& PlayerId,
    FVector PlayerLocation,
    FVector PlayerVelocity,
    float MinDistanceAhead,
    FVector& OutLocation,
    FRotator& OutRotation) const;

/**
 * Deploy a roadblock at the optimal position ahead of player
 */
UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics|Roadblock")
bool DeployOptimalRoadblock(
    const FString& PlayerId,
    FVector PlayerLocation,
    FVector PlayerVelocity,
    int32 NumUnits = 3,
    bool bIncludeSpikeStrip = false);
```

**File:** `Private/Pursuit/MGPursuitSubsystem.cpp`

**Positioning Algorithm:**
1. Get current distance along track spline
2. Calculate minimum reaction distance based on player speed (3 seconds ahead)
3. Use larger of: minimum distance parameter OR reaction distance
4. Query track spline for position and direction at target distance
5. Rotate roadblock to face oncoming traffic
6. Check for existing roadblocks and adjust if too close (50m minimum spacing)
7. Deploy roadblock with configured units and spike strips

**Example Calculation:**
```cpp
// Calculate minimum placement distance based on player speed
float PlayerSpeed = PlayerVelocity.Size(); // cm/s
float MinReactionDistance = PlayerSpeed * 3.0f; // 3 seconds ahead minimum
float ActualMinDistance = FMath::Max(MinDistanceAhead * 100.0f, MinReactionDistance);

// Target distance along track
float TargetDistance = CurrentDistance + ActualMinDistance;

// Get position at target distance from track spline
OutLocation = TrackSubsystem->GetPositionAtDistance(TargetDistance);
OutRotation = TrackSubsystem->GetDirectionAtDistance(TargetDistance);
OutRotation.Yaw += 180.0f; // Face oncoming traffic
```

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Public/Pursuit/MGPursuitSubsystem.h` | +Cooldown fields, +Prediction fields, +Roadblock functions |
| `Private/Pursuit/MGPursuitSubsystem.cpp` | +Include, +Cooldown initialization, +AI prediction, +Cooldown logic in tactics, +Roadblock positioning |
| `Public/Tire/MGTireSubsystem.h` | +Surface integration functions |
| `Private/Tire/MGTireSubsystem.cpp` | +Include, +Surface integration implementations |

**Lines Changed:** ~400 lines

---

## TACTIC BEHAVIOR CHANGES

### RAM Tactic (with cooldown)

**Before:**
- Units continuously attempt RAM when in range
- No recovery period between attempts
- Could result in "bumper car" gameplay

**After:**
- 8-second cooldown after successful RAM attempt
- During cooldown, unit maintains follow distance
- Uses prediction for intercept path
- Feels more realistic and tactical

### PIT Maneuver (with cooldown)

**Before:**
- Continuous PIT attempts when alongside target
- No positioning refinement
- Success depended on proximity only

**After:**
- 12-second cooldown after PIT attempt
- Enhanced positioning using player velocity perpendicular
- Aims for rear quarter panel accurately
- Uses intercept prediction for approach

### EMP (with cooldown)

**Before:**
- Instant EMP deployment when in range
- Could spam EMP attempts
- No strategic timing

**After:**
- 30-second cooldown (significant tactical resource)
- EMP becomes strategic choice
- Units track deployment success/failure

---

## GAMEPLAY IMPACT

### Pursuit Experience

**Before:**
- Police tactics felt "spammy"
- Unrealistic aggressive behavior
- No tactical depth

**After:**
- Tactics have weight and timing
- Player can learn cooldown windows
- More realistic pursuit pacing
- Creates "cat and mouse" gameplay

### Surface Physics

**Before:**
- Tire grip was uniform everywhere
- No penalty for going off-track
- Surface detection data unused

**After:**
- Real-time grip changes on surface transitions
- Off-track provides significant grip penalty
- Weather puddles affect handling
- Creates track knowledge advantage

### Roadblock Encounters

**Before:**
- Roadblocks placed at static positions
- No intelligence in placement
- Could appear unfairly close

**After:**
- Roadblocks placed along racing line
- Minimum reaction distance guaranteed
- Proper spacing between multiple roadblocks
- Feels like deliberate police strategy

---

## TESTING RECOMMENDATIONS

### Tactic Cooldown Test

```cpp
void TestTacticCooldowns()
{
    UMGPursuitSubsystem* Pursuit = GetPursuitSubsystem();

    // Start pursuit
    Pursuit->StartPursuit(TEXT("Player1"), EMGPursuitIntensity::High);

    // Get a unit and force RAM execution
    TArray<FMGPursuitUnit> Units = Pursuit->GetActiveUnits(TEXT("Player1"));
    FMGPursuitUnit& RamUnit = Units[0];
    RamUnit.CurrentTactic = EMGPursuitTactic::Ram;

    // Simulate close range
    RamUnit.DistanceToTarget = 200.0f;

    // Update AI - should trigger cooldown
    Pursuit->UpdateUnitAI(TEXT("Player1"), 0.1f);

    // Verify cooldown was set
    TArray<FMGPursuitUnit> UpdatedUnits = Pursuit->GetActiveUnits(TEXT("Player1"));
    ASSERT_GT(UpdatedUnits[0].RamCooldownRemaining, 0.0f);

    // Verify cooldown decrements
    Pursuit->UpdateUnitAI(TEXT("Player1"), 1.0f);
    UpdatedUnits = Pursuit->GetActiveUnits(TEXT("Player1"));
    ASSERT_LT(UpdatedUnits[0].RamCooldownRemaining, 8.0f);
}
```

### Surface Integration Test

```cpp
void TestSurfaceIntegration()
{
    UMGTireSubsystem* Tires = GetTireSubsystem();

    // Register test vehicle
    FName VehicleID = TEXT("TestCar");
    Tires->RegisterVehicle(VehicleID);
    Tires->SetTireCompound(VehicleID, EMGTirePosition::FrontLeft, EMGTireCompoundType::Medium);

    // Test asphalt grip
    FVector AsphaltPosition = FVector(1000, 0, 0);
    float AsphaltGrip = Tires->CalculateGripAtPosition(VehicleID, EMGTirePosition::FrontLeft, AsphaltPosition);

    // Test grass grip (should be lower)
    FVector GrassPosition = FVector(2000, 500, 0); // Off-track
    float GrassGrip = Tires->CalculateGripAtPosition(VehicleID, EMGTirePosition::FrontLeft, GrassPosition);

    ASSERT_GT(AsphaltGrip, GrassGrip);
}
```

### Roadblock Positioning Test

```cpp
void TestRoadblockPositioning()
{
    UMGPursuitSubsystem* Pursuit = GetPursuitSubsystem();

    // Start pursuit
    Pursuit->StartPursuit(TEXT("Player1"), EMGPursuitIntensity::High);

    // Player moving at 100 km/h forward
    FVector PlayerLocation = FVector(0, 0, 0);
    FVector PlayerVelocity = FVector(2778, 0, 0); // ~100 km/h in cm/s

    // Deploy optimal roadblock
    bool bDeployed = Pursuit->DeployOptimalRoadblock(
        TEXT("Player1"),
        PlayerLocation,
        PlayerVelocity,
        3,    // 3 units
        true  // Include spike strips
    );

    ASSERT_TRUE(bDeployed);

    // Verify roadblock is ahead of player
    TArray<FMGRoadblock> Roadblocks = Pursuit->GetActiveRoadblocks(TEXT("Player1"));
    ASSERT_EQ(Roadblocks.Num(), 1);
    ASSERT_GT(Roadblocks[0].Location.X, PlayerLocation.X + 50000); // At least 500m ahead
}
```

---

## NEXT STEPS

### Iteration 96 Recommendations

1. **Tactic Success/Failure Events** - Broadcast events when tactics succeed or fail
2. **Adaptive AI Cooldowns** - Adjust cooldowns based on pursuit intensity
3. **Surface Wear Multipliers** - Different surfaces cause different tire wear rates
4. **Dynamic Roadblock Width** - Adjust roadblock width based on track width

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P1 (Core gameplay systems - pursuit AI, physics)
**Type:** Feature Enhancement / System Integration

---

## MILESTONE: INTELLIGENT PURSUIT & PHYSICS INTEGRATION

**Iteration 95 delivered:**
- Tactic cooldown system for RAM (8s), PIT (12s), and EMP (30s)
- AI prediction using last known velocity for up to 2 seconds
- Surface grip integration connecting track detection to tire physics
- Intelligent roadblock positioning using track spline data
- Enhanced tactic execution with success/failure tracking

Police pursuits now feature tactical depth with meaningful cooldowns, and vehicle physics respond dynamically to track surfaces.

---
