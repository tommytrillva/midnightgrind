# Quick Start: NFSU Arcade Mode Tuning

**Time to implement: 30-60 minutes for basic feel**

---

## Immediate Changes (Do This First!)

### Step 1: Update Arcade Mode Constants (5 minutes)

**File:** `Source/MidnightGrind/Private/Vehicle/MGPhysicsConstants.cpp`

Find `UMGPhysicsHandlingConfig::GetArcadeSettings()` and change these values:

```cpp
// OLD → NEW
Settings.StabilityControl = 0.7f;         → 0.85f
Settings.AntiFlipTorque = 15000.0f;       → 25000.0f
Settings.WeightTransferRate = 4.0f;       → 2.0f
Settings.BaseTireGrip = 1.2f;             → 1.4f
Settings.EngineBrakingMultiplier = 0.5f;  → 0.3f
Settings.ArcadeSteeringSpeed = 8.0f;      → 12.0f
Settings.ArcadeSteeringReturnSpeed = 12.0f; → 18.0f
```

**Impact:** Car immediately feels more planted, responsive, and forgiving.

### Step 2: Test in Editor (10 minutes)

1. Compile project (Ctrl+Shift+B in Visual Studio)
2. Open Unreal Editor
3. Load test vehicle
4. Set Physics Handling Mode to "Arcade" in vehicle details
5. Test drive:
   - High-speed cornering (should not spin out)
   - Sharp turns (should feel responsive)
   - Handbrake turns (should slide smoothly)

### Step 3: Adjust to Taste (15-30 minutes)

Use these console commands in-editor for live tuning:

```
// In-game console (~ key):
mg.ArcadeGrip 1.5          // Increase grip
mg.ArcadeGrip 1.3          // Decrease grip
mg.StabilityControl 0.9    // More stability
mg.StabilityControl 0.7    // Less stability
```

Add these console variables to `MGPhysicsConstants.cpp`:

```cpp
static TAutoConsoleVariable<float> CVarArcadeGrip(
    TEXT("mg.ArcadeGrip"),
    1.4f,
    TEXT("Base tire grip in arcade mode"),
    ECVF_Cheat
);

static TAutoConsoleVariable<float> CVarStability(
    TEXT("mg.StabilityControl"),
    0.85f,
    TEXT("Stability control strength"),
    ECVF_Cheat
);
```

Then use these values in `GetArcadeSettings()`:

```cpp
Settings.BaseTireGrip = CVarArcadeGrip.GetValueOnGameThread();
Settings.StabilityControl = CVarStability.GetValueOnGameThread();
```

---

## Target Feel Checklist

After tuning, your arcade mode should:

- ✅ **Corner at 100+ MPH** without spinning out
- ✅ **Handbrake turn 180°** easily with controller
- ✅ **Recover from wall hits** within 0.5 seconds
- ✅ **Drift through S-curves** without losing control
- ✅ **Never flip over** during normal gameplay
- ✅ **Feel snappy** on steering inputs
- ✅ **Maintain speed** through corners better than sim mode

---

## Quick Parameter Reference

| Parameter | Arcade | Balanced | Sim | What It Does |
|-----------|--------|----------|-----|--------------|
| **BaseTireGrip** | 1.4 | 1.0 | 1.0 | Overall grip level |
| **StabilityControl** | 0.85 | 0.3 | 0.0 | Auto-correction strength |
| **WeightTransferRate** | 2.0 | 8.0 | 12.0 | How fast weight shifts |
| **ArcadeSteeringSpeed** | 12.0 | 5.0 | 3.0 | Turn-in response |
| **AntiFlipTorque** | 25000 | 5000 | 0 | Rollover prevention |

### Adjustment Guidelines:

**Too Grippy?** Decrease BaseTireGrip to 1.3
**Too Loose?** Increase BaseTireGrip to 1.5
**Too Twitchy?** Decrease StabilityControl to 0.7
**Too Stable?** Increase StabilityControl to 0.9
**Too Sluggish?** Increase ArcadeSteeringSpeed to 15.0
**Too Sensitive?** Decrease ArcadeSteeringSpeed to 10.0

---

## Advanced: Arcade Enhancements Component

For drift assist and collision bounce (Phase 1.3-1.4), add this to your vehicle Blueprint:

### Blueprint Setup

1. Open vehicle Blueprint (e.g., `BP_VehicleBase`)
2. Add Component → Search "MG Vehicle Arcade Enhancements"
3. Configure in Details panel:
   - ✅ Enable Arcade Mode
   - Drift Assist Strength: 0.7
   - Collision Bounce Mult: 1.5
   - Ideal Drift Angle: 30°

### Testing Drift Assist

1. Drive at 40+ MPH
2. Turn hard + handbrake
3. Car should automatically maintain ~30° drift angle
4. Release handbrake to exit drift

If drift angle won't hold:
- Increase "Drift Assist Strength" to 0.8-0.9
- Lower "Ideal Drift Angle" to 25°

