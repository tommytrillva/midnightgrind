# ITERATION 86 - Physics Constants & Handling Mode Presets
## Midnight Grind - Designer-Friendly Physics Configuration

**Date:** 2026-01-26
**Phase:** Phase 3 - System Refinement
**Focus:** Document magic numbers, create physics handling presets

---

## IMPLEMENTATION COMPLETE

Created a comprehensive physics constants system and handling mode presets that allow designers to easily switch between Arcade, Balanced, and Simulation physics feels.

---

## NEW FILES CREATED

### 1. MGPhysicsConstants.h
**Path:** `Public/Vehicle/MGPhysicsConstants.h`

Documents all physics "magic numbers" with:
- Detailed comments explaining what each value does
- Real-world analogies where applicable
- Expected value ranges
- How values interact with other parameters

**Namespace Structure:**
```cpp
namespace MGPhysicsConstants
{
    namespace WeightTransfer { ... }
    namespace TireTemperature { ... }
    namespace Surface { ... }
    namespace Geometry { ... }
    namespace Differential { ... }
    namespace Wear { ... }
}
```

### 2. MGPhysicsConstants.cpp
**Path:** `Private/Vehicle/MGPhysicsConstants.cpp`

Implements the default handling settings for each physics mode.

---

## PHYSICS HANDLING MODE SYSTEM

### New Enum: EMGPhysicsHandlingMode

```cpp
enum class EMGPhysicsHandlingMode : uint8
{
    Arcade,      // Forgiving, assisted
    Balanced,    // Default, realistic but accessible
    Simulation   // Challenging, minimal assists
};
```

### Handling Settings Struct: FMGPhysicsHandlingSettings

| Parameter | Arcade | Balanced | Simulation |
|-----------|--------|----------|------------|
| StabilityControl | 0.7 | 0.3 | 0.0 |
| AntiFlipTorque | 15000 | 5000 | 0 |
| SpeedSensitiveSteeringFactor | 0.8 | 0.5 | 0.2 |
| WeightTransferRate | 4.0 | 8.0 | 12.0 |
| BaseTireGrip | 1.2 | 1.0 | 1.0 |
| TireTempInfluence | 0.0 | 0.3 | 1.0 |
| TurboLagSimulation | 0.0 | 0.5 | 1.0 |
| EngineBrakingMultiplier | 0.5 | 1.0 | 1.2 |
| ArcadeSteeringSpeed | 8.0 | 5.0 | 3.0 |
| ArcadeSteeringReturnSpeed | 12.0 | 8.0 | 5.0 |

---

## DOCUMENTED PHYSICS CONSTANTS

### Weight Transfer Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| LONGITUDINAL_RATIO | 0.15f | How much load transfers per unit of longitudinal acceleration |
| LATERAL_RATIO | 0.12f | How much load transfers per unit of lateral acceleration |
| LOAD_MIN | 0.3f | Minimum wheel load multiplier (prevents zero grip) |
| LOAD_MAX | 1.8f | Maximum wheel load multiplier (caps overloaded wheels) |
| ACCEL_TO_TRANSFER | 0.0001f | Converts raw acceleration to normalized weight transfer |
| DEFAULT_RATE | 8.0f | Default weight transfer interpolation speed |

### Tire Temperature Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| AMBIENT | 25.0f°C | Cold tire starting temperature |
| OPTIMAL | 90.0f°C | Best grip temperature |
| PEAK | 110.0f°C | Maximum grip temperature |
| OVERHEAT | 120.0f°C | Temperature where grip begins degrading |
| COLD_GRIP_MIN | 0.7f | Grip multiplier when cold |
| PEAK_GRIP | 1.05f | Maximum grip multiplier at optimal temp |

### Suspension Geometry Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| TOE_EFFECT_FACTOR | 0.15f | How much toe affects turn-in response |
| CAMBER_GRIP_PER_DEG | 0.02f | Lateral grip gain per degree of negative camber |
| OPTIMAL_CAMBER_DEG | -3.0f | Optimal camber for maximum grip |

### Surface Detection Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| ICE_FRICTION_THRESHOLD | 0.3f | Friction below this = ice surface |
| TRACES_PER_FRAME | 4 | Number of surface detection traces |

---

## VEHICLE MOVEMENT COMPONENT CHANGES

### New Functions

