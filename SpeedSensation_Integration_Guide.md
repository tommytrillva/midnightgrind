# Speed Sensation System - Integration Guide
## Midnight Grind Racing VFX System

---

## 📋 Overview

The Speed Sensation System is a comprehensive, plug-and-play solution that makes speed feel **visceral and intense** in Midnight Grind. It orchestrates camera effects, post-processing, screen overlays, particles, and audio to create an adrenaline-pumping racing experience.

### What You Get

✅ **5 Preset Profiles** - Modern, Arcade, Simulation, Y2K Cyberpunk, Cinematic  
✅ **10 Effect Categories** - FOV, shake, motion blur, speed lines, chromatic aberration, and more  
✅ **Dynamic Scaling** - All effects scale smoothly with velocity (80-300 KPH configurable)  
✅ **Context Awareness** - Boost multipliers, proximity pulses, tunnel amplification  
✅ **Full Blueprint Support** - Configure and control everything from BP  
✅ **Performance Optimized** - Integrates with existing subsystems, no duplicate work  
✅ **Accessibility Friendly** - Global intensity scale, per-category disable options  

---

## 🚀 Quick Start (5 Minutes)

### Step 1: Add Component to Vehicle

**C++ Vehicle:**
```cpp
// In your vehicle header (e.g., AMGVehiclePawn.h)
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
UMGSpeedSensationComponent* SpeedSensation;

// In constructor (e.g., AMGVehiclePawn.cpp)
SpeedSensation = CreateDefaultSubobject<UMGSpeedSensationComponent>(TEXT("SpeedSensation"));
```

**Blueprint Vehicle:**
1. Open your vehicle Blueprint (e.g., `BP_MG_Vehicle`)
2. Add Component → Search "Speed Sensation"
3. Click Add **MGSpeedSensationComponent**

### Step 2: Choose a Profile

**In BeginPlay or Constructor:**
```cpp
SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Y2KCyberpunk);
```

**In Blueprint:**
- Event BeginPlay → Get SpeedSensation Component
- Call **Set Effect Profile**
- Choose from dropdown: Modern / Arcade / Simulation / Y2K Cyberpunk / Cinematic

### Step 3: Done!

That's it! The component will automatically:
- Detect vehicle velocity
- Update all effect systems
- Scale intensity based on speed
- Integrate with existing camera, post-process, and screen effect subsystems

---

## 🎮 Profiles Explained

### Modern (Default)
**Best for:** General play, balanced experience  
**Feel:** Subtle but noticeable speed sensation

- FOV: 90° → 102° at max speed
- Moderate motion blur
- Light speed lines (peripheral only)
- Subtle chromatic aberration
- Clean center of screen maintained

### Arcade
**Best for:** Controller players, casual/action-focused  
**Feel:** Intense, exaggerated, "Need for Speed" style

- FOV: 90° → 110° (dramatic tunnel vision)
- Heavy radial blur
- Anime-style speed lines (full screen)
- Strong chromatic aberration
- Colorful particle trails

### Simulation
**Best for:** Wheel players, realism-focused  
**Feel:** Minimal effects, natural motion

- FOV: 90° → 98° (barely noticeable)
- Natural motion blur only (no radial blur)
- **No** speed lines or particles
- Subtle screen shake
- Realistic camera behavior

### Y2K Cyberpunk
**Best for:** Night racing, neon-lit tracks  
**Feel:** Stylized, glitchy, vaporwave aesthetic

- FOV: 90° → 108°
- Neon-colored speed lines (cyan/magenta)
- Heavy chromatic aberration with RGB split
- Purple vignette tint
- Glitch effects at extreme speeds
- Doppler audio enhancements

### Cinematic
**Best for:** Replays, photo mode, trailers  
**Feel:** Dramatic, film-like

- FOV: 90° → 106°
- Film-grade motion blur
- Heavy vignette (focus attention)
- Minimal speed lines (subtle)
- Lens flare enhancement
- Bloom boost

---

## ⚙️ Advanced Configuration

### Customizing Individual Categories

```cpp
// Reduce FOV change for motion-sensitive players
SpeedSensation->SetCategoryIntensity(EMGSpeedEffectCategory::CameraFOV, 0.5f);

// Disable screen shake entirely
SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ScreenShake, false);

// Amplify speed lines
SpeedSensation->SetCategoryIntensity(EMGSpeedEffectCategory::SpeedLines, 1.5f);
```

### Global Intensity Scale

```cpp
// Reduce ALL effects to 50% (accessibility)
SpeedSensation->SetGlobalIntensityScale(0.5f);

// Boost ALL effects to 150% (intense mode)
SpeedSensation->SetGlobalIntensityScale(1.5f);
```

### Custom Speed Thresholds

