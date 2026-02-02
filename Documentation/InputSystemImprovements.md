# Midnight Grind - Input System Improvements for Competitive Racing
## Overview

This document outlines the enhancements made to the input response system to achieve **tight, responsive controls** suitable for competitive racing. The focus is on **immediate feedback**, **precise control**, and **competitive-level responsiveness**.

---

## Key Improvements

### 1. **Advanced Response Curves**

#### Throttle Response Curves
- **Linear (Default)**: Direct 1:1 mapping - best for experienced players
- **Progressive**: Gentle at low inputs, aggressive at high - good for beginners
- **Aggressive**: Quick response even at low inputs - for sensitive control
- **S-Curve**: Smooth acceleration with precision in mid-range

#### Brake Response Curves
- **Linear**: Consistent brake pressure
- **Front-Loaded**: More immediate braking for late brakers
- **Trail Braking**: Gentler initial pressure, allows modulation

#### Steering Response Curves
- **Linear**: Direct steering
- **Progressive**: More precision at center, sharp at edges
- **Aggressive**: Quick turn-in response
- **Custom Exponential**: Tunable for player preference

### 2. **Keyboard-Specific Enhancements**

Keyboard inputs are binary (on/off), which makes analog control difficult. Improvements:

- **Pseudo-Analog Simulation**: Smooth ramp-up/down when pressing keys
- **Adjustable Ramp Speed**: Customizable response time (50-300ms)
- **Tap Detection**: Quick taps = gentle inputs, holds = full input
- **Counter-Input Priority**: Opposite direction inputs cancel quickly

### 3. **Input Smoothing & Anti-Deadzone**

#### Smart Smoothing
- **Adaptive Interpolation**: Faster for quick inputs, slower for gentle ones
- **Input Prediction**: Slight anticipation of input direction
- **Jitter Reduction**: Filters out noise from worn controllers

#### Anti-Deadzone Processing
- **Deadzone Shaping**: Radial vs. axial deadzones for sticks
- **Edge Calibration**: Ensures full range is usable
- **Outer Deadzone**: Prevents false max values from worn controllers

### 4. **Frame-Perfect Input Buffering**

Enhanced the existing buffer system:

- **Reduced Buffer Window**: 100ms default (was 150ms) for competitive feel
- **Priority System**: Critical inputs (shift, brake) processed first
- **Input Queuing**: Multiple rapid inputs queued properly
- **Frame-Accurate Timestamps**: Ensures consistent 60/120/144fps input

### 5. **Force Feedback Integration**

Complete implementation of FFB for enhanced feedback:

- **Tire Slip Feedback**: Feel when tires lose grip
- **Road Surface**: Different textures for different surfaces
- **Weight Transfer**: Feel chassis load during braking/cornering
- **Impact Response**: Collisions and curb hits
- **Engine Vibration**: RPM-based rumble at high revs

### 6. **Controller Auto-Detection**

Improved detection and switching:

- **Automatic Switching**: Seamlessly switches between KB/M, Gamepad, Wheel
- **Hot-Plug Support**: Detects controllers being plugged in mid-session
- **Per-Device Settings**: Different sensitivity for different controllers
- **Device-Specific UI**: Shows appropriate button prompts instantly

### 7. **Competitive-Focused Features**

#### Input Display
- **Real-time Input Viewer**: Shows exact input values for replays/streaming
- **Input History**: Visual trail of recent inputs
- **Input Overlay**: Minimal on-screen display for practice

#### Input Analysis
- **Input Smoothness Score**: Measures input consistency
- **Correction Detection**: Tracks steering corrections per lap
- **Optimal Input Suggestion**: Hints for smoother driving

#### Accessibility Options
- **One-Handed Mode**: Remap all controls to one side
- **Steering Assist Levels**: 0-100% adjustable
- **Auto-Shift**: Optional automatic transmission
- **Brake Assist**: ABS-like behavior for casual players

---

## Implementation Files

### New/Modified Files

1. **MGInputResponseCurves.h/.cpp** - Response curve implementations
2. **MGKeyboardInputSimulator.h/.cpp** - Keyboard pseudo-analog system
3. **MGInputSmoothing.h/.cpp** - Advanced smoothing algorithms
4. **MGForceFeedbackManager.h/.cpp** - Comprehensive FFB system
5. **MGInputAnalytics.h/.cpp** - Input analysis and metrics
6. **MGVehicleInputHandler.cpp** - Enhanced with new features

---

## Configuration Presets

### Competitive/Pro
```cpp
- Deadzone: 0.05 (minimal)
- Sensitivity: 1.0 (1:1)
- Linearity: 1.0 (linear)
- Smoothing: 0.0 (immediate)
- Speed-Sensitive Steering: Disabled
- All Assists: Disabled
- Buffer Window: 100ms
```

### Balanced
```cpp
- Deadzone: 0.10
- Sensitivity: 1.1
- Linearity: 1.5 (slight curve)
- Smoothing: 0.3 (gentle)
- Speed-Sensitive Steering: Enabled (50%)
- Counter-Steer Assist: 30%
- Buffer Window: 120ms
```

### Casual/Accessible
```cpp
- Deadzone: 0.15 (forgiving)
- Sensitivity: 1.3
- Linearity: 2.0 (heavy curve)
- Smoothing: 0.5 (significant)
- Speed-Sensitive Steering: Enabled (70%)
- Steering Assist: 50%
- Counter-Steer Assist: 60%
- Brake Assist: Enabled
- Buffer Window: 150ms
```

---

## Testing Recommendations

### Input Latency Testing
1. Use high-speed camera to measure button press to screen response
2. Target: <50ms total input latency at 60fps
3. Target: <33ms total input latency at 120fps

### Feel Testing
1. **Slalom Test**: Quick left-right transitions
2. **Brake Zone Test**: Late braking consistency
3. **Hairpin Test**: Tight corner precision
4. **Highway Weave**: High-speed lane changes

### Controller Comparison
- Test same lap with KB/M, Xbox controller, PS5 controller, Racing Wheel
- Ensure all feel responsive and fair
- No input method should have unfair advantage

---

## Performance Considerations

- All input processing runs in **TG_PrePhysics** tick group
- Maximum 0.5ms CPU time per frame for input processing
- No dynamic memory allocation in hot paths
- Circular buffers for zero-copy input history
- SIMD optimization for smoothing calculations (optional)

---

## Future Enhancements

1. **AI Input Learning**: ML model learns optimal inputs from top players
2. **Haptic Triggers**: PS5 DualSense adaptive trigger support
3. **Motion Controls**: Gyro aiming for camera control
4. **Mobile Touch Controls**: For potential mobile port
5. **Accessibility Controller Support**: Xbox Adaptive Controller

---

## References

- Forza Horizon 5 - Input handling analysis
- iRacing - Force feedback implementation
- Gran Turismo 7 - Controller feel study
- Dirt Rally 2.0 - Wheel feedback reference

---

**Document Version**: 1.0  
**Last Updated**: 2024  
**Author**: OpenClaw AI - Input System Specialist
