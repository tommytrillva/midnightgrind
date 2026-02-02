# Midnight Grind - Dynamic Camera System

## Overview

The Midnight Grind dynamic camera system provides immersive racing cameras with advanced features:

- **Dynamic Chase Camera** - Smooth following with look-ahead prediction
- **Cockpit/Interior Camera** - Realistic first-person view with G-force simulation
- **Cinematic Camera** - Dramatic angles for replays and highlights
- **Collision Avoidance** - Cameras intelligently avoid obstacles
- **Retro Aesthetic** - Y2K/PS1-PS2 visual effects

## Components

### 1. MGDynamicCameraComponent
**Purpose:** Advanced camera controller with predictive behavior

**Features:**
- Look-ahead prediction based on vehicle velocity
- Speed-adaptive positioning (pulls back at high speed)
- Turn-based camera lean and lateral offset
- Environmental collision avoidance
- Smooth interpolation with acceleration curves
- Retro aesthetic effects (vertex jitter, chromatic aberration)

**Usage:**
```cpp
// Add to your vehicle Blueprint or C++ class
UMGDynamicCameraComponent* DynamicCamera = CreateDefaultSubobject<UMGDynamicCameraComponent>(TEXT("DynamicCamera"));

// Configure behavior mode
DynamicCamera->SetBehaviorMode(EMGCameraBehaviorMode::Aggressive);

// Enable specific features
DynamicCamera->SetLookAheadEnabled(true);
DynamicCamera->SetCollisionAvoidanceEnabled(true);
```

### 2. MGChaseCameraComponent
**Purpose:** Specialized chase camera for third-person racing

**Features:**
- Predictive steering anticipation (camera leans into turns)
- Speed-based distance and FOV adjustments
- Terrain-adaptive height (stays above ground)
- Multiple camera styles (Standard, Tight, Cinematic, Action)

**Usage:**
```cpp
// Add to vehicle
UMGChaseCameraComponent* ChaseCamera = CreateDefaultSubobject<UMGChaseCameraComponent>(TEXT("ChaseCamera"));

// Set camera style
ChaseCamera->SetCameraStyle(EMGChaseCameraStyle::Cinematic);

// Configure parameters
FMGChaseCameraConfig Config;
Config.BaseDistance = 700.0f;
Config.BaseHeight = 220.0f;
Config.SpeedFOVBonus = 18.0f;
ChaseCamera->SetChaseCameraConfig(Config);
```

### 3. MGCockpitCameraComponent
**Purpose:** Immersive first-person cockpit camera

**Features:**
- G-force simulation (head movement during acceleration/braking/cornering)
- Speed-based head bob
- Look-to-apex during turns (driver looks into corners)
- Engine and road surface shake
- Configurable realism levels (Stable, Realistic, Arcade)

**Usage:**
```cpp
// Add to vehicle
UMGCockpitCameraComponent* CockpitCamera = CreateDefaultSubobject<UMGCockpitCameraComponent>(TEXT("CockpitCamera"));

// Set movement style
CockpitCamera->SetHeadMovementStyle(EMGHeadMovementStyle::Realistic);

// Fine-tune G-force effects
FMGGForceConfig GForceConfig;
GForceConfig.LongitudinalShiftAmount = 6.0f;
GForceConfig.LateralShiftAmount = 10.0f;
CockpitCamera->SetGForceConfig(GForceConfig);
```

### 4. MGCameraVFXComponent (Existing - Enhanced)
**Purpose:** Camera visual effects and post-processing

**Features:**
- Dynamic FOV based on speed
- Camera shake presets (Light, Medium, Heavy, Drift, NOS)
- Impact flash and judder effects
- Slow-motion support
- Post-process effects (chromatic aberration, vignette, motion blur)

## Camera Modes

### Chase Camera
**Best For:** Standard racing gameplay  
**Characteristics:**
- Follows vehicle from behind and above
- Smooth, predictable movement
- Good visibility of track ahead
- Speed-based distance adjustment

**Configuration Tips:**
- Use `Standard` style for balanced gameplay
- Use `Tight` style for technical tracks requiring precision
- Use `Cinematic` style for dramatic replays
- Use `Action` style for arcade-style excitement

### Cockpit Camera
**Best For:** Immersive sim-racing experience  
**Characteristics:**
- First-person view from driver's seat
- Realistic head movement and G-forces
- Look-to-apex helps judge corner entry
- Most challenging but most rewarding

**Configuration Tips:**
- Use `Realistic` style for sim racers
- Use `Arcade` style for accessibility
- Use `Stable` style for motion-sensitive players

### Bumper Camera
**Best For:** Competitive time attack  
**Characteristics:**
- Low, forward-facing view
- Maximum visibility
- Minimal camera movement
- Best for judging braking points