```cpp
// Apply a handling mode preset
UFUNCTION(BlueprintCallable, Category = "Vehicle|Handling")
void ApplyPhysicsHandlingMode(EMGPhysicsHandlingMode Mode);

// Apply custom handling settings
UFUNCTION(BlueprintCallable, Category = "Vehicle|Handling")
void ApplyPhysicsHandlingSettings(const FMGPhysicsHandlingSettings& Settings);

// Get current handling mode
UFUNCTION(BlueprintPure, Category = "Vehicle|Handling")
EMGPhysicsHandlingMode GetPhysicsHandlingMode() const;

// Get current handling settings
UFUNCTION(BlueprintPure, Category = "Vehicle|Handling")
const FMGPhysicsHandlingSettings& GetPhysicsHandlingSettings() const;
```

### New Member Variables

```cpp
UPROPERTY()
EMGPhysicsHandlingMode CurrentHandlingMode = EMGPhysicsHandlingMode::Balanced;

UPROPERTY()
FMGPhysicsHandlingSettings CurrentHandlingSettings;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drivetrain")
float EngineBrakingMultiplier = 1.0f;
```

### Updated Weight Transfer Calculation

**Before:**
```cpp
const float TargetLongitudinal = -LocalAccel.X * LongitudinalTransferFactor * 0.0001f;
```

**After:**
```cpp
const float TargetLongitudinal = -LocalAccel.X * LongitudinalTransferFactor
    * MGPhysicsConstants::WeightTransfer::ACCEL_TO_TRANSFER;
```

### Updated Wheel Load Multiplier

**Before:**
```cpp
if (bFront) BaseLoad += LongitudinalTransfer * 0.15f;
else BaseLoad -= LongitudinalTransfer * 0.15f;
if (bRight) BaseLoad += LateralTransfer * 0.12f;
return FMath::Clamp(BaseLoad, 0.3f, 1.8f);
```

**After:**
```cpp
using namespace MGPhysicsConstants::WeightTransfer;
if (bFront) BaseLoad += LongitudinalTransfer * LONGITUDINAL_RATIO;
else BaseLoad -= LongitudinalTransfer * LONGITUDINAL_RATIO;
if (bRight) BaseLoad += LateralTransfer * LATERAL_RATIO;
return FMath::Clamp(BaseLoad, LOAD_MIN, LOAD_MAX);
```

---

## GAME SETTINGS INTEGRATION

### FMGGameSettings Updated

Added to `MGMenuSubsystem.h`:

```cpp
/**
 * @brief Physics handling mode preset
 *
 * Controls overall physics feel:
 * - Arcade: Forgiving, assisted, great for casual play
 * - Balanced: Default, realistic but accessible
 * - Simulation: Challenging, minimal assists, for enthusiasts
 */
UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
EMGPhysicsHandlingMode PhysicsHandlingMode = EMGPhysicsHandlingMode::Balanced;
```

This allows the handling mode to be:
- Saved with player settings
- Changed from the game options menu
- Persisted across sessions

---

## ARCHITECTURE DIAGRAM

```
┌─────────────────────────────────────────────────────────────────┐
│                       Game Settings                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ FMGGameSettings                                          │   │
│  │   PhysicsHandlingMode = EMGPhysicsHandlingMode::Balanced │   │
│  └──────────────────────────┬──────────────────────────────┘   │
└─────────────────────────────┼───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   UMGPhysicsHandlingConfig                       │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│  │ GetArcadeSettings│ │GetBalancedSettings│ │GetSimulationSettings│
│  │ - High assists  │ │ - Moderate assists│ │ - No assists    │   │
│  │ - More grip     │ │ - Standard grip  │ │ - Realistic     │   │
│  │ - No turbo lag  │ │ - Partial effects│ │ - Full effects  │   │
│  └────────┬────────┘ └────────┬─────────┘ └────────┬────────┘   │
└───────────┼───────────────────┼──────────────────────┼──────────┘
            │                   │                      │
            └───────────────────┼──────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│              UMGVehicleMovementComponent                         │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ ApplyPhysicsHandlingSettings(Settings)                    │ │
│  │   - StabilityControl = Settings.StabilityControl          │ │
│  │   - WeightTransferRate = Settings.WeightTransferRate      │ │
│  │   - BaseTireGrip = Settings.BaseTireGrip                  │ │
│  │   - TurboLagSimulation = Settings.TurboLagSimulation      │ │
│  │   - Update all wheel friction multipliers                 │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  Physics calculations now use MGPhysicsConstants::*            │
│  instead of magic numbers                                       │
└─────────────────────────────────────────────────────────────────┘
```

