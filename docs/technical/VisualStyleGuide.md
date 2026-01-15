# MIDNIGHT GRIND - Visual Style Implementation Guide

## PS1/PS2 Era Aesthetic Technical Reference

This document provides technical guidance for implementing the MIDNIGHT GRIND visual style in Unreal Engine 5. The goal is to achieve an authentic PS1/PS2 era look while leveraging modern rendering capabilities for the neon-soaked midnight car culture aesthetic.

---

## Table of Contents

1. [Overview](#overview)
2. [Core Visual Effects](#core-visual-effects)
3. [Implementation Guide](#implementation-guide)
4. [Material Setup](#material-setup)
5. [Post-Process Configuration](#post-process-configuration)
6. [Performance Considerations](#performance-considerations)
7. [Presets and Settings](#presets-and-settings)

---

## Overview

### Target Aesthetic

MIDNIGHT GRIND combines:
- **PS1/PS2 Era Rendering** - Vertex wobble, affine texture mapping, limited color palette, dithering
- **Neon-Soaked Night Vibes** - Hot pink accents, electric blue highlights, dark atmospheric environments
- **Street Racing Culture** - Chrome reflections, underglow effects, urban environments

### Key Visual Pillars

1. **Authentic Retro Feel** - Not just a filter, but fundamental rendering changes
2. **Midnight Atmosphere** - Dark environments with vibrant neon punctuation
3. **Readable Gameplay** - Retro effects should enhance, not hinder gameplay
4. **Scalable Quality** - Players can adjust retro intensity to preference

---

## Core Visual Effects

### 1. Resolution Scaling

**What it does:** Renders the scene at a lower resolution, then upscales

**PS1/PS2 Reference:**
- PS1: 256x224 to 640x480
- PS2: 512x448 typical

**Implementation:**
```cpp
// In MGRetroRenderingSettings.h
float ResolutionScale = 0.5f;  // Renders at 50% resolution
bool bPointFilterUpscale = true;  // Nearest-neighbor for chunky pixels
```

**Recommended Settings:**
| Preset | Resolution Scale | Upscale Filter |
|--------|-----------------|----------------|
| Subtle | 1.0 (native) | N/A |
| Medium | 0.75 | Bilinear |
| Authentic | 0.5 | Point |
| Extreme | 0.25 | Point |

---

### 2. Vertex Snapping (PS1 Wobble)

**What it does:** Snaps vertex positions to a grid, causing the characteristic "wobbling" polygons

**Why PS1 did this:** Fixed-point math with limited precision

**Implementation:**
```hlsl
// In MGPS1VertexEffects.usf
float3 SnapToGrid(float3 WorldPosition, float GridSize)
{
    return round(WorldPosition / GridSize) * GridSize;
}
```

**Material Setup:**
1. Create Material Function `MF_PS1VertexSnap`
2. Add to World Position Offset in all meshes
3. Use Material Parameter Collection for global control

**Grid Size Guide:**
| Grid Size | Effect | Use Case |
|-----------|--------|----------|
| 256+ | Very subtle | Subtle preset |
| 160 | Authentic PS1 | Default |
| 100 | Heavy wobble | Extreme preset |
| 64 | Very heavy | Stylized scenes |

---

### 3. Affine Texture Mapping

**What it does:** Removes perspective correction from texture mapping, causing textures to "swim" on polygons

**Why PS1 did this:** No hardware texture perspective correction

**Implementation:**
```hlsl
// Vertex shader prepares affine data
float3 AffineUV = float3(UV * Depth, Depth);

// Pixel shader would NOT divide by depth (causing warping)
// In practice, UE5 always does perspective correction, so we fake it
```

**Note:** True affine mapping requires custom vertex/pixel shaders. In UE5, we simulate this with:
- Texture coordinate snapping
- Per-vertex UV interpolation in material
- Custom shader nodes

---

### 4. Color Quantization & Dithering

**What it does:** Reduces color palette and applies ordered dithering to smooth gradients

**PS1/PS2 Color Depth:**
- PS1: 15-bit color (32,768 colors) or 24-bit
- PS2: 16-bit or 32-bit

**Implementation:**
```hlsl
// From MGRetroPostProcess.usf
float3 QuantizeWithDither(float3 Color, float Levels, float2 ScreenPos,
                          float DitherIntensity, float DitherSpread)
{
    float Dither = GetBayerDither4x4(ScreenPos);
    float3 DitheredColor = Color + (Dither - 0.5) * DitherSpread * DitherIntensity;
    return QuantizeColor(saturate(DitheredColor), Levels);
}
```

**Bayer Dithering Matrix (4x4):**
```
 0  8  2 10
12  4 14  6
 3 11  1  9
15  7 13  5
```

**Color Levels Guide:**
| Levels | Bits | Look |
|--------|------|------|
| 256 | 8-bit | Modern |
| 64 | 6-bit | PS2-like |
| 32 | 5-bit | PS1 authentic |
| 16 | 4-bit | Very retro |

---

### 5. CRT Simulation

**Components:**
- **Scanlines** - Horizontal dark lines
- **Curvature** - Barrel distortion at edges
- **Phosphor Glow** - Bloom on bright areas
- **Shadow Mask** - RGB phosphor pattern
- **Chromatic Aberration** - Color fringing at edges

**Implementation:**
```hlsl
// Scanlines
float Scanline = sin(ScreenPos.y * 3.14159 * Scale) * 0.5 + 0.5;
Color *= 1.0 - (Intensity * (1.0 - pow(Scanline, 1.5)));

// Curvature
float2 Curved = UV * 2.0 - 1.0;
float2 Offset = Curved.yx * Curved.yx * Curvature;
Curved = Curved + Curved * Offset;
```

---

### 6. Neon Glow (MIDNIGHT GRIND Signature)

**What it does:** Applies selective bloom to saturated, bright colors (neons, taillights, underglow)

**Implementation:**
```hlsl
float GetNeonMask(float3 Color, float Threshold)
{
    float Saturation = (max(Color) - min(Color)) / (max(Color) + 0.001);
    float Brightness = dot(Color, float3(0.299, 0.587, 0.114));
    return smoothstep(Threshold * 0.5, Threshold, Saturation * Brightness);
}
```

**Signature Colors:**
| Color | RGB | Use |
|-------|-----|-----|
| Hot Pink | (1.0, 0.0, 0.5) | Primary accent |
| Electric Blue | (0.0, 0.5, 1.0) | Secondary accent |
| Neon Purple | (0.5, 0.0, 1.0) | Tertiary |
| Midnight | (0.05, 0.02, 0.1) | Background |
| Chrome | (0.8, 0.85, 0.9) | Metallic |
| Sodium Orange | (1.0, 0.6, 0.2) | Street lamps |

---

## Implementation Guide

### Step 1: Create Material Parameter Collection

Create `MPC_RetroRendering` with these parameters:

**Scalar Parameters:**
- `EffectsEnabled` (0 or 1)
- `ResolutionScale` (0.25 - 1.0)
- `VertexSnapGridSize` (32 - 512)
- `VertexJitterIntensity` (0 - 1)
- `ColorLevels` (4 - 256)
- `DitherIntensity` (0 - 1)
- `ScanlineIntensity` (0 - 1)
- `Time` (auto-updated)

**Vector Parameters:**
- `FogColor`
- `NeonTint`

### Step 2: Create Post-Process Material

1. Create new Material, set Domain to "Post Process"
2. Add Custom node with shader code from `MGRetroPostProcess.usf`
3. Connect Material Parameter Collection inputs
4. Set Blendable Location to "Before Tonemapping"

### Step 3: Create PS1 Vertex Material Function

1. Create Material Function `MF_PS1Vertex`
2. Implement vertex snapping logic
3. Output to World Position Offset
4. Use in all world meshes

### Step 4: Configure Post-Process Volume

1. Place Post-Process Volume in level
2. Set to Infinite Extent (Unbound)
3. Add Retro Post-Process Material to Blendables
4. Configure other settings (Bloom, Exposure) to complement

### Step 5: Add MGRetroRenderingComponent

1. Attach to Game Mode or Camera
2. Configure preset or custom settings
3. Component will update Material Parameter Collection

---

## Material Setup

### Vehicle Materials

**Base Car Paint:**
```
- Use Subsurface profile for metallic paint depth
- Apply vertex snapping in WPO
- Use lower mip levels for retro texture feel
- Add fresnel-based chrome edge highlights
```

**Neon/Underglow:**
```
- Emissive material with high intensity
- Use Material Parameter Collection for color
- Apply subtle pulse animation
- Tag for neon glow post-process
```

### Environment Materials

**Asphalt:**
```
- Use detail normal with reduced intensity
- Wet variation with simple reflection
- Apply vertex snap at lower intensity
```

**Buildings:**
```
- Simple flat colors or low-res textures
- Strong vertex snapping for PS1 feel
- Limited detail in distance (LOD)
```

---

## Post-Process Configuration

### Recommended Post-Process Settings

**For Authentic PS1 Feel:**
```
Bloom:
  - Method: Standard
  - Intensity: 0.3
  - Threshold: 0.8

Exposure:
  - Min/Max Brightness: 0.8 / 1.2
  - Auto-Exposure off (fixed)

Color Grading:
  - Slight desaturation (0.9)
  - Lifted shadows (for visibility)
  - Crushed highlights

Motion Blur: OFF (PS1 didn't have it)

Depth of Field: OFF or minimal (optional)
```

### Post-Process Material Blend Order

1. **Neon Glow Pass** - Selective bloom on neon colors
2. **Resolution Downscale** - Render at lower res
3. **Color Quantization** - Reduce palette with dithering
4. **CRT Effects** - Scanlines, curvature, chromatic aberration
5. **Vignette & Noise** - Final touches

---

## Performance Considerations

### Optimization Tips

1. **Resolution Scaling** - Actually improves performance
2. **Vertex Snapping** - Minimal GPU cost
3. **Post-Process** - Single pass, optimized for PS4/Xbox One
4. **Color Quantization** - Very cheap (just math)
5. **Dithering** - Texture lookup, cheap

### Target Performance

| Platform | Target | Notes |
|----------|--------|-------|
| PC | 60 FPS | Scale resolution for high refresh |
| Console | 60 FPS | Lower retro resolution helps |
| Steam Deck | 40-60 FPS | Aggressive resolution scaling |

### Quality Scalability

```cpp
// In graphics options
enum class ERetroQuality
{
    Performance,  // Extreme preset, 0.25 scale
    Balanced,     // Authentic preset, 0.5 scale
    Quality,      // Medium preset, 0.75 scale
    Modern        // Subtle preset, 1.0 scale
};
```

---

## Presets and Settings

### Preset: Subtle

*For players who want a modern look with light retro touches*

- Resolution: Native (1.0)
- Vertex Snap: Off
- Color Levels: 128
- Dithering: None
- CRT: Off
- Neon Glow: On

### Preset: Medium

*Balanced PS2-era look*

- Resolution: 0.75 (bilinear)
- Vertex Snap: Off
- Vertex Jitter: 0.15
- Color Levels: 64
- Dithering: Bayer 8x8 (0.3)
- CRT: Light scanlines (0.2)
- Neon Glow: On (1.2x)

### Preset: Authentic (DEFAULT)

*True PS1 experience - THE TARGET LOOK*

- Resolution: 0.5 (point filter)
- Vertex Snap: On (160 grid)
- Vertex Jitter: 0.3
- Color Levels: 32 (5-bit)
- Dithering: Bayer 4x4 (0.5)
- CRT: Scanlines (0.35), Curvature (0.1)
- Chromatic Aberration: 0.2
- Phosphor Glow: On
- Vignette: On (0.3)
- Distance Fog: On
- Neon Glow: On (1.5x)
- Light Streaks: On

### Preset: Extreme

*Maximum lo-fi style*

- Resolution: 0.25 (point filter)
- Vertex Snap: On (100 grid)
- Vertex Jitter: 0.5
- Color Levels: 16 (4-bit)
- Dithering: Bayer 4x4 (0.7)
- CRT: Heavy (composite style)
- All effects maximized

---

## Appendix: File Reference

### Source Files

| File | Purpose |
|------|---------|
| `MGRetroRenderingSettings.h` | C++ configuration and component |
| `MGRetroRenderingSettings.cpp` | Implementation |
| `MGRetroPostProcess.usf` | Post-process shader code |
| `MGPS1VertexEffects.usf` | Vertex shader effects |

### Material Assets to Create

- `MI_RetroPostProcess` - Post-process material instance
- `MF_PS1VertexSnap` - Vertex snapping function
- `MF_PS1AffineUV` - Affine texture mapping
- `MPC_RetroRendering` - Parameter collection
- `MI_NeonGlow` - Neon/emissive material

---

*Document Version: 1.0*
*Last Updated: Pre-Production Phase*
*Engine Version: Unreal Engine 5.x*