---

## Collision Testing

### Test Scenarios:

1. **Head-on wall at 60 MPH:**
   - Should bounce off, not stop dead
   - Should maintain ~30 MPH after bounce
   - Should auto-orient upright

2. **Side-scrape at 80 MPH:**
   - Should slide along wall
   - Minimal speed loss
   - Should bounce away when wall ends

3. **180° wall impact:**
   - Should reverse direction
   - Should not get stuck
   - Should maintain ~20 MPH

### If collisions feel wrong:

**Too punishing?**
- Increase `CollisionBounceMultiplier` to 2.0
- Reduce damage multiplier in arcade mode

**Too bouncy?**
- Decrease `CollisionBounceMultiplier` to 1.2
- Increase `RecoveryDuration` to 0.7s

---

## Performance Verification

After changes, check performance:

```
stat game    // Should be < 16ms for 60fps
stat unit    // Overall frame time
stat physics // Physics cost
```

**Expected Impact:**
- Arcade mode: ~0.1ms overhead (negligible)
- Drift assist: ~0.05ms when active
- Collision recovery: ~0.02ms during recovery

If performance is worse:
- Disable debug visualizations
- Reduce tick frequency of arcade component
- Check for unnecessary calculations

---

## Common Issues & Fixes

### Issue: Car still spins out in arcade mode

**Cause:** Stability control not strong enough or weight transfer too fast

**Fix:**
```cpp
Settings.StabilityControl = 0.9f;  // Increase
Settings.WeightTransferRate = 1.5f; // Decrease
```

### Issue: Steering feels too slow

**Cause:** Arcade steering speed too low

**Fix:**
```cpp
Settings.ArcadeSteeringSpeed = 15.0f; // Increase
```

### Issue: Drifts won't initiate

**Cause:** Handbrake not strong enough or grip too high

**Fix:**
- Check handbrake torque in vehicle data
- Temporarily reduce `BaseTireGrip` to 1.2
- Ensure `MinDriftAngle` in drift subsystem is 10° or lower

### Issue: Collision damage still too high in arcade

**Cause:** Arcade mode modifier not applied to damage system

**Fix:**
In `MGCollisionSubsystem::ProcessCollision()`:
```cpp
if (bIsArcadeMode)
{
    Event.DamageDealt *= 0.2f;  // Reduce to 20%
    Event.SpeedLoss *= 0.3f;    // Reduce to 30%
}
```

---

## Iteration Workflow

### Day 1: Core Feel
1. ✅ Implement Step 1 changes
2. Test 10 laps on test track
3. Adjust grip and stability
4. Get 3 people to test
5. Note feedback

### Day 2: Drift Feel
1. Add arcade enhancements component
2. Test drift initiation
3. Adjust drift assist strength
4. Test drift transitions (S-curves)
5. Fine-tune angle tolerance

### Day 3: Collision Feel
1. Test wall impacts at various speeds
2. Adjust bounce multiplier
3. Test traffic collisions
4. Fine-tune recovery duration

### Day 4: Polish
1. Create per-vehicle arcade presets
2. Balance different car classes
3. Final playtest session
4. Document final values

---

## Success Metrics

**You've nailed the NFSU feel when:**

- Players can drift through entire tracks without spinout
- Wall collisions feel bouncy/fun, not punishing
- Steering feels responsive on gamepad
- New players can complete races without frustration
- Cars feel "planted" and confidence-inspiring
- Drifting feels rewarding, not scary
- Players describe it as "arcadey but not too easy"

---

## Next Steps After Basic Tuning

Once basic arcade feel is good:

1. **Create Vehicle Presets:**
   - Import Tuner preset (nimble)
   - Muscle Car preset (powerful)
   - Exotic preset (balanced)

2. **Add Visual Feedback:**
   - Drift assist indicator HUD
   - Speed boost visual effect
   - Collision recovery screen shake

3. **Advanced Assists:**
   - Auto-brake for corners (optional)
   - Throttle control in turns
   - Gear selection assistance

4. **Multiplayer Balancing:**
   - Ensure assists don't give unfair advantage
   - Consider "Pure Arcade" vs "Assisted Arcade" modes
   - Test against simulation mode players

---

## Support

If something doesn't work or you need help:

1. Check the main report: `VEHICLE_PHYSICS_TUNING_REPORT.md`
2. Review implementation files:
   - `MG_VHCL_ArcadeEnhancements.h/cpp`
   - `MGPhysicsConstants.h/cpp`
3. Test in isolation (single vehicle, empty track)
4. Use debug visualizations (`bShowDebugInfo = true`)
5. Check console for errors/warnings

**Common mistakes:**
- Forgetting to set vehicle to Arcade mode
- Not recompiling after code changes
- Testing with wrong vehicle BP
- Collisions not hooked up to arcade component

---

**End of Quick Start Guide**

*Good luck making it feel awesome! 🏁*
