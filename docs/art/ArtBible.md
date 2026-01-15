# MIDNIGHT GRIND
## Art Bible & Style Guide
### Version 1.0

---

## TABLE OF CONTENTS

1. [Art Direction Overview](#1-art-direction-overview)
2. [Visual Philosophy](#2-visual-philosophy)
3. [Color Theory & Palettes](#3-color-theory--palettes)
4. [Typography](#4-typography)
5. [Vehicle Art Direction](#5-vehicle-art-direction)
6. [Environment Art Direction](#6-environment-art-direction)
7. [Character & NPC Design](#7-character--npc-design)
8. [Lighting Design](#8-lighting-design)
9. [Visual Effects (VFX)](#9-visual-effects-vfx)
10. [UI/UX Visual Design](#10-uiux-visual-design)
11. [Technical Art Guidelines](#11-technical-art-guidelines)
12. [Asset Production Standards](#12-asset-production-standards)

---

## 1. ART DIRECTION OVERVIEW

### 1.1 Visual Identity Statement

MIDNIGHT GRIND captures the raw energy of late 90s/early 2000s street racing culture through a deliberately stylized retro-modern aesthetic. We're not recreating the technical limitations of PS1/PS2 hardware—we're capturing the *feeling* of that era: the saturated neon nights, the chrome-and-speed-lines graphic design, the VHS-tinged memories of racing through rain-slicked streets.

**The Visual Promise:**
> "A night you remember through the haze of nostalgia—more vivid than reality, saturated with possibility, alive with the glow of a thousand neon signs reflected in wet asphalt."

### 1.2 Core Visual Pillars

```
PILLAR 1: NOSTALGIC AUTHENTICITY
├── Era-appropriate design language (1998-2004)
├── Intentional imperfection (not limitation)
├── Memories are more vivid than reality
└── Y2K aesthetic as artistic choice

PILLAR 2: NOCTURNAL ATMOSPHERE
├── Night is the primary time period
├── Artificial light creates the world
├── Contrast and shadow define space
└── Neon is our sun

PILLAR 3: TACTILE MATERIALITY
├── Cars feel real and physical
├── Metal, rubber, glass have presence
├── Damage and wear are visible
└── The world has weight

PILLAR 4: KINETIC ENERGY
├── Speed is visible even when static
├── Motion blur, light trails
├── Dynamic angles and compositions
└── Nothing is ever completely still
```

### 1.3 Reference Board

**Primary References:**

| Category | References | Key Takeaways |
|----------|------------|---------------|
| **Games** | NFS Underground 1&2, Midnight Club 2&3, Tokyo Xtreme Racer | Neon aesthetic, garage UI, car presentation |
| **Film** | Fast & Furious (2001), 2F2F, Tokyo Drift | Color grading, cinematography, car culture authenticity |
| **Anime** | Initial D, Wangan Midnight | Speed lines, dramatic tension, night atmosphere |
| **Design** | Y2K graphic design, 90s import magazines | Typography, chrome effects, aggressive geometry |
| **Photography** | Larry Chen, Speedhunters archive | Authentic car culture documentation |

**Secondary References:**

| Category | References | Key Takeaways |
|----------|------------|---------------|
| **Music Videos** | Late 90s/early 2000s hip-hop, J-pop | Visual style, editing pace, neon usage |
| **Architecture** | Tokyo Shibuya, LA downtown, Miami Beach | Urban density, signage, night lighting |
| **Product Design** | PlayStation 2, Motorola RAZR, early iPod | Era-appropriate industrial design |

---

## 2. VISUAL PHILOSOPHY

### 2.1 The "Memory Filter"

Our visual approach is based on the concept of the "Memory Filter"—the way we remember experiences is never photographically accurate. Memories are:

- **More saturated** than reality
- **Higher contrast** with deeper blacks and brighter highlights
- **Selectively detailed** (important things are sharp, unimportant things blur)
- **Emotionally colored** (warm memories feel warm, exciting memories feel electric)

**Application:**
```
MEMORY FILTER PRINCIPLES
├── Saturation: 1.15-1.30x natural levels
├── Contrast: Deeper blacks, protected highlights
├── Color grading: Warm in interiors, cool-blue for streets
├── Focus: Vehicles sharp, background progressively softer
├── Noise: Subtle film grain adds texture and life
└── Imperfection: Minor aberrations feel authentic
```

### 2.2 The Rule of Cool

When making visual decisions, we apply the "Rule of Cool": if it looks awesome, we find a way to make it work. This doesn't mean abandoning consistency—it means our visual language is designed to accommodate dramatic, stylized choices.

**Examples:**
- Headlights that cast volumetric beams even when physics says they shouldn't
- Underglow that illuminates more area than physically possible
- Neon reflections that appear on surfaces that aren't directly below the source
- Speed lines and motion effects that emphasize velocity

### 2.3 Contrast as Character

The game lives in contrast:

| Dark | Light |
|------|-------|
| Deep shadows | Neon brilliance |
| Black asphalt | Chrome reflections |
| Night sky | City glow |
| Danger | Excitement |

**Contrast ratios should be dramatic.** We avoid muddy mid-tones. Either something is dark or it's lit—this creates the cinematic punch of night racing.

---

## 3. COLOR THEORY & PALETTES

### 3.1 Master Color Philosophy

MIDNIGHT GRIND uses a carefully controlled color palette that evokes the specific era while maintaining readability and visual hierarchy.

**The Night Baseline:**
- Primary darkness is not pure black, but deep blue-black (#0a0a14)
- This allows for "blacker" elements when needed
- Night sky has subtle gradient toward deep purple at horizon

### 3.2 Primary Color Palette

```
NEON ACCENT COLORS (High Energy)
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  ELECTRIC CYAN    NEON PINK       HOT ORANGE    ACID GREEN  │
│  #00f0ff          #ff2d95         #ff6b00       #39ff14     │
│  RGB(0,240,255)   RGB(255,45,149) RGB(255,107,0) RGB(57,255,20) │
│                                                             │
│  Primary racing   Secondary       Warning/Heat   Money/REP  │
│  UI elements      UI elements     Police         Success    │
│                                                             │
└─────────────────────────────────────────────────────────────┘

CORE NEUTRALS
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  NIGHT BLACK     ASPHALT         CONCRETE      STEEL       │
│  #0a0a14         #1a1a24         #2a2a38       #4a4a5c     │
│                                                             │
│  Deep shadow     Road surfaces   Buildings     Metal trim  │
│  Backgrounds     Menus           Walls         UI borders  │
│                                                             │
└─────────────────────────────────────────────────────────────┘

ATMOSPHERIC COLORS
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  DUSK PURPLE     CITY ORANGE     RAIN BLUE     FOG WHITE   │
│  #2a1a3a         #ff8c42         #1a3a5c       #c0c8d4     │
│                                                             │
│  Night sky       Sodium lights   Wet surfaces  Atmosphere  │
│  Distance haze   Industrial      Reflections   Weather     │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 3.3 Vehicle Color Palette

**Factory Colors (40 base options):**

```
CLASSIC COLORS
├── Solid Black (#0a0a0a)
├── Pure White (#f5f5f5)
├── Guards Red (#c40000)
├── Racing Yellow (#ffd700)
├── BRG (British Racing Green) (#004225)
├── Midnight Blue (#191970)
├── Silver Metallic (#c0c0c0)
└── Champagne Gold (#f7e7ce)

JDM ICONIC COLORS
├── Bayside Blue (#2a52be) - R34 GT-R
├── Championship White (#f8f8ff) - Type R
├── Lightning Yellow (#ffc40c) - RX-7
├── Sunset Orange (#ff6f3c) - 350Z
├── Winning Blue (#0057b8) - WRX
├── Apex Silver (#8c92ac) - NSX
├── Sonic Yellow (#fff200) - S2000
└── Milano Red (#8e0028) - Integra

AMERICAN MUSCLE COLORS
├── Grabber Blue (#3399ff)
├── Plum Crazy (#7d0552)
├── Hugger Orange (#ff6600)
├── Sublime Green (#b4d941)
├── Go Mango (#ff8c00)
├── Torred (#ff0800)
├── B5 Blue (#6495ed)
└── Triple Yellow (#ffcc00)

EURO COLORS
├── Laguna Seca Blue (#3c78b5)
├── Imola Red (#c42c36)
├── Estoril Blue (#2a4b8d)
├── Phoenix Yellow (#ffb700)
├── Dakar Yellow (#f4ca16)
├── Papaya Orange (#ff7518)
├── Riviera Blue (#3c6e8f)
└── Oak Green (#3d5e3a)

CUSTOM/WILD COLORS
├── Midnight Purple (#4b0082)
├── Chameleon Green-Purple
├── Candy Apple Red
├── Root Beer Brown
├── Matte Military Green
├── Chrome (special)
├── Satin Black
└── Pearl White
```

### 3.4 UI Color Usage

```
UI COLOR HIERARCHY
├── PRIMARY ACTIONS
│   ├── Confirm/Select: Electric Cyan (#00f0ff)
│   ├── Back/Cancel: Steel Gray (#4a4a5c)
│   └── Special Action: Neon Pink (#ff2d95)
│
├── STATUS INDICATORS
│   ├── Success/Money: Acid Green (#39ff14)
│   ├── Warning: Hot Orange (#ff6b00)
│   ├── Error/Damage: Neon Pink (#ff2d95)
│   ├── Neutral Info: Electric Cyan (#00f0ff)
│   └── Locked/Disabled: Steel Gray (#4a4a5c)
│
├── RACING HUD
│   ├── Speed/Tach: Electric Cyan (#00f0ff)
│   ├── Position: Context-dependent
│   │   ├── 1st: Gold (#ffd700)
│   │   ├── 2nd-3rd: Silver (#c0c0c0)
│   │   └── 4th+: White (#ffffff)
│   ├── Nitrous: Acid Green (#39ff14)
│   └── Heat/Police: Hot Orange → Neon Pink (escalating)
│
├── PART QUALITY TIERS
│   ├── Stock: Gray (#808080)
│   ├── Street: White (#ffffff)
│   ├── Sport: Cyan (#00f0ff)
│   ├── Race: Pink (#ff2d95)
│   ├── Pro: Orange (#ff6b00)
│   └── Legendary: Gold (#ffd700)
│
└── TEXT
    ├── Headers: White (#ffffff)
    ├── Body: Light Gray (#c0c0c0)
    ├── Subtext: Medium Gray (#808080)
    ├── Disabled: Dark Gray (#404040)
    └── Accent: Electric Cyan (#00f0ff)
```

### 3.5 Color Grading LUTs

**Scene-Specific Color Grades:**

| Scene | Grade Name | Characteristics |
|-------|------------|-----------------|
| Downtown | "Electric City" | Cool shadows, warm highlights from sodium lights, boosted neons |
| Industrial | "Grit" | Desaturated except for artificial light, orange-teal contrast |
| Highway | "Velocity" | Blue-shifted, motion-blur friendly, enhanced light trails |
| Canyon | "Mountain Spirit" | Natural tones, cooler overall, fog-enhanced depth |
| Garage | "Chrome Dreams" | Neutral-warm, accurate color for car presentation |
| Rain | "Reflection" | Boosted blues, enhanced specular response |

---

## 4. TYPOGRAPHY

### 4.1 Typography Philosophy

Our typography draws from the aggressive, angular aesthetics of Y2K design—the era of Wipeout, NFS Underground, and import tuner magazines. Text should feel fast, technical, and slightly futuristic (by 2002 standards).

### 4.2 Font Families

```
PRIMARY TYPEFACE: "GRIDNIK" (Custom/Licensed)
├── Usage: Headlines, UI headers, race position
├── Style: Bold, condensed, angular
├── Character: Aggressive, technical, fast
├── Fallback: Eurostile Bold Extended, Industry Bold
│
│  EXAMPLE: "MIDNIGHT GRIND"
│  ███╗   ███╗██╗██████╗ ███╗   ██╗██╗ ██████╗ ██╗  ██╗████████╗
│  ████╗ ████║██║██╔══██╗████╗  ██║██║██╔════╝ ██║  ██║╚══██╔══╝
│  ██╔████╔██║██║██║  ██║██╔██╗ ██║██║██║  ███╗███████║   ██║
│  ██║╚██╔╝██║██║██║  ██║██║╚██╗██║██║██║   ██║██╔══██║   ██║
│  ██║ ╚═╝ ██║██║██████╔╝██║ ╚████║██║╚██████╔╝██║  ██║   ██║
│  ╚═╝     ╚═╝╚═╝╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝

SECONDARY TYPEFACE: "TECH MONO" (Custom/Licensed)
├── Usage: Stats, data, technical readouts
├── Style: Monospace, clean, digital
├── Character: Precise, informational
├── Fallback: OCR-A, Consolas, JetBrains Mono
│
│  EXAMPLE: "HP: 450 | TQ: 380 | WT: 3,200 LBS"

BODY TYPEFACE: "INTER" (Open Source)
├── Usage: Descriptions, menus, dialogue
├── Style: Clean sans-serif, highly legible
├── Character: Modern, readable, neutral
├── Weights: Regular (400), Medium (500), Bold (700)
│
│  EXAMPLE: "Install this turbocharger to increase boost pressure
│           and unlock additional horsepower potential."

ACCENT TYPEFACE: "JAPANESE GOTHIC" (Licensed)
├── Usage: Japanese text, flavor text, Easter eggs
├── Style: Clean Japanese gothic
├── Character: Authentic JDM feel
├── Note: Support for full Japanese character set
```

### 4.3 Typography Scale

```
TYPE SCALE (Based on 8px grid)
├── Display XL:   72px / 80px line-height (Logo, splash)
├── Display:      48px / 56px line-height (Main headers)
├── H1:           32px / 40px line-height (Section headers)
├── H2:           24px / 32px line-height (Subsections)
├── H3:           20px / 28px line-height (Card headers)
├── Body Large:   18px / 28px line-height (Featured text)
├── Body:         16px / 24px line-height (Standard text)
├── Body Small:   14px / 20px line-height (Secondary text)
├── Caption:      12px / 16px line-height (Labels, hints)
└── Micro:        10px / 14px line-height (Fine print)
```

### 4.4 Typography Treatments

```
TEXT EFFECTS (Use sparingly)
├── CHROME TEXT
│   ├── Gradient fill: Silver to white
│   ├── Thin black outline (1px)
│   ├── Usage: Premium items, achievements
│   └── Example: "LEGENDARY"
│
├── NEON TEXT
│   ├── Color fill with glow
│   ├── Blur: 8-16px, same color at 50% opacity
│   ├── Usage: Emphasis, neon signs
│   └── Example: "OPEN 24/7"
│
├── SPEED TEXT
│   ├── Italic + horizontal motion blur
│   ├── Speed lines trailing
│   ├── Usage: Racing results, speed callouts
│   └── Example: "156 MPH"
│
└── GLITCH TEXT
    ├── RGB split / chromatic aberration
    ├── Horizontal displacement
    ├── Usage: Errors, warnings, corruption
    └── Example: "CONNECTION LOST"
```

---

## 5. VEHICLE ART DIRECTION

### 5.1 Vehicle Design Philosophy

Vehicles are the stars of MIDNIGHT GRIND. Every car should feel like a character with history, personality, and potential. Our approach prioritizes recognizability and customization depth over photorealism.

**Design Priorities:**
1. **Silhouette Recognition** - Each car instantly identifiable by shape
2. **Customization Clarity** - Parts should be visually distinct when changed
3. **Material Authenticity** - Metal, glass, rubber read correctly
4. **Damage Readability** - Wear and damage tell a story

### 5.2 Vehicle Modeling Standards

```
POLYGON BUDGETS (Per LOD)
├── LOD0 (Close-up/Garage)
│   ├── Exterior: 80,000-120,000 tris
│   ├── Interior: 30,000-50,000 tris
│   ├── Engine Bay: 20,000-40,000 tris
│   └── Total: ~150,000-200,000 tris
│
├── LOD1 (Racing/Nearby)
│   ├── Exterior: 40,000-60,000 tris
│   ├── Interior: 15,000-25,000 tris
│   └── Total: ~60,000-85,000 tris
│
├── LOD2 (Medium Distance)
│   ├── Exterior: 15,000-25,000 tris
│   ├── Interior: Simplified
│   └── Total: ~20,000-30,000 tris
│
├── LOD3 (Far/Traffic)
│   └── Total: 5,000-10,000 tris
│
└── LOD4 (Distant/Impostor)
    └── Billboard or 1,000 tris

TEXTURE BUDGETS (Per Vehicle)
├── Body: 4096x4096 (Albedo, Normal, ORM)
├── Interior: 2048x2048 (Albedo, Normal, ORM)
├── Engine: 2048x2048 (Albedo, Normal, ORM)
├── Wheels: 1024x1024 per wheel design
├── Lights: 512x512 (Emissive maps)
└── Decals: 2048x2048 (Shared atlas)
```

### 5.3 Vehicle Material System

```
VEHICLE MATERIAL LAYERS
├── BODY PAINT
│   ├── Base Color (from player selection)
│   ├── Metallic (0.0-1.0, paint type dependent)
│   ├── Roughness (0.2-0.6, finish dependent)
│   ├── Clearcoat (0.8-1.0)
│   ├── Clearcoat Roughness (0.0-0.3)
│   ├── Flake Layer (metallic paints)
│   │   ├── Flake density
│   │   ├── Flake size
│   │   └── Flake color variance
│   └── Color-shift Layer (chameleon paints)
│       ├── Base color
│       ├── Shift color
│       └── Shift angle
│
├── CHROME/METAL TRIM
│   ├── Metallic: 1.0
│   ├── Roughness: 0.0-0.2
│   ├── Environment reflection: High priority
│   └── Note: Use reflection captures
│
├── GLASS
│   ├── Transmission: 0.9-1.0
│   ├── Roughness: 0.0
│   ├── Tint color (from player selection)
│   ├── Tint opacity (5%-100%)
│   └── Refraction: Subtle
│
├── RUBBER/TIRES
│   ├── Base Color: Near black (#1a1a1a)
│   ├── Roughness: 0.7-0.9
│   ├── Normal: Tread pattern
│   ├── Wear mask (for condition display)
│   └── Procedural dirt/dust
│
├── PLASTIC (Interior/Exterior)
│   ├── Roughness: 0.4-0.8
│   ├── Metallic: 0.0
│   └── Subtle texture detail
│
├── CARBON FIBER
│   ├── Normal: Weave pattern
│   ├── Roughness: 0.3-0.5
│   ├── Metallic: 0.0
│   ├── Clearcoat: 1.0
│   └── Anisotropic: Subtle
│
└── FABRIC (Seats/Interior)
    ├── Roughness: 0.7-0.95
    ├── Normal: Fabric weave
    ├── Subsurface: Minimal
    └── Fuzz: Optional for suede
```

### 5.4 Damage System Visuals

```
DAMAGE VISUALIZATION
├── LEVEL 1: MINOR (Cosmetic)
│   ├── Scratches (normal map overlay)
│   ├── Small dents (vertex displacement)
│   ├── Scuff marks (decals)
│   └── Dirt accumulation
│
├── LEVEL 2: MODERATE
│   ├── Larger dents (mesh deformation)
│   ├── Paint chips (mask reveals primer)
│   ├── Cracked lights (emissive change)
│   ├── Bent body panels
│   └── Broken trim pieces
│
├── LEVEL 3: SEVERE
│   ├── Major deformation (morph targets)
│   ├── Exposed metal (rust potential)
│   ├── Broken glass (particle + mesh)
│   ├── Hanging parts (physics)
│   └── Smoke/steam from engine
│
└── REPAIR VISUAL
    ├── Bondo patches (texture)
    ├── Mismatched paint (cheap repair)
    ├── Visible welds (texture + normal)
    └── Full restoration (returns to clean)

DAMAGE MAPPING
├── Collision zones defined per vehicle
├── Impact force determines damage level
├── Damage accumulates (persistent)
├── Repair options at various quality/cost
└── History affects vehicle value
```

### 5.5 Customization Visuals

```
BODY KIT VISUALIZATION
├── Each body part is separate mesh
├── Clean swap system (no clipping)
├── Consistent mounting points
├── LODs for all variants
└── Material instance per part

WHEEL VISUALIZATION
├── Separate rim and tire meshes
├── Multiple finish options per rim
│   ├── Chrome
│   ├── Gunmetal
│   ├── Black
│   ├── White
│   ├── Bronze
│   ├── Gold
│   └── Custom color match
├── Tire sidewall text (decal)
├── Brake caliper visible through spokes
└── Rotor glow under hard braking

LIGHTING CUSTOMIZATION
├── Headlight types
│   ├── Halogen (warm, yellow-white)
│   ├── HID (bright, blue-white)
│   ├── LED (pure white)
│   └── Custom colors (aftermarket)
├── Taillight variants
├── Underglow system
│   ├── Color selection
│   ├── Animation patterns
│   ├── Ground projection
│   └── Chassis illumination
└── Interior lighting

ENGINE BAY
├── Viewable in garage mode
├── Parts visually change with upgrades
│   ├── Intake manifold swaps
│   ├── Turbo/supercharger visible
│   ├── Valve cover changes
│   ├── Strut bar additions
│   └── Wiring/hose upgrades
├── Cleanliness level (maintained vs. neglected)
└── Performance parts have visual flair
```

---

## 6. ENVIRONMENT ART DIRECTION

### 6.1 Environment Philosophy

The world of MIDNIGHT GRIND is a stylized urban landscape that serves the racing experience. Every environment should feel like a stage set for street racing—dramatic, atmospheric, and navigable at high speed.

**Key Principles:**
- **Readability at Speed** - Clear racing lines, visible obstacles
- **Atmospheric Density** - Lots of visual interest without clutter
- **Verticality** - Buildings create canyons of light
- **Lived-In Feel** - Signs of life without requiring NPCs everywhere

### 6.2 District Visual Identities

```
DOWNTOWN
├── Architecture: High-rise modern, 80s-90s aesthetic
├── Lighting: Dense neon, billboard screens, street lights
├── Palette: Electric blue, hot pink, cool white
├── Signage: Japanese + English mix, brands everywhere
├── Ground: Reflective wet asphalt (always slightly wet feel)
├── Density: High (buildings close, narrow sight lines)
├── Landmarks: Central tower, main plaza, the strip
└── Mood: Electric, alive, the heart of the scene

INDUSTRIAL ZONE
├── Architecture: Warehouses, factories, container yards
├── Lighting: Sodium orange, sparse, pools of light
├── Palette: Orange, rust, concrete gray, dark shadows
├── Signage: Functional (loading zones, warnings)
├── Ground: Cracked concrete, painted lines, oil stains
├── Density: Low buildings, wide open spaces
├── Landmarks: The strip (drag), container maze
└── Mood: Gritty, dangerous, anything-goes

PORT DISTRICT
├── Architecture: Docks, cranes, waterfront buildings
├── Lighting: Fog-diffused, ship lights, minimal street
├── Palette: Navy blue, rust, weathered gray
├── Signage: Maritime (shipping companies, pier numbers)
├── Ground: Wet concrete, metal grating, wooden piers
├── Density: Medium, open to water on one side
├── Landmarks: Pier 7, dry dock, import lot
└── Mood: Isolated, mysterious, import culture

HIGHWAY NETWORK
├── Architecture: Elevated concrete, tunnel sections
├── Lighting: Regular street lights, tunnel fluorescent
├── Palette: Gray concrete, white/yellow lines, city glow
├── Signage: Highway signs (exits, distances)
├── Ground: Smooth asphalt, lane markings, rumble strips
├── Density: Open sky above, city visible below/beside
├── Landmarks: Bay tunnel, mountain connector, downtown loop
└── Mood: Speed, flow, urban vein system

THE HILLS (CANYON)
├── Architecture: Minimal (guardrails, tunnels)
├── Lighting: Moonlight, occasional street light, headlights
├── Palette: Deep green, rock gray, night blue
├── Signage: Warning signs (curves, falling rocks)
├── Ground: Worn mountain road, patches, cracks
├── Density: Trees and rocks create walls
├── Landmarks: Devil's corner, summit overlook, hot springs
└── Mood: Technical, dangerous, Initial D territory

SUBURBS
├── Architecture: Residential, strip malls, parking lots
├── Lighting: Street lights, store fronts, house windows
├── Palette: Warm residential vs cool commercial
├── Signage: Store signs, street names, house numbers
├── Ground: Residential streets, parking lot concrete
├── Density: Low buildings, wide streets
├── Landmarks: Dead mall, high school lot
└── Mood: Quiet streets, hidden danger
```

### 6.3 Modular Building System

```
BUILDING CONSTRUCTION KIT
├── BASE MODULES (Per district style)
│   ├── Ground floor (storefronts, entrances)
│   ├── Mid floors (office, residential, mixed)
│   ├── Top floor (rooftops, mechanical, penthouses)
│   ├── Corner pieces
│   └── Cap pieces
│
├── FACADE ELEMENTS
│   ├── Windows (lit/unlit variations)
│   ├── Air conditioners
│   ├── Fire escapes
│   ├── Signage mounting points
│   ├── Awnings
│   ├── Balconies
│   └── Satellite dishes
│
├── SIGNAGE SYSTEM
│   ├── Neon signs (animated, flickering options)
│   ├── LED displays
│   ├── Painted signs
│   ├── Banner mounts
│   ├── Rooftop billboards
│   └── Street-level sandwich boards
│
└── GROUND ELEMENTS
    ├── Sidewalk variations
    ├── Road surfaces
    ├── Parking lots
    ├── Curbs and gutters
    ├── Storm drains
    └── Street furniture
```

### 6.4 Road and Track Design

```
ROAD STANDARDS
├── LANE WIDTH
│   ├── Standard: 3.5m
│   ├── Highway: 3.7m
│   ├── Narrow (canyon): 3.0m
│   └── Alley: 2.5m
│
├── SURFACE TYPES
│   ├── Fresh asphalt (smooth, dark)
│   ├── Worn asphalt (lighter, cracked)
│   ├── Concrete (joints visible)
│   ├── Patched (color variation)
│   └── Cobblestone (historic areas)
│
├── MARKINGS
│   ├── Lane dividers (white/yellow)
│   ├── Edge lines
│   ├── Crosswalks
│   ├── Turn arrows
│   ├── Stop lines
│   └── Speed limit markings
│
├── RACING LINE VISIBILITY
│   ├── Apex curbs (red/white)
│   ├── Rumble strips
│   ├── Tire marks (persistent)
│   └── Subtle lighting guidance
│
└── BARRIERS
    ├── Jersey barriers
    ├── Guardrails
    ├── Tire walls
    ├── Catch fencing
    └── Soft barriers (destructible)
```

### 6.5 Environmental Storytelling

```
WORLD DETAILS
├── EVIDENCE OF CAR CULTURE
│   ├── Tire marks on roads
│   ├── Abandoned car parts
│   ├── Street racing graffiti
│   ├── "No street racing" signs
│   ├── Police checkpoint remnants
│   └── Energy drink cans/bottles
│
├── LIVING WORLD ELEMENTS
│   ├── Lit windows (animated)
│   ├── TV flicker in apartments
│   ├── Distant traffic sounds
│   ├── Steam from vents
│   ├── Blowing debris
│   └── Flickering signs
│
├── TIME OF DAY TELLS
│   ├── Closing stores (lights off)
│   ├── 24-hour businesses (always lit)
│   ├── Shift change at factories
│   └── Dawn delivery trucks
│
└── WEATHER EFFECTS
    ├── Wet surfaces everywhere (baseline)
    ├── Rain puddles
    ├── Fog density
    ├── Steam from grates
    └── Reflected city lights
```

---

## 7. CHARACTER & NPC DESIGN

### 7.1 Character Philosophy

While MIDNIGHT GRIND focuses on cars, human characters provide context, rivals, and social connection. Our character approach is stylized and economical—memorable silhouettes and strong personality archetypes.

### 7.2 Character Design Principles

```
DESIGN APPROACH
├── SILHOUETTE FIRST
│   ├── Recognizable at distance
│   ├── Distinctive posture/stance
│   ├── Signature accessories
│   └── Crew affiliation visible
│
├── STYLE AUTHENTICITY
│   ├── Era-appropriate fashion (late 90s-early 2000s)
│   ├── Car culture fashion (racing jackets, JDM brands)
│   ├── Streetwear influence
│   └── Regional variations
│
├── DIVERSITY
│   ├── Multiple ethnicities
│   ├── Gender diversity
│   ├── Age range (18-45 primarily)
│   └── Body type variety
│
└── EFFICIENCY
    ├── Limited poly budget (not focus)
    ├── Simple rigs
    ├── Reusable animations
    └── Modular clothing system
```

### 7.3 Rival Character Archetypes

```
RIVAL ARCHETYPES
├── THE VETERAN
│   ├── Appearance: Older, weathered, confident
│   ├── Style: Classic racing jacket, jeans
│   ├── Car preference: Muscle, classics
│   └── Personality: Respectful of skill, old school rules

├── THE RICH KID
│   ├── Appearance: Designer clothes, perfect hair
│   ├── Style: Expensive streetwear, subtle brands
│   ├── Car preference: Euros, high-end JDM
│   └── Personality: Arrogant but secretly insecure

├── THE TECHNICIAN
│   ├── Appearance: Mechanic's build, grease stains
│   ├── Style: Work clothes, tool belt
│   ├── Car preference: Sleepers, built engines
│   └── Personality: Quiet, lets results speak

├── THE SHOWOFF
│   ├── Appearance: Flashy, attention-seeking
│   ├── Style: Bold colors, bling, wild hair
│   ├── Car preference: Show cars, stance builds
│   └── Personality: Loud, loves an audience

├── THE IMPORT QUEEN/KING
│   ├── Appearance: JDM style, authentic gear
│   ├── Style: Japanese streetwear, brand loyal
│   ├── Car preference: JDM only, authentic parts
│   └── Personality: Purist, knowledgeable

├── THE DRIFT SPECIALIST
│   ├── Appearance: Helmet hair, driving gloves
│   ├── Style: Race suit or casual technical wear
│   ├── Car preference: Drift builds (240, RX-7)
│   └── Personality: Zen focus, artistic approach

├── THE DRAG RACER
│   ├── Appearance: Strong build, confident stance
│   ├── Style: Crew shirt, baseball cap
│   ├── Car preference: Drag builds, big power
│   └── Personality: Calculating, statistical

└── THE LEGEND
    ├── Appearance: Enigmatic, rarely seen
    ├── Style: Minimalist, forgettable
    ├── Car preference: The unbeatable build
    └── Personality: Mythic status, final boss
```

### 7.4 NPC Types

```
NPC CATEGORIES
├── AMBIENT PEDESTRIANS
│   ├── Budget: 3,000-5,000 tris
│   ├── Animation: Walk cycles, idle
│   ├── Variation: 20+ base models
│   └── Behavior: Pathfinding, react to cars

├── MEET SPOT CROWDS
│   ├── Budget: 5,000-8,000 tris
│   ├── Animation: Social idles, phone use, point at cars
│   ├── Variation: 30+ models (car culture specific)
│   └── Behavior: Gather around nice cars

├── SHOP VENDORS
│   ├── Budget: 8,000-12,000 tris
│   ├── Animation: Work animations, greet
│   ├── Variation: 5-10 unique characters
│   └── Behavior: Stationary, interactive

├── POLICE
│   ├── Budget: 8,000-12,000 tris
│   ├── Animation: Patrol, pursuit, arrest
│   ├── Variation: 5-8 officer types
│   └── Behavior: AI controlled, chase states

└── RIVALS (Named characters)
    ├── Budget: 15,000-25,000 tris
    ├── Animation: Full set (cutscenes, idle, race)
    ├── Variation: Unique per rival
    └── Behavior: Story scripted + AI racing
```

---

## 8. LIGHTING DESIGN

### 8.1 Lighting Philosophy

Light creates the world of MIDNIGHT GRIND. In the absence of daylight, every light source is intentional—placed to guide, reveal, or obscure. Our lighting should feel cinematic: dramatic, moody, and always in service of the nocturnal fantasy.

### 8.2 Global Lighting Setup

```
TIME OF DAY LIGHTING
├── DUSK (7PM-9PM)
│   ├── Sun: 5° above horizon, warm orange
│   ├── Sky: Gradient purple to orange
│   ├── Ambient: Warm fill, soft shadows
│   ├── City lights: Beginning to activate
│   └── Mood: Transition, anticipation
│
├── NIGHT (9PM-4AM) [PRIMARY]
│   ├── Moon: Cool blue-white, subtle
│   ├── Sky: Deep blue-purple, stars optional
│   ├── Ambient: Very dark, from bounce only
│   ├── City lights: Full activation
│   └── Mood: Electric, alive, dangerous
│
├── LATE NIGHT (12AM-3AM) [PEAK]
│   ├── Moon: Low angle, long shadows
│   ├── Sky: Darkest, minimal gradient
│   ├── Ambient: Minimum, pools of light only
│   ├── City lights: Some dimmed (closed stores)
│   └── Mood: Intensity, serious racing time
│
└── DAWN (4AM-6AM)
    ├── Sun: Below horizon, sky lightening
    ├── Sky: Blue-gray to pink at horizon
    ├── Ambient: Cool fill returning
    ├── City lights: Beginning to deactivate
    └── Mood: End of night, reflection
```

### 8.3 Light Source Types

```
ARTIFICIAL LIGHT SOURCES
├── STREET LIGHTS
│   ├── Sodium: Orange (5,200K), most common
│   ├── Mercury vapor: Blue-green tint
│   ├── LED: Pure white (6,500K), newer areas
│   ├── Spacing: 30-50m on major roads
│   ├── Height: 8-12m
│   └── Cone angle: 90-120°
│
├── NEON SIGNS
│   ├── Colors: Full spectrum (per sign)
│   ├── Emission: High intensity, no shadow
│   ├── Flicker: Random variation (subtle)
│   ├── Glow: Screen space + volumetric
│   └── Reflection: High priority
│
├── BUILDING WINDOWS
│   ├── Warm residential: 2,700-3,000K
│   ├── Cool office: 4,000-5,000K
│   ├── TV flicker: Animated emissive
│   ├── Variation: Random on/off per window
│   └── LOD: Emissive only at distance
│
├── VEHICLE LIGHTS
│   ├── Headlights: Player-controlled (on/off/high)
│   │   ├── Halogen: Warm white, medium intensity
│   │   ├── HID: Blue-white, high intensity
│   │   └── LED: Pure white, sharp cutoff
│   ├── Taillights: Red emissive, brake brightens
│   ├── Underglow: Player color, ground projection
│   └── Interior: Subtle dash illumination
│
├── COMMERCIAL LIGHTING
│   ├── Store fronts: Warm white, inviting
│   ├── Gas stations: Bright white flood
│   ├── Billboards: Backlit or LED
│   └── Parking lots: High-mounted floods
│
└── SPECIAL SOURCES
    ├── Police lights: Red/blue strobe
    ├── Emergency vehicles: Amber warning
    ├── Construction: Amber + white spots
    └── Tunnels: Fluorescent banks
```

### 8.4 Lighting Techniques

```
TECHNICAL IMPLEMENTATION
├── DYNAMIC LIGHTS
│   ├── Player vehicle headlights: Always dynamic
│   ├── Police lights: Dynamic during pursuit
│   ├── Neon signs (key locations): Dynamic
│   └── Budget: 8-12 dynamic lights visible
│
├── STATIONARY LIGHTS
│   ├── Street lights: Stationary, shadows optional
│   ├── Building lights: Stationary, no shadows
│   ├── Most neon: Stationary
│   └── Managed via distance culling
│
├── BAKED LIGHTING
│   ├── Ambient occlusion
│   ├── Indirect bounce (lightmass)
│   ├── Building interior fill
│   └── Ground contact shadows
│
├── VOLUMETRIC LIGHTING
│   ├── Fog interaction with lights
│   ├── Headlight cones in rain/fog
│   ├── God rays through geometry
│   └── Performance: Optional quality setting
│
├── REFLECTION SYSTEM
│   ├── Screen-space reflections: Primary
│   ├── Planar reflections: Puddles (limited)
│   ├── Cube captures: Fallback, strategic placement
│   └── Priority: Wet roads always reflective
│
└── SHADOW SETUP
    ├── Cascaded shadows: 3 cascades
    ├── Shadow distance: 100m primary, 300m far
    ├── Resolution: 2048 primary, 1024 secondary
    └── Soft shadows: Percentage closer filtering
```

### 8.5 District Lighting Guides

```
DOWNTOWN LIGHTING
├── Density: Very high
├── Primary sources: Neon signs, billboards
├── Secondary: Street lights, store fronts
├── Character: Electric, overwhelming, colorful
├── Key colors: Cyan, pink, white, yellow
├── Shadow quality: High (dense geometry)
└── Special: Animated signs, scrolling text

INDUSTRIAL LIGHTING
├── Density: Low (isolated pools)
├── Primary sources: High-mounted floods
├── Secondary: Loading dock lights, vehicles
├── Character: Harsh, utilitarian, stark contrast
├── Key colors: Sodium orange, white floods
├── Shadow quality: Sharp, dramatic
└── Special: Swinging work lights

CANYON LIGHTING
├── Density: Very low
├── Primary sources: Moonlight
├── Secondary: Occasional street light
├── Character: Natural, mysterious, headlight-dependent
├── Key colors: Blue moonlight, warm headlights
├── Shadow quality: Soft from moon
└── Special: Fog interaction, tree shadows
```

---

## 9. VISUAL EFFECTS (VFX)

### 9.1 VFX Philosophy

Visual effects in MIDNIGHT GRIND should enhance the feeling of speed, power, and intensity. Effects are stylized rather than realistic—they should look "cool" and communicate gameplay information clearly.

### 9.2 Vehicle VFX

```
EXHAUST EFFECTS
├── IDLE
│   ├── Particles: Light wisps
│   ├── Rate: Low (5-10/sec)
│   ├── Color: Transparent gray
│   ├── Size: Small
│   └── Lifetime: 1-2 seconds
│
├── ACCELERATION
│   ├── Particles: Dense burst
│   ├── Rate: High (50-100/sec)
│   ├── Color: Gray to black (depending on tune)
│   ├── Size: Medium, expanding
│   ├── Lifetime: 0.5-1 second
│   └── Note: Flame sprites on high-power/anti-lag
│
├── BACKFIRE
│   ├── Particles: Flash + smoke burst
│   ├── Rate: Burst (50 particles)
│   ├── Color: Orange flash, dark smoke
│   ├── Light: Brief point light
│   └── Trigger: Decel at high RPM, shift
│
├── TURBO FLUTTER
│   ├── Particles: Quick puffs
│   ├── Rate: Matched to flutter frequency
│   ├── Color: Translucent white
│   └── Sound: Synchronized
│
└── NITROUS FLAME
    ├── Particles: Continuous flame sprites
    ├── Rate: Very high while active
    ├── Color: Blue-white core, orange edges
    ├── Light: Flickering blue point light
    ├── Trail: Lingers briefly behind
    └── Special: Lens flare optional

TIRE EFFECTS
├── BURNOUT SMOKE
│   ├── Particles: Dense white cloud
│   ├── Rate: Very high (200+/sec)
│   ├── Color: White to gray
│   ├── Size: Large, billowing
│   ├── Lifetime: 3-5 seconds
│   ├── Physics: Rises and disperses
│   └── Rubber debris: Small black particles
│
├── DRIFT SMOKE
│   ├── Particles: Trail from contact patches
│   ├── Rate: Based on slip angle
│   ├── Color: White
│   ├── Size: Medium
│   ├── Lifetime: 1-3 seconds
│   └── Trail: Follows wheel path
│
├── LAUNCH SMOKE
│   ├── Particles: Burst from rear
│   ├── Rate: High initial, taper
│   ├── Direction: Backward spray
│   └── Duration: Until traction gained
│
├── SKID MARKS
│   ├── Type: Decal projection
│   ├── Color: Black rubber
│   ├── Width: Based on tire width
│   ├── Persistence: Fade over 30 seconds
│   └── Buildup: Darker with repeated passes
│
└── WET SPRAY
    ├── Particles: Water mist
    ├── Rate: Based on speed
    ├── Color: Translucent white
    ├── Direction: Rooster tail behind
    └── Trigger: Wet surface only

COLLISION EFFECTS
├── SPARKS
│   ├── Particles: Metal spark sprites
│   ├── Rate: Burst based on impact
│   ├── Color: Orange-yellow
│   ├── Physics: Bounce, gravity
│   ├── Light: Brief flicker
│   └── Sound: Synchronized
│
├── DEBRIS
│   ├── Type: Mesh particles
│   ├── Variations: Glass, metal, plastic
│   ├── Physics: Rigid body
│   ├── Lifetime: 2-5 seconds
│   └── Despawn: Fade out
│
└── DUST/DIRT
    ├── Particles: Dust cloud
    ├── Trigger: Off-road contact
    ├── Color: Brown, surface-dependent
    └── Size: Based on speed
```

### 9.3 Environmental VFX

```
WEATHER EFFECTS
├── RAIN
│   ├── Particles: Falling drops (GPU)
│   ├── Rate: Intensity based
│   ├── Speed: Affected by wind
│   ├── Splash: On surface contact
│   ├── Streak: Motion blur at speed
│   └── Sound: Layered rain audio
│
├── FOG
│   ├── Type: Volumetric (exponential height)
│   ├── Density: Variable
│   ├── Color: Blue-gray
│   ├── Light interaction: Scattering
│   └── Performance: LOD based
│
├── WIND
│   ├── Debris: Paper, leaves
│   ├── Steam: Vent redirection
│   ├── Flags: Cloth simulation
│   └── Sound: Wind audio
│
└── HEAT DISTORTION
    ├── Type: Screen-space distortion
    ├── Source: Engine bays, exhausts
    ├── Intensity: Temperature based
    └── Quality: Optional setting

AMBIENT EFFECTS
├── STEAM VENTS
│   ├── Particles: Rising steam
│   ├── Color: White, translucent
│   ├── Light: Subtle from below
│   └── Variation: Burst or continuous
│
├── FLICKERING SIGNS
│   ├── Type: Material animation
│   ├── Pattern: Random flicker
│   ├── Sound: Electric buzz
│   └── Variation: Multiple patterns
│
├── TRASH/DEBRIS
│   ├── Particles: Paper, cans, leaves
│   ├── Physics: Wind-blown
│   ├── Spawn: Street level
│   └── Lifetime: Loop in area
│
└── WATER FEATURES
    ├── Puddle ripples
    ├── Drain water flow
    ├── Fountain spray
    └── Harbor waves
```

### 9.4 UI/Feedback VFX

```
SCREEN EFFECTS
├── SPEED LINES
│   ├── Type: Screen-space overlay
│   ├── Trigger: High speed
│   ├── Intensity: Speed-based
│   ├── Color: White, transparent
│   └── Style: Radial from center
│
├── NITROUS ACTIVATION
│   ├── Screen shake: Subtle
│   ├── Color grading: Blue push
│   ├── Blur: Radial zoom
│   ├── Duration: While active
│   └── Sound: Synchronized whoosh
│
├── DAMAGE FEEDBACK
│   ├── Screen shake: Impact-based
│   ├── Flash: Red edge vignette
│   ├── Blur: Momentary
│   └── Duration: Brief
│
├── NEAR MISS
│   ├── Bullet time: Brief slow-mo
│   ├── Streak: Object passes
│   ├── Sound: Whoosh
│   └── Reward: REP bonus display
│
└── POSITION CHANGE
    ├── Text flash: "+1" or "-1"
    ├── Color: Green (gain) / Red (lose)
    ├── Animation: Slide and fade
    └── Sound: Position audio cue
```

---

## 10. UI/UX VISUAL DESIGN

### 10.1 UI Philosophy

The user interface of MIDNIGHT GRIND is an integral part of the aesthetic experience. Our UI draws from Y2K design, early 2000s racing games, and modern UX principles—resulting in something that feels nostalgic but plays smoothly.

### 10.2 UI Visual Language

```
UI DESIGN PRINCIPLES
├── AGGRESSIVE GEOMETRY
│   ├── Angled edges (15-45°)
│   ├── Speed lines as decoration
│   ├── Sharp corners, no rounded edges
│   └── Asymmetric layouts
│
├── LAYERED DEPTH
│   ├── Overlapping panels
│   ├── Drop shadows (hard edges)
│   ├── Transparency layers
│   └── Parallax movement
│
├── NEON INTEGRATION
│   ├── Glowing borders
│   ├── Text glow on focus
│   ├── Animated pulse
│   └── Color consistency with palette
│
├── MOTION
│   ├── Slides and wipes (not fades)
│   ├── Stagger animations
│   ├── Bounce/overshoot easing
│   └── Speed: Fast but readable
│
└── READABILITY
    ├── High contrast text
    ├── Clear hierarchy
    ├── Consistent interaction patterns
    └── Accessibility options
```

### 10.3 UI Component Library

```
BUTTON STYLES
├── PRIMARY BUTTON
│   ├── Background: Gradient (cyan to darker cyan)
│   ├── Border: 2px, electric cyan, angled corners
│   ├── Text: White, bold, uppercase
│   ├── Hover: Glow effect, slight scale
│   ├── Press: Darken, inset effect
│   └── Disabled: Gray, no glow
│
├── SECONDARY BUTTON
│   ├── Background: Transparent
│   ├── Border: 1px, steel gray
│   ├── Text: Light gray
│   ├── Hover: Border turns cyan
│   ├── Press: Brief flash
│   └── Disabled: Darker gray
│
├── DANGER BUTTON
│   ├── Background: Gradient (pink to darker)
│   ├── Border: 2px, neon pink
│   ├── Text: White
│   ├── Hover: Glow, pulse
│   └── Usage: Destructive actions, pink slips
│
└── ICON BUTTON
    ├── Background: Circle or square
    ├── Icon: Centered, single color
    ├── Hover: Background lightens
    └── Size: 40px, 48px, 56px options

PANEL STYLES
├── PRIMARY PANEL
│   ├── Background: Night black (#0a0a14) @ 95%
│   ├── Border: 1px steel gray, angled corners
│   ├── Shadow: 8px offset, 50% opacity
│   └── Usage: Main content areas
│
├── FLOATING PANEL
│   ├── Background: Darker (#050508) @ 90%
│   ├── Border: 2px, cyan (focused) or gray
│   ├── Shadow: Larger offset
│   └── Usage: Modals, popovers
│
├── CARD
│   ├── Background: Slight gradient
│   ├── Border: 1px, subtle
│   ├── Corner treatment: Cut corner (bottom-right)
│   └── Usage: Items, parts, vehicles
│
└── HUD PANEL
    ├── Background: Minimal or none
    ├── Border: Stylized frame
    ├── Transparency: High
    └── Usage: In-race information

INPUT STYLES
├── TEXT INPUT
│   ├── Background: Dark (#0a0a14)
│   ├── Border: Bottom only, 2px
│   ├── Focus: Border turns cyan, caret glow
│   ├── Placeholder: Medium gray, italic
│   └── Error: Border turns pink
│
├── SLIDER
│   ├── Track: Thin line, gray
│   ├── Fill: Cyan gradient
│   ├── Handle: Circle with glow
│   ├── Value label: Above handle
│   └── Snap points: Tick marks optional
│
├── DROPDOWN
│   ├── Closed: Like text input with arrow
│   ├── Open: Panel expands below
│   ├── Options: Hover highlight
│   └── Animation: Slide down
│
└── TOGGLE
    ├── Off: Gray track, left position
    ├── On: Cyan track, right position
    ├── Animation: Slide with bounce
    └── Label: Adjacent text
```

### 10.4 Screen Layouts

```
MAIN MENU
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│   [LOGO: MIDNIGHT GRIND]                                     │
│   ═══════════════════════════════════════                    │
│                                                              │
│        ╔══════════════════════════════╗                      │
│        ║  > CONTINUE                  ║                      │
│        ║    NEW GAME                  ║                      │
│        ║    GARAGE                    ║                      │
│        ║    MULTIPLAYER               ║                      │
│        ║    OPTIONS                   ║                      │
│        ║    EXIT                      ║                      │
│        ╚══════════════════════════════╝                      │
│                                                              │
│   [Car turntable in background]                              │
│                                                              │
│   ─────────────────────────────────────────────────────      │
│   CASH: $125,450  │  REP: 24,500 (RESPECTED)  │  v1.0.0     │
└──────────────────────────────────────────────────────────────┘

GARAGE VIEW
┌──────────────────────────────────────────────────────────────┐
│  ◄ BACK                         GARAGE                  ▸   │
├────────────────────────────────────────────────────┬─────────┤
│                                                    │ MY CARS │
│                                                    │─────────│
│         [3D CAR VIEW - ROTATABLE]                  │ ▸ Car 1 │
│                                                    │   Car 2 │
│                                                    │   Car 3 │
│                                                    │   Car 4 │
│                                                    │   +Add  │
│                                                    │         │
├────────────────────────────────────────────────────┴─────────┤
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │ ENGINE   │ │DRIVETRAIN│ │SUSPENSION│ │  VISUAL  │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘        │
│                                                              │
│  ═══════════════════════════════════════════════════════     │
│  HP: 450 (+25)  │  TQ: 380  │  WT: 3,200  │  PI: 520 (B)    │
└──────────────────────────────────────────────────────────────┘

RACING HUD
┌──────────────────────────────────────────────────────────────┐
│  1ST                                            LAP 2/3      │
│  ───                                            ───────      │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│                    [RACING VIEWPORT]                         │
│                                                              │
│                                                              │
│                                                              │
│                                                              │
│  ┌────────┐                                     ┌─────────┐  │
│  │MINIMAP │                                     │ N2O     │  │
│  │   •    │                                     │ ████░░  │  │
│  └────────┘                                     └─────────┘  │
│                 ╔═══════════════════════╗                    │
│                 ║   ▓▓▓▓▓▓ 156 MPH ▓▓▓  ║                    │
│                 ║      ⚙ 4TH GEAR       ║                    │
│                 ╚═══════════════════════╝                    │
└──────────────────────────────────────────────────────────────┘
```

### 10.5 Animation Guidelines

```
UI ANIMATION PRINCIPLES
├── TIMING
│   ├── Fast: 150ms (button feedback)
│   ├── Medium: 250ms (panel transitions)
│   ├── Slow: 400ms (screen transitions)
│   └── Very slow: 600ms+ (dramatic reveals)
│
├── EASING
│   ├── In: Slow start (for exits)
│   ├── Out: Slow end (for entries)
│   ├── InOut: For continuous motion
│   └── Bounce: For playful elements
│
├── PATTERNS
│   ├── Slide: Panels enter/exit
│   ├── Scale: Focus/selection
│   ├── Fade: Overlays only
│   ├── Stagger: List items
│   └── Pulse: Attention
│
└── TRANSITIONS
    ├── Menu to menu: Slide left/right
    ├── Open panel: Scale up + fade
    ├── Close panel: Scale down + fade
    ├── Focus change: Quick scale pulse
    └── Error shake: Horizontal wiggle
```

---

## 11. TECHNICAL ART GUIDELINES

### 11.1 Shader Standards

```
SHADER COMPLEXITY BUDGET
├── VEHICLE SHADERS
│   ├── Instructions: 200-400 (body paint)
│   ├── Texture samples: 8-12
│   ├── Features: Clearcoat, metallic flake, damage mask
│   └── LOD: Simplified at distance
│
├── ENVIRONMENT SHADERS
│   ├── Instructions: 100-200 (standard)
│   ├── Texture samples: 4-8
│   ├── Features: Wet surface, emission
│   └── Instancing: Required for props
│
├── EFFECT SHADERS
│   ├── Instructions: 50-150
│   ├── Texture samples: 2-4
│   ├── Features: Additive, distortion
│   └── Fillrate: Minimize overdraw
│
└── UI SHADERS
    ├── Instructions: Minimal
    ├── Features: Masks, gradients, glow
    └── Performance: Priority for responsiveness
```

### 11.2 LOD Strategy

```
LOD DISTANCES (Based on object importance)
├── VEHICLES
│   ├── LOD0: 0-15m (full detail)
│   ├── LOD1: 15-50m (reduced)
│   ├── LOD2: 50-100m (simple)
│   ├── LOD3: 100-200m (very simple)
│   └── LOD4: 200m+ (impostor/cull)
│
├── ENVIRONMENT (Large)
│   ├── LOD0: 0-50m
│   ├── LOD1: 50-150m
│   ├── LOD2: 150-300m
│   └── LOD3: 300m+ (billboard)
│
├── PROPS
│   ├── LOD0: 0-20m
│   ├── LOD1: 20-50m
│   └── Cull: 50-100m (importance based)
│
└── CHARACTERS
    ├── LOD0: 0-10m
    ├── LOD1: 10-30m
    ├── LOD2: 30-50m
    └── Cull: 50m+
```

### 11.3 Memory Budgets

```
TEXTURE MEMORY BUDGET (Per Category)
├── Vehicles (loaded): 512MB
│   ├── ~8-10 vehicles at LOD0
│   └── Streaming for garage mode
│
├── Environment (visible): 1GB
│   ├── District textures
│   ├── Shared atlases
│   └── Streaming managed
│
├── UI: 256MB
│   ├── Atlases
│   ├── Fonts
│   └── Icons
│
├── Effects: 128MB
│   ├── Particle textures
│   ├── Decals
│   └── Shared resources
│
└── Audio Visuals: 64MB
    └── Waveforms, UI feedback

MESH MEMORY BUDGET
├── Vehicles: 256MB
├── Environment: 512MB
├── Characters: 64MB
└── Props: 128MB
```

### 11.4 Performance Guidelines

```
DRAW CALL TARGETS
├── Vehicles: 50-100 calls (per vehicle)
├── Environment: 500-1000 calls (visible)
├── UI: 50-100 calls
├── Effects: 100-200 calls
└── Total: <2000 calls per frame

TRIANGLE TARGETS
├── Vehicles: 100k-200k (all visible)
├── Environment: 500k-1M
├── Characters: 50k-100k
├── Effects: Variable
└── Total: <2M triangles per frame

TEXTURE STREAMING
├── Pool size: 2GB
├── Priority: Vehicles > Environment > Props
├── Mip bias: Adjusted by distance
└── Pre-loading: On district transition
```

---

## 12. ASSET PRODUCTION STANDARDS

### 12.1 Naming Conventions

```
ASSET NAMING PATTERN
[Category]_[Name]_[Variant]_[Suffix]

CATEGORIES
├── VH_ : Vehicles
├── PT_ : Parts
├── EN_ : Environment
├── PR_ : Props
├── CH_ : Characters
├── UI_ : User Interface
├── FX_ : Effects
├── MT_ : Materials
├── TX_ : Textures
└── AU_ : Audio

SUFFIXES
├── _D   : Diffuse/Albedo
├── _N   : Normal
├── _ORM : Occlusion/Roughness/Metallic
├── _E   : Emissive
├── _M   : Mask
├── _SK  : Skeletal Mesh
├── _SM  : Static Mesh
├── _MAT : Material Instance
├── _BP  : Blueprint
└── _LODn: Level of Detail

EXAMPLES
├── VH_SakuraGTR_Base_SM
├── VH_SakuraGTR_Hood_Carbon_SM
├── PT_Turbo_Large_SM
├── EN_Downtown_Building_01_SM
├── TX_Asphalt_Wet_D
├── MT_CarPaint_Metallic_MAT
└── FX_TireSmoke_BP
```

### 12.2 Folder Structure

```
/Content
├── /Vehicles
│   ├── /VH_SakuraGTR
│   │   ├── /Mesh
│   │   ├── /Materials
│   │   ├── /Textures
│   │   └── /Blueprints
│   └── /[VehicleName]...
│
├── /Parts
│   ├── /Engine
│   ├── /Drivetrain
│   ├── /Suspension
│   ├── /Brakes
│   ├── /Wheels
│   ├── /Body
│   └── /Interior
│
├── /Environment
│   ├── /Downtown
│   ├── /Industrial
│   ├── /Port
│   ├── /Highway
│   ├── /Canyon
│   ├── /Suburbs
│   └── /Shared
│
├── /Characters
│   ├── /Rivals
│   ├── /NPCs
│   └── /Shared
│
├── /UI
│   ├── /Widgets
│   ├── /Icons
│   ├── /Fonts
│   └── /Animations
│
├── /Effects
│   ├── /Particles
│   ├── /Decals
│   └── /PostProcess
│
├── /Audio
│   ├── /Engines
│   ├── /Environment
│   ├── /UI
│   └── /Music
│
├── /Blueprints
│   ├── /Core
│   ├── /Vehicle
│   ├── /Racing
│   └── /World
│
└── /Data
    ├── /DataTables
    ├── /Curves
    └── /Config
```

### 12.3 Quality Checklist

```
ASSET QUALITY CHECKLIST
├── MODELING
│   ☐ Correct scale (1 UE unit = 1 cm)
│   ☐ Clean topology (no n-gons in deform areas)
│   ☐ Proper UV layouts (no stretching, efficient)
│   ☐ LODs generated and validated
│   ☐ Collision meshes simplified
│   ☐ Pivot points correct
│   ☐ Naming convention followed
│
├── TEXTURING
│   ☐ Resolution appropriate for usage
│   ☐ Power of 2 dimensions
│   ☐ Correct color space (sRGB for color, Linear for data)
│   ☐ Proper compression settings
│   ☐ No visible seams
│   ☐ Tiling where appropriate
│   ☐ Alpha channels optimized
│
├── MATERIALS
│   ☐ Material instances (not unique materials)
│   ☐ Efficient shader complexity
│   ☐ Proper parameter naming
│   ☐ LOD material switching
│   ☐ Tested in multiple lighting
│
├── RIGGING (Characters/Vehicles)
│   ☐ Clean hierarchy
│   ☐ Proper bone naming
│   ☐ Weight painting validated
│   ☐ No mesh tearing at extremes
│
└── INTEGRATION
    ☐ Placed in correct folder
    ☐ Redirectors cleaned up
    ☐ No broken references
    ☐ Tested in engine
    ☐ Reviewed by lead
```

---

## Related Documents

- [Game Design Document](../design/GDD.md)
- [Technical Design Document](../technical/TechnicalDesign.md)
- [Audio Design Document](../audio/AudioDesign.md)

---

**Document Version:** 1.0
**Project:** MIDNIGHT GRIND
**Status:** Development Ready