---

## BENEFITS

### For Designers
1. **Easy Mode Switching**: One function call changes entire physics feel
2. **Documented Parameters**: Every tuning value has clear documentation
3. **Blueprint Accessible**: All functions exposed to Blueprints
4. **Saveable Settings**: Handling mode saved with player preferences

### For Programmers
1. **No More Magic Numbers**: All constants documented and named
2. **Single Source of Truth**: MGPhysicsConstants.h for all physics constants
3. **Easy to Modify**: Change a constant in one place, affects entire system
4. **Self-Documenting Code**: Constants explain their purpose

### For Players
1. **Choose Your Challenge**: Arcade for fun, Simulation for realism
2. **Consistent Experience**: Settings persist across sessions
3. **Gradual Learning**: Start with Arcade, progress to Simulation

---

## FILES MODIFIED

| File | Changes |
|------|---------|
| `Public/Vehicle/MGPhysicsConstants.h` | **NEW** - Physics constants and handling presets |
| `Private/Vehicle/MGPhysicsConstants.cpp` | **NEW** - Default settings implementations |
| `Public/Vehicle/MGVehicleMovementComponent.h` | +Include, +handling mode functions, +member vars |
| `Private/Vehicle/MGVehicleMovementComponent.cpp` | +Handling mode implementation, +named constants |
| `Public/UI/MGMenuSubsystem.h` | +Include, +PhysicsHandlingMode setting |

**Lines Changed:** ~450 lines

---

## TESTING STRATEGY

### Unit Test: Mode Switching

```cpp
void TestHandlingModeSwitching()
{
    UMGVehicleMovementComponent* Movement = GetPlayerVehicleMovement();

    // Test Arcade mode
    Movement->ApplyPhysicsHandlingMode(EMGPhysicsHandlingMode::Arcade);
    ASSERT_EQUAL(Movement->GetPhysicsHandlingMode(), EMGPhysicsHandlingMode::Arcade);
    ASSERT_EQUAL(Movement->StabilityControl, 0.7f);
    ASSERT_EQUAL(Movement->BaseTireGrip, 1.2f);

    // Test Simulation mode
    Movement->ApplyPhysicsHandlingMode(EMGPhysicsHandlingMode::Simulation);
    ASSERT_EQUAL(Movement->StabilityControl, 0.0f);
    ASSERT_EQUAL(Movement->TireTempGripInfluence, 1.0f);
}
```

### Integration Test: Settings Persistence

```cpp
void TestSettingsPersistence()
{
    UMGMenuSubsystem* Menu = GetMenuSubsystem();

    // Change handling mode
    FMGGameSettings Settings = Menu->GetSettings();
    Settings.PhysicsHandlingMode = EMGPhysicsHandlingMode::Simulation;
    Menu->ApplyAndSaveSettings(Settings);

    // Reload settings
    FMGGameSettings LoadedSettings = Menu->GetSettings();
    ASSERT_EQUAL(LoadedSettings.PhysicsHandlingMode, EMGPhysicsHandlingMode::Simulation);
}
```

---

## NEXT STEPS

### Iteration 87 Recommendations

1. **Weather System Unification** - Address dual weather architecture
2. **AI System Refinement** - Test mood/learning system integration
3. **Economy System Validation** - Test transaction edge cases
4. **Performance Profiling** - Add markers to hot code paths

### Future Enhancements

1. **Per-Vehicle Presets**: Allow vehicles to have different default handling modes
2. **Dynamic Mode Blending**: Interpolate between modes based on player skill
3. **Mode-Specific Achievements**: Unlock achievements for mastering Simulation mode

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - System Refinement
**Priority:** P0 (High - Physics tuning per PRD Section 3.2 Gap 2)
**Type:** Physics System Refinement

---

## MILESTONE: DESIGNER-FRIENDLY PHYSICS

**Iteration 86 delivered:**
- 2 new source files (physics constants system)
- 6 documented constant namespaces
- 3 physics handling mode presets
- 4 new Blueprint-callable functions
- 1 new game setting option
- All weight transfer magic numbers documented

The vehicle physics system is now designer-friendly with clear presets and documented parameters.

---
