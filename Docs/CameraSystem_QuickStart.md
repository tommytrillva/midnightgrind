# Camera System Quick Start Guide

## 5-Minute Setup

### 1. Add Camera Components (Blueprint)

Open `BP_VehicleAdvPawnBase` or your vehicle Blueprint:

1. **Add Components Panel** → Add Component → Search "MGDynamicCameraComponent"
2. **Add Components Panel** → Add Component → Search "MGChaseCameraComponent"
3. **Add Components Panel** → Add Component → Search "MGCockpitCameraComponent"

### 2. Configure Default Settings (Event Graph)

Add this to **Event BeginPlay**:

```
Event BeginPlay
├─ DynamicCameraComponent → Set Behavior Mode (Classic)
├─ DynamicCameraComponent → Set Look Ahead Enabled (True)
├─ DynamicCameraComponent → Set Collision Avoidance Enabled (True)
├─ ChaseCameraComponent → Set Camera Style (Standard)
└─ CockpitCameraComponent → Set Head Movement Style (Realistic)
```

### 3. Test Camera Switching

Press your camera toggle button (default: C or View button)
- Chase → Hood → Bumper → Interior → Cinematic → Chase

### 4. Adjust to Taste

**Too much camera movement?**
- Set Behavior Mode to `Arcade`
- Reduce look-ahead distance multiplier

**Want more dramatic cameras?**
- Set Behavior Mode to `Cinematic`
- Set Chase Camera Style to `Cinematic`

**First-person too intense?**
- Set Head Movement Style to `Stable`
- Reduce G-force amounts

## Recommended Presets

### Casual Racing (Forza Horizon style)
```cpp
// Dynamic Camera
BehaviorMode: Classic
LookAhead: Enabled
SpeedAdaptive: Enabled

// Chase Camera
Style: Standard
BaseDistance: 600
SpeedFOVBonus: 15

// Cockpit Camera
HeadMovement: Arcade
```

### Simulation Racing (Gran Turismo style)
```cpp
// Dynamic Camera
BehaviorMode: Aggressive
LookAhead: Enabled (1.5x multiplier)
TurnLean: Enabled (8° max)

// Chase Camera
Style: Tight
BaseDistance: 450
TerrainAdaptive: Enabled

// Cockpit Camera
HeadMovement: Realistic
GForce: High sensitivity
LookToApex: Enabled
```

### Arcade Racing (Need for Speed style)
```cpp
// Dynamic Camera
BehaviorMode: Action
LookAhead: Enabled (2x multiplier)
TurnLean: Exaggerated (12° max)

// Chase Camera
Style: Action
SpeedFOVBonus: 20
SteerAnticipation: High

// Cockpit Camera
HeadMovement: Arcade
GForce: Exaggerated
HeadBob: Enhanced
```

### Drift Racing (Initial D style)
```cpp
// Dynamic Camera
BehaviorMode: Drift
TurnLean: Maximum (15°)
DriftLeanMultiplier: 2.0

// Chase Camera
Style: Cinematic
LateralOffset: 120

// Cockpit Camera
HeadMovement: Realistic
GForce: High lateral sensitivity
```

### Retro/Y2K Style (Midnight Club, Tokyo Xtreme Racer)
```cpp
// Dynamic Camera
BehaviorMode: Classic
RetroAesthetic: Enabled
  - VertexJitter: 0.08
  - ChromaticAberration: 0.5
  - Scanlines: 0.15

// Chase Camera
Style: Standard (closer)
BaseDistance: 500
FOV: 95° base

// Post-Process (via MGCameraVFXComponent)
ChromaticAberration: 0.4
ColorReduction: 0.2
Dithering: 0.15
```

## Common Tweaks

### Make Camera More Responsive
1. Increase `PositionLagSpeed` (try 12-15)
2. Increase `RotationLagSpeed` (try 15-20)
3. Increase `LookAheadInterpSpeed` (try 5-6)