```cpp
FMGSpeedSensationConfig CustomConfig = SpeedSensation->GetConfiguration();
CustomConfig.MinSpeedThreshold = 100.0f;  // Start effects at 100 KPH
CustomConfig.MaxSpeedThreshold = 350.0f;  // Max intensity at 350 KPH
SpeedSensation->SetCustomConfiguration(CustomConfig);
```

---

## 🎯 Context-Aware Effects

### NOS/Boost Integration

When player activates nitrous:

```cpp
void AMyVehicle::ActivateNitrous()
{
    // Amplify speed effects during boost
    FMGSpeedBoostParams BoostParams;
    BoostParams.Duration = 2.5f;                     // 2.5 second boost
    BoostParams.FOVMultiplier = 1.4f;                // Extra FOV punch
    BoostParams.MotionBlurMultiplier = 2.0f;         // Double motion blur
    BoostParams.ParticleTrailMultiplier = 5.0f;      // Crazy particle trails
    BoostParams.ChromaticMultiplier = 1.8f;          // RGB fringing
    
    SpeedSensation->ApplySpeedBoost(BoostParams);
}

// Simple version (just global multiplier):
SpeedSensation->BoostIntensity(1.5f, 2.0f);  // 1.5x intensity for 2 seconds
```

### Near-Miss Detection

When player narrowly avoids collision:

```cpp
void AMyVehicle::OnNearMiss(AActor* PassedActor, float Distance)
{
    // Calculate intensity based on how close it was
    float MaxNearMissDistance = 200.0f;  // cm
    float Intensity = 1.0f - (Distance / MaxNearMissDistance);
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Brief intensity spike
    SpeedSensation->TriggerProximityPulse(Intensity, 0.3f);
}
```

### Tunnel/Environment Modulation

When entering tunnels or enclosed spaces:

```cpp
// In trigger volume overlap begin
void ATunnelTrigger::OnEnter(AActor* Vehicle)
{
    AMyVehicle* VehiclePawn = Cast<AMyVehicle>(Vehicle);
    if (VehiclePawn && VehiclePawn->SpeedSensation)
    {
        // Amplify effects in tunnel (sound reverb, enclosed feeling)
        VehiclePawn->SpeedSensation->SetEnvironmentMultiplier(1.6f);
    }
}

// On exit
void ATunnelTrigger::OnExit(AActor* Vehicle)
{
    AMyVehicle* VehiclePawn = Cast<AMyVehicle>(Vehicle);
    if (VehiclePawn && VehiclePawn->SpeedSensation)
    {
        // Back to normal
        VehiclePawn->SpeedSensation->SetEnvironmentMultiplier(1.0f);
    }
}
```

---

## 🎬 Manual Control (Cutscenes/Replays)

### Scripted Speed

For cutscenes where the camera isn't attached to a moving vehicle:

```cpp
// Override speed manually
SpeedSensation->SetManualSpeed(250.0f);  // Simulate 250 KPH

// When done
SpeedSensation->ClearManualSpeed();
```

### Pause/Resume

```cpp
// Pause effects (e.g., during menu)
SpeedSensation->PauseEffects();

// Resume
SpeedSensation->ResumeEffects();
```

---

## 🎨 Blueprint Integration

### Get Current Speed Intensity

Useful for UI elements (speedometer glow, HUD distortion):

```
Get Speed Sensation Component
  ↓
Get Current Speed Intensity  (returns 0-1)
  ↓
Use for Material Parameter, Widget Animation, etc.
```

### Events

The component broadcasts useful events:

**On Speed Intensity Changed:**
```
Event: On Speed Intensity Changed (float Intensity)
  → Fired when intensity changes significantly
  → Use for: HUD updates, audio triggers, particle bursts
```

**On Speed Threshold Crossed:**
```
Event: On Speed Threshold Crossed (bool Entered High Speed)
  → Fired when crossing MinSpeedThreshold
  → Use for: Achievement tracking, gameplay state changes
```

**On Speed Boost Applied:**
```
Event: On Speed Boost Applied (float Multiplier)
  → Fired when ApplySpeedBoost is called
  → Use for: UI notifications, sound effects
```

---

## 🛠️ Settings Menu Integration

### Recommended Settings Sliders