### Hood Camera
**Best For:** Balance of immersion and visibility  
**Characteristics:**
- View from hood/bonnet of car
- Better track visibility than cockpit
- Still feels connected to vehicle
- Popular in arcade racers

## Integration Guide

### Step 1: Add Components to Vehicle Blueprint

1. Open your vehicle Blueprint (e.g., `BP_PlayerVehicle`)
2. Add components:
   - `MGDynamicCameraComponent`
   - `MGChaseCameraComponent`
   - `MGCockpitCameraComponent`
3. Position cameras appropriately:
   - Chase camera: SpringArm already exists
   - Cockpit camera: Position at driver's head location

### Step 2: Configure Camera Settings

In the vehicle Blueprint's Construction Script or BeginPlay:

```cpp
void AMGVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    // Setup dynamic camera controller
    if (DynamicCameraComponent)
    {
        DynamicCameraComponent->SetBehaviorMode(EMGCameraBehaviorMode::Classic);
        DynamicCameraComponent->SetLookAheadEnabled(true);
    }

    // Setup chase camera
    if (ChaseCameraComponent)
    {
        ChaseCameraComponent->SetCameraStyle(EMGChaseCameraStyle::Standard);
    }

    // Setup cockpit camera
    if (CockpitCameraComponent)
    {
        CockpitCameraComponent->SetHeadMovementStyle(EMGHeadMovementStyle::Realistic);
    }

    // Start with chase camera active
    SetCameraMode(EMGCameraMode::Chase);
}
```

### Step 3: Camera Switching

The existing `SetCameraMode()` and `CycleCamera()` functions work with the new system:

```cpp
void AMGVehiclePawn::SetCameraMode(EMGCameraMode NewMode)
{
    CurrentCameraMode = NewMode;

    // Disable all cameras
    if (ChaseCameraComponent)
        ChaseCameraComponent->SetCameraEnabled(false);
    if (CockpitCameraComponent)
        CockpitCameraComponent->SetCameraEnabled(false);

    // Enable selected camera
    switch (NewMode)
    {
        case EMGCameraMode::Chase:
            if (Camera) Camera->SetActive(true);
            if (ChaseCameraComponent) ChaseCameraComponent->SetCameraEnabled(true);
            break;

        case EMGCameraMode::Interior:
            if (InteriorCamera) InteriorCamera->SetActive(true);
            if (CockpitCameraComponent) CockpitCameraComponent->SetCameraEnabled(true);
            break;

        // ... other camera modes
    }
}
```

### Step 4: Configure Retro Aesthetic (Optional)

For Y2K/PS1-PS2 retro feel:

```cpp
// In DynamicCameraComponent
FMGRetroAestheticConfig RetroConfig;
RetroConfig.bEnabled = true;
RetroConfig.VertexJitterIntensity = 0.08f;
RetroConfig.ChromaticAberration = 0.5f;
RetroConfig.ScanlineIntensity = 0.15f;
DynamicCameraComponent->SetRetroAestheticConfig(RetroConfig);

// Also configure camera VFX for retro post-processing
if (CameraVFX)
{
    CameraVFX->SetChromaticAberration(0.5f);
    // Additional retro effects via post-process materials
}
```

## Behavior Modes Explained

### Classic Mode
- Balanced camera for most players
- Moderate look-ahead
- Smooth, predictable movement
- Good for learning tracks

### Aggressive Mode
- Enhanced look-ahead (2x multiplier)
- Faster interpolation
- More dramatic lean in turns
- Best for experienced players who want anticipation

### Cinematic Mode
- Wide camera angles
- Slower, more dramatic movement
- Increased distance and height at speed
- Perfect for replays and photo mode

### Drift Mode
- Exaggerated lean during drifts (2x multiplier)
- Increased lateral offset
- Follows drift angle closely
- Makes drifting feel more dynamic

### Arcade Mode
- Fixed distance (no speed adaptation)
- Fast, responsive movement
- Minimal look-ahead
- Classic arcade racer feel

### Custom Mode
- Uses your configured parameters
- No preset overrides
- Full manual control

## Advanced Configuration

### Fine-Tuning Look-Ahead

```cpp
FMGCameraLookAheadConfig LookAheadConfig;
LookAheadConfig.bEnabled = true;
LookAheadConfig.DistanceMultiplier = 1.8f;      // How far ahead to look
LookAheadConfig.VerticalOffset = 60.0f;         // Height of look target
LookAheadConfig.InterpSpeed = 4.0f;             // Transition speed
LookAheadConfig.MaxDistance = 1200.0f;          // Maximum look distance
LookAheadConfig.MinSpeedKPH = 80.0f;            // Speed to enable feature

DynamicCameraComponent->SetLookAheadConfig(LookAheadConfig);
```