### Make Camera Smoother
1. Decrease lag speeds (try 4-6)
2. Enable `UseAccelerationCurve`
3. Increase `MaxVelocity` limit

### Increase Look-Ahead
1. Increase `DistanceMultiplier` (try 2.0-2.5)
2. Increase `MaxDistance` (try 1500)
3. Decrease `MinSpeedKPH` (try 40)

### Reduce Motion Sickness
1. Disable or reduce G-force effects
2. Set HeadMovement to `Stable`
3. Disable head bob
4. Increase camera lag for smoother motion
5. Consider wider FOV (100-105°)

## Testing Checklist

- [ ] Chase camera follows vehicle smoothly
- [ ] Look-ahead activates at speed (look for green sphere if debug enabled)
- [ ] Camera leans during turns
- [ ] Camera avoids collisions with walls/objects
- [ ] Camera pulls back at high speed
- [ ] Cockpit camera shows G-force head movement
- [ ] Head bob appears when moving
- [ ] Look-to-apex turns head during corners
- [ ] Camera switch button cycles through all modes
- [ ] No jarring transitions between cameras
- [ ] FOV changes smoothly with speed
- [ ] Retro effects visible (if enabled)

## Debug Console Commands

Open console (~) and try:

```
mg.camera.debug 1                    // Show all camera debug
mg.camera.lookahead 1                // Show look-ahead target
mg.camera.collision 1                // Show collision traces
mg.camera.gforce 1                   // Show G-force vectors
stat fps                             // Check performance impact
```

## Performance Check

Expected performance cost:
- **Dynamic Camera:** ~0.1ms per frame
- **Chase Camera:** ~0.05ms per frame
- **Cockpit Camera:** ~0.08ms per frame
- **Collision Traces:** ~0.02ms per frame (if enabled)

**Total overhead:** ~0.25ms per vehicle with all features enabled

If performance is critical:
1. Disable collision avoidance
2. Reduce trace frequency
3. Disable retro effects
4. Use simpler behavior modes

## Integration with Existing Systems

### With MGCameraVFXComponent
The new camera components work **alongside** the existing camera VFX:

- **Dynamic/Chase/Cockpit Components:** Handle camera positioning and movement
- **MGCameraVFXComponent:** Handles shake, FOV effects, post-processing

Both systems are complementary and should be used together.

### With Existing Camera Switching
Your existing camera mode enum and switching logic remain unchanged:

```cpp
// This still works!
void AMGVehiclePawn::CycleCamera()
{
    switch (CurrentCameraMode)
    {
        case EMGCameraMode::Chase:
            SetCameraMode(EMGCameraMode::Hood);
            break;
        // ... etc
    }
}
```

Just add enable/disable calls for the new components in `SetCameraMode()`.

## Next Steps

1. ✅ Complete basic setup (above)
2. 📖 Read full documentation: `CameraSystem_README.md`
3. 🎨 Customize camera settings for your game feel
4. 🎥 Create cinematic camera sequences for replays
5. 🏎️ Test with different vehicle types and adjust
6. 🎮 Get player feedback and iterate

## Support

If you encounter issues:

1. Check that components are attached to vehicle
2. Verify camera references are valid (SpringArm, Camera, InteriorCamera)
3. Ensure vehicle movement component is working
4. Enable debug visualization to see what's happening
5. Check console for warning messages

---

**Quick Reference:**

| Feature | Component | Method |
|---------|-----------|--------|
| Look-ahead | Dynamic Camera | `SetLookAheadEnabled()` |
| Camera lean | Dynamic Camera | `SetTurnLeanConfig()` |
| Speed distance | Chase Camera | `SetChaseCameraConfig()` |
| G-forces | Cockpit Camera | `SetGForceConfig()` |
| Collision avoid | Dynamic Camera | `SetCollisionAvoidanceEnabled()` |
| Retro effects | Dynamic Camera | `SetRetroAestheticConfig()` |
| Camera shake | Camera VFX | `TriggerShake()` |
| FOV effects | Camera VFX | `UpdateSpeedEffects()` |