```cpp
// Global Intensity (0.0 - 2.0, default 1.0)
void USettingsMenu::OnEffectIntensityChanged(float Value)
{
    UMGSpeedSensationComponent* SpeedFX = GetPlayerVehicleSpeedSensation();
    if (SpeedFX)
    {
        SpeedFX->SetGlobalIntensityScale(Value);
    }
}

// FOV Intensity (0.0 - 2.0, default 1.0)
void USettingsMenu::OnFOVIntensityChanged(float Value)
{
    SpeedFX->SetCategoryIntensity(EMGSpeedEffectCategory::CameraFOV, Value);
}

// Motion Blur Toggle
void USettingsMenu::OnMotionBlurToggled(bool bEnabled)
{
    SpeedFX->SetCategoryEnabled(EMGSpeedEffectCategory::MotionBlur, bEnabled);
}

// Screen Shake Toggle (motion sickness)
void USettingsMenu::OnScreenShakeToggled(bool bEnabled)
{
    SpeedFX->SetCategoryEnabled(EMGSpeedEffectCategory::ScreenShake, bEnabled);
}

// Chromatic Aberration Toggle
void USettingsMenu::OnChromaticToggled(bool bEnabled)
{
    SpeedFX->SetCategoryEnabled(EMGSpeedEffectCategory::ChromaticAberration, bEnabled);
}
```

### Accessibility Presets

```cpp
// "Reduced Motion" preset
void USettingsMenu::ApplyReducedMotionPreset()
{
    SpeedFX->SetEffectProfile(EMGSpeedSensationProfile::Simulation);
    SpeedFX->SetGlobalIntensityScale(0.5f);
    SpeedFX->SetCategoryEnabled(EMGSpeedEffectCategory::ScreenShake, false);
    SpeedFX->SetCategoryEnabled(EMGSpeedEffectCategory::ChromaticAberration, false);
}

// "Maximum Intensity" preset
void USettingsMenu::ApplyMaxIntensityPreset()
{
    SpeedFX->SetEffectProfile(EMGSpeedSensationProfile::Arcade);
    SpeedFX->SetGlobalIntensityScale(1.5f);
}
```

---

## 🔧 Per-Vehicle Tuning

Different vehicles can have different speed sensation configs:

```cpp
// In your vehicle data asset or constructor
void AMG_TunedSportsCar::BeginPlay()
{
    Super::BeginPlay();
    
    // Sports car: more aggressive effects
    FMGSpeedSensationConfig CustomConfig = SpeedSensation->GetConfiguration();
    CustomConfig.FOVSettings.MaxFOVIncrease = 18.0f;  // More FOV change
    CustomConfig.MinSpeedThreshold = 70.0f;           // Effects start earlier
    CustomConfig.MaxSpeedThreshold = 320.0f;          // Higher top speed threshold
    SpeedSensation->SetCustomConfiguration(CustomConfig);
}

void AMG_HeavyTruck::BeginPlay()
{
    Super::BeginPlay();
    
    // Truck: more subtle, realistic
    SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Simulation);
    SpeedSensation->SetGlobalIntensityScale(0.7f);
}
```

---

## 📊 Performance Considerations

### What's Expensive?

1. **Motion Blur** - GPU post-process (moderate cost)
2. **Particle Trails** - Depends on particle count (can be expensive)
3. **Radial Blur** - GPU post-process (moderate cost)
4. **Chromatic Aberration** - GPU post-process (cheap)
5. **Speed Lines** - GPU overlay (cheap)
6. **FOV Changes** - Essentially free (camera property)
7. **Screen Shake** - Essentially free (transform math)

### Optimization Tips

**Low-End Hardware:**
```cpp
// Use Simulation profile (minimal effects)
SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Simulation);

// Or disable expensive categories
SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::MotionBlur, false);
SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ParticleTrails, false);
```

**High-End Hardware:**
```cpp
// Max everything
SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Arcade);
SpeedSensation->SetGlobalIntensityScale(1.5f);
```

**Dynamic Scaling:**
```cpp
// Based on current framerate
void AMyVehicle::Tick(float DeltaTime)
{
    float CurrentFPS = 1.0f / DeltaTime;
    
    if (CurrentFPS < 45.0f)
    {
        // Drop expensive effects
        SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ParticleTrails, false);
    }
    else if (CurrentFPS > 60.0f)
    {
        // Re-enable
        SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ParticleTrails, true);
    }
}
```

---

## 🐛 Troubleshooting

### Effects Not Working

**Check 1: Component Added?**
```cpp
// Verify component exists
if (SpeedSensation == nullptr)
{
    UE_LOG(LogTemp, Error, TEXT("SpeedSensation component not found!"));
}
```

**Check 2: Subsystems Available?**
```cpp
// The component needs these subsystems
UMGPostProcessSubsystem* PostProcess = GetGameInstance()->GetSubsystem<UMGPostProcessSubsystem>();
UMGScreenEffectSubsystem* ScreenFX = GetGameInstance()->GetSubsystem<UMGScreenEffectSubsystem>();

if (!PostProcess || !ScreenFX)
{
    UE_LOG(LogTemp, Error, TEXT("Required subsystems not initialized!"));
}
```