### Collision Avoidance Tuning

```cpp
FMGCameraCollisionConfig CollisionConfig;
CollisionConfig.bEnabled = true;
CollisionConfig.ResponseType = EMGCameraCollisionResponse::Adaptive;  // Smart blending
CollisionConfig.ProbeRadius = 35.0f;            // Collision detection size
CollisionConfig.RecoverySpeed = 6.0f;           // Return to normal speed
CollisionConfig.MaxPushDistance = 400.0f;       // Maximum camera push

DynamicCameraComponent->SetCollisionConfig(CollisionConfig);
```

### G-Force Customization

```cpp
FMGGForceConfig GForceConfig;
GForceConfig.bEnabled = true;
GForceConfig.LongitudinalShiftAmount = 7.0f;    // Head movement forward/back
GForceConfig.LateralShiftAmount = 10.0f;        // Head movement side-to-side
GForceConfig.VerticalShiftAmount = 4.0f;        // Head compression under G
GForceConfig.ResponseSpeed = 3.5f;              // How quickly head reacts
GForceConfig.MaxGForce = 3.5f;                  // Maximum G-force to simulate

CockpitCameraComponent->SetGForceConfig(GForceConfig);
```

## Performance Considerations

### Optimization Tips

1. **Collision Traces:**
   - The dynamic camera performs line traces for collision avoidance
   - Set `CollisionConfig.bEnabled = false` if performance is critical
   - Use simple collision on environment geometry

2. **Tick Groups:**
   - All camera components tick in `TG_PostPhysics`
   - This ensures they see final vehicle physics state
   - Don't change tick groups unless necessary

3. **Interpolation:**
   - Higher `InterpSpeed` values = more responsive but potentially jittery
   - Lower values = smoother but more laggy
   - Balance based on your game's feel

4. **Retro Effects:**
   - Retro aesthetic effects are lightweight in C++
   - Heavy effects should be in post-process materials
   - Disable if targeting low-end hardware

## Blueprint Exposure

All major functions are Blueprint-callable:

**Events:**
- Call camera configuration from Blueprint
- Create custom camera transitions
- Trigger special camera effects on events (near miss, drift, etc.)

**Example Blueprint Usage:**
```
Event BeginPlay
├─ Set Behavior Mode (Aggressive)
├─ Set Look Ahead Enabled (True)
└─ Set Camera Style (Cinematic)

Event On Near Miss
└─ Trigger Camera Shake (NearMiss)

Event On Drift Started
└─ Set Behavior Mode (Drift)

Event On Drift Ended
└─ Set Behavior Mode (Classic)
```

## Debugging

### Visual Debug Commands

Add console commands for debugging:

```cpp
// In MGDynamicCameraComponent.cpp
#if !UE_BUILD_SHIPPING
void UMGDynamicCameraComponent::TickComponent(...)
{
    // Draw debug look-ahead target
    if (CVarShowCameraDebug.GetValueOnGameThread())
    {
        DrawDebugSphere(GetWorld(), CurrentLookAheadTarget, 50.0f, 12, FColor::Green, false, 0.0f);
    }
}
#endif
```

Console commands:
- `mg.camera.debug 1` - Show camera debug visualization
- `mg.camera.lookahead 1` - Show look-ahead target
- `mg.camera.collision 1` - Show collision traces

## Common Issues & Solutions

### Camera Feels Too Laggy
**Solution:** Increase `PositionLagSpeed` and `RotationLagSpeed` in smoothing config

### Camera Too Jittery
**Solution:** Decrease interp speeds, enable acceleration curve smoothing

### Look-Ahead Not Working
**Solution:** Check that vehicle speed exceeds `MinSpeedKPH` threshold

### G-Forces Too Intense
**Solution:** Reduce G-force amounts in `FMGGForceConfig`, increase `ResponseSpeed` for gentler movement

### Collision Avoidance Causing Issues
**Solution:** Adjust `TraceChannel`, increase `RecoverySpeed`, or disable if not needed

## Future Enhancements

Potential additions:
- [ ] Replay camera paths
- [ ] Photo mode camera (free cam with constraints)
- [ ] Split-screen camera manager
- [ ] VR camera support
- [ ] Dynamic camera cuts based on race events
- [ ] AI director for automatic cinematic angles

## Credits

Developed for Midnight Grind racing game.  
Optimized for Y2K/PS1-PS2 retro aesthetic.  
Built with Unreal Engine 5.7.

---

**Note:** This camera system integrates with existing `MGCameraVFXComponent` for visual effects. Both systems work together to create the complete camera experience.