**Check 3: Effects Paused?**
```cpp
if (SpeedSensation->IsUsingManualSpeed())
{
    // Effects may be paused or using manual speed
    SpeedSensation->ResumeEffects();
    SpeedSensation->ClearManualSpeed();
}
```

**Check 4: Global Intensity Zero?**
```cpp
float Intensity = SpeedSensation->GetGlobalIntensityScale();
if (Intensity <= 0.0f)
{
    SpeedSensation->SetGlobalIntensityScale(1.0f);
}
```

### FOV Not Changing

**Check Camera Component:**
```cpp
// The component needs to find a camera
UCameraComponent* Camera = GetComponentByClass<UCameraComponent>();
if (!Camera)
{
    UE_LOG(LogTemp, Error, TEXT("No camera component found on vehicle!"));
}
```

**Check FOV Category Enabled:**
```cpp
if (!SpeedSensation->IsCategoryEnabled(EMGSpeedEffectCategory::CameraFOV))
{
    SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::CameraFOV, true);
}
```

### Speed Lines Not Appearing

**Check Screen Effect Subsystem:**
```cpp
UMGScreenEffectSubsystem* ScreenFX = GetGameInstance()->GetSubsystem<UMGScreenEffectSubsystem>();
if (ScreenFX)
{
    // Ensure speed effects are enabled
    ScreenFX->SetSpeedEffectsEnabled(true);
}
```

**Check Speed Threshold:**
```cpp
float CurrentSpeed = SpeedSensation->GetCurrentSpeedKPH();
FMGSpeedSensationConfig Config = SpeedSensation->GetConfiguration();

if (CurrentSpeed < Config.MinSpeedThreshold)
{
    UE_LOG(LogTemp, Warning, TEXT("Speed %f below threshold %f"), 
           CurrentSpeed, Config.MinSpeedThreshold);
}
```

---

## 📝 Advanced: Custom Curve Authoring

You can create custom intensity curves for fine-tuned control:

### Create Curve Asset

1. Content Browser → Right Click → Miscellaneous → **Curve**
2. Choose **CurveFloat**
3. Name it `Curve_SpeedFOV`
4. Open and edit keys:
   - Key 0.0 → Value 0.0 (no speed = no effect)
   - Key 0.5 → Value 0.3 (half speed = 30% intensity)
   - Key 1.0 → Value 1.0 (max speed = full intensity)

### Apply Custom Curve

```cpp
FMGSpeedSensationConfig CustomConfig = SpeedSensation->GetConfiguration();
CustomConfig.FOVSettings.FOVCurve = EMGSpeedCurveType::Custom;
CustomConfig.FOVSettings.CustomFOVCurve = LoadObject<UCurveFloat>(nullptr, TEXT("/Game/Curves/Curve_SpeedFOV"));
SpeedSensation->SetCustomConfiguration(CustomConfig);
```

---

## 🎓 Design Tips

### Making Speed Feel Fast

1. **Peripheral Motion > Center Motion**  
   Keep the center of the screen clear. Put speed lines and blur in peripheral vision.

2. **Contrast = Speed**  
   Vignette and focus effects make the player feel like they're cutting through space.

3. **FOV is Power**  
   Subtle FOV changes (8-15°) are enough. More feels "fisheye" and unnatural.

4. **Less is More**  
   Don't enable every effect at max. Choose 3-4 signature effects and balance them.

5. **Context Matters**  
   Speed through a tunnel should feel different than open highway.

6. **Reward Speed**  
   Use boost effects as positive feedback. "You did something cool, here's eye candy!"

### Common Mistakes

❌ **Too much FOV change** - Players get motion sick  
✅ Use 10-15° max, favor other effects

❌ **Speed lines blocking view** - Can't see road ahead  
✅ Use peripheral-only mode, clear center radius

❌ **Effects start too early** - Feels fast at slow speeds  
✅ Set MinSpeedThreshold to actual "fast" speed (100+ KPH)

❌ **No variation** - Same effects all the time  
✅ Use boost, proximity pulses, environment mods

❌ **Forgetting accessibility** - Motion-sensitive players suffer  
✅ Always provide intensity sliders and disable options

---

## 📚 See Also

- **MGCameraVFXComponent.h** - Camera-specific effects
- **MGPostProcessSubsystem.h** - Post-process pipeline
- **MGScreenEffectSubsystem.h** - Screen overlays and speed lines
- **MGVehicleVFXComponent.h** - Particle effects
- **MGEngineAudioComponent.h** - Audio integration

---

## 🎉 You're Done!

You now have a complete, professional-grade speed sensation system. Experiment with profiles, tune per-vehicle, and create your own custom configurations.

**Happy Racing! 🏁**
