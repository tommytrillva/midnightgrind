# MIDNIGHT GRIND
## Vehicle Systems Specification
### Version 2.0

---

## TABLE OF CONTENTS

1. [Vehicle Architecture](#1-vehicle-architecture)
2. [Performance Calculation System](#2-performance-calculation-system)
3. [Parts Database](#3-parts-database)
4. [Visual Customization](#4-visual-customization)
5. [Vehicle Classification](#5-vehicle-classification)

---

## 1. VEHICLE ARCHITECTURE

### 1.1 Base Vehicle Structure

Every vehicle in the game is composed of interconnected systems:

```
VEHICLE
├── CHASSIS
│   ├── Frame Type (Unibody/Body-on-Frame)
│   ├── Weight Distribution
│   ├── Rigidity Rating
│   └── Modification Potential
│
├── ENGINE SYSTEM
│   ├── Engine Block
│   ├── Cylinder Head(s)
│   ├── Rotating Assembly
│   ├── Valvetrain
│   ├── Aspiration System
│   ├── Fuel System
│   ├── Ignition System
│   ├── Exhaust System
│   ├── Cooling System
│   └── Engine Management
│
├── DRIVETRAIN SYSTEM
│   ├── Clutch Assembly
│   ├── Transmission
│   ├── Final Drive
│   ├── Driveshaft(s)
│   └── Axles/CV Shafts
│
├── SUSPENSION SYSTEM
│   ├── Front Suspension
│   └── Rear Suspension
│
├── BRAKE SYSTEM
│   ├── Front Brakes
│   ├── Rear Brakes
│   ├── Brake Lines
│   ├── Master Cylinder
│   ├── Brake Bias
│   └── Parking Brake
│
├── WHEEL & TIRE SYSTEM
│   ├── Wheels
│   └── Tires
│
├── BODY SYSTEM
│   ├── Body Panels
│   ├── Aerodynamics
│   ├── Glass
│   ├── Lighting
│   └── Interior
│
├── ELECTRICAL SYSTEM
│   ├── Battery
│   └── Alternator
│
├── NITROUS SYSTEM (Optional)
│   ├── Bottle
│   ├── System Type
│   ├── Jet Size
│   └── Activation
│
└── SAFETY EQUIPMENT
    ├── Roll Cage
    ├── Harness
    ├── Fire Suppression
    └── Kill Switch
```

### 1.2 Engine System Detail

```
ENGINE SYSTEM
├── Engine Block
│   ├── Configuration (I4, I6, V6, V8, Rotary, etc.)
│   ├── Displacement (cc/ci)
│   ├── Material (Iron/Aluminum)
│   ├── Strength Rating (max HP potential)
│   └── Modification Slots
│
├── Cylinder Head(s)
│   ├── Valve Configuration (SOHC/DOHC)
│   ├── Valves Per Cylinder
│   ├── Port Design
│   └── Flow Rating
│
├── Rotating Assembly
│   ├── Crankshaft
│   ├── Connecting Rods
│   ├── Pistons
│   └── Balance Rating
│
├── Valvetrain
│   ├── Camshaft(s)
│   ├── Valve Springs
│   ├── Retainers
│   └── Timing System
│
├── Aspiration System
│   ├── Intake Manifold
│   ├── Throttle Body
│   ├── Air Filter
│   └── Forced Induction (if equipped)
│       ├── Turbocharger(s)
│       │   ├── Compressor Size
│       │   ├── Turbine Size
│       │   ├── Bearing Type
│       │   └── Boost Threshold
│       ├── Supercharger
│       │   ├── Type (Roots/Twin-Screw/Centrifugal)
│       │   ├── Displacement
│       │   └── Drive Ratio
│       ├── Intercooler
│       │   ├── Type (FMIC/TMIC/Side Mount)
│       │   ├── Core Size
│       │   └── Efficiency Rating
│       └── Boost Control
│           ├── Wastegate
│           └── Boost Controller
│
├── Fuel System
│   ├── Fuel Injectors
│   │   ├── Flow Rate (cc/min)
│   │   ├── Injector Count
│   │   └── Type (Port/Direct)
│   ├── Fuel Pump
│   │   ├── Flow Rate (lph)
│   │   └── Pressure Rating
│   ├── Fuel Rail
│   ├── Fuel Lines
│   └── Fuel Type Compatibility
│
├── Ignition System
│   ├── Spark Plugs
│   ├── Ignition Coils
│   ├── Timing Control
│   └── Spark Energy
│
├── Exhaust System
│   ├── Exhaust Manifold/Headers
│   │   ├── Primary Tube Diameter
│   │   ├── Collector Size
│   │   └── Material
│   ├── Catalytic Converter (or delete)
│   ├── Midpipe
│   │   ├── Diameter
│   │   └── Configuration (X-pipe/H-pipe)
│   ├── Muffler
│   │   ├── Type (Straight-through/Chambered)
│   │   └── Sound Character
│   └── Tips (Cosmetic)
│
├── Cooling System
│   ├── Radiator
│   │   ├── Core Size
│   │   ├── Row Count
│   │   └── Material
│   ├── Cooling Fans
│   ├── Thermostat
│   ├── Water Pump
│   └── Oil Cooler (optional)
│
└── Engine Management
    ├── ECU
    │   ├── Base Map
    │   ├── Tune State
    │   └── Feature Unlocks
    └── Sensors
```

### 1.3 Drivetrain System Detail

```
DRIVETRAIN SYSTEM
├── Clutch Assembly
│   ├── Disc Type
│   ├── Pressure Plate
│   └── Torque Capacity
│
├── Transmission
│   ├── Type (Manual/Sequential/Auto)
│   ├── Gear Count
│   ├── Gear Ratios (per gear)
│   ├── Shift Speed
│   └── Strength Rating
│
├── Final Drive
│   ├── Ring & Pinion Ratio
│   └── Differential Type
│       ├── Open
│       ├── Limited Slip (1-way/1.5-way/2-way)
│       ├── Torsen
│       └── Locking
│
├── Driveshaft(s)
│   ├── Material
│   └── Weight
│
└── Axles/CV Shafts
    ├── Strength Rating
    └── Type
```

### 1.4 Suspension System Detail

```
SUSPENSION SYSTEM
├── Front Suspension
│   ├── Type (MacPherson/Double Wishbone/etc.)
│   ├── Springs
│   │   ├── Rate (lb/in or N/mm)
│   │   └── Preload
│   ├── Dampers
│   │   ├── Compression Rate
│   │   ├── Rebound Rate
│   │   └── Adjustability
│   ├── Anti-Roll Bar
│   │   └── Stiffness
│   ├── Control Arms
│   ├── Bushings
│   │   └── Material (Rubber/Poly/Solid)
│   └── Geometry Settings
│       ├── Camber
│       ├── Caster
│       └── Toe
│
└── Rear Suspension
    └── [Same structure as front]
```

---

## 2. PERFORMANCE CALCULATION SYSTEM

### 2.1 Power Output Calculation

The engine's power output is calculated from all contributing components:

```
BASE POWER
├── Defined per engine configuration
├── Stock power curve (HP vs RPM)
└── Stock torque curve (TQ vs RPM)

ASPIRATION MULTIPLIERS
├── Air Filter: 1.00 - 1.03x
├── Intake Manifold: 1.00 - 1.08x
├── Throttle Body: 1.00 - 1.05x
├── Headers: 1.00 - 1.10x
├── Exhaust: 1.00 - 1.08x
├── Catalytic Converter Delete: 1.02 - 1.05x
└── Combined CAP: ~1.25x naturally aspirated

INTERNAL MULTIPLIERS
├── Camshaft: Reshapes power curve
│   ├── Stock: Balanced
│   ├── Stage 1: +5% peak, higher RPM bias
│   ├── Stage 2: +10% peak, further RPM shift
│   └── Stage 3: +15% peak, narrow power band
├── Cylinder Head: 1.00 - 1.15x
│   └── Also affects rev limit ceiling
├── Rotating Assembly: Enables higher limits
│   ├── Pistons: +500-2000 RPM ceiling
│   ├── Rods: +500-1500 RPM ceiling
│   └── Crank: +500-1000 RPM ceiling
└── Head Gasket: Enables boost levels

FORCED INDUCTION
├── Turbocharger:
│   ├── Boost Pressure x Efficiency = Multiplier
│   ├── Small Turbo: Fast spool, 1.3-1.6x, caps early
│   ├── Medium Turbo: Balanced, 1.5-2.0x
│   ├── Large Turbo: Slow spool, 1.8-2.5x, top end
│   └── Compound: Twin setup, broadest range
├── Supercharger:
│   ├── Linear power delivery
│   ├── Roots: Instant boost, 1.3-1.5x
│   ├── Twin-Screw: Efficient, 1.4-1.7x
│   └── Centrifugal: RPM-dependent, 1.5-2.0x
└── Intercooler Efficiency affects max sustained boost

FUEL SYSTEM LIMITS
├── Injector flow must support fuel demands
├── Fuel pump must supply injectors
├── Insufficient fueling = power cap + engine risk
└── Lean conditions cause damage over time

ECU TUNING
├── Base Map: 90% of potential
├── Stage 1 Tune: 95% of potential
├── Stage 2 Tune: 100% of potential
├── Custom Tune: 100% + optimizations
└── Aggressive Tune: 105% with reliability risk
```

**Final Power Formula:**
```
HP = BasePower × AspirationMods × InternalMods ×
     ForcedInduction × ECUEfficiency × FuelSystemCap ×
     ConditionMultiplier

TQ = BaseTorque × (similar calculation) ×
     PowerBandShape × RPMPosition
```

### 2.2 Weight Calculation

```
BASE CURB WEIGHT
└── Defined per vehicle

ADDITIONS
├── Aftermarket Parts (net weight vs stock)
├── Roll Cage: +30-80 lbs
├── Stereo System: +20-100 lbs
├── Nitrous System: +20-40 lbs
└── Passengers: +150 lbs each

REDUCTIONS
├── Lightweight Battery: -15-25 lbs
├── Carbon Hood: -20-40 lbs
├── Carbon Trunk: -15-30 lbs
├── Lightweight Seats: -30-60 lbs (pair)
├── Interior Delete: -50-150 lbs
├── AC Delete: -30-50 lbs
├── Rear Seat Delete: -30-50 lbs
├── Lexan Windows: -20-40 lbs
├── Fiberglass Panels: Variable
└── Titanium Exhaust: -10-20 lbs

WEIGHT DISTRIBUTION
├── Front/Rear Percentage
├── Affected by part locations
├── Battery relocation shifts balance
└── Engine position is primary factor

GAMEPLAY EFFECT
├── Acceleration = Power / Weight
├── Braking = Weight affects stopping distance
├── Handling = Weight affects responsiveness
└── Tire Wear = Weight increases wear rate
```

### 2.3 Grip & Handling Calculation

```
BASE GRIP
├── Tire compound provides base grip coefficient
│   ├── Economy: 0.70
│   ├── Sport: 0.85
│   ├── Performance: 0.95
│   ├── Semi-Slick: 1.05
│   └── Slick: 1.15 (dry), 0.40 (wet)
│
├── Tire width affects contact patch
│   └── Wider = More grip, more drag
│
├── Tire condition multiplier
│   ├── New: 1.00
│   ├── Good: 0.98
│   ├── Worn: 0.90
│   ├── Critical: 0.75
│   └── Bald: 0.50

SUSPENSION FACTORS
├── Spring Rate affects weight transfer
├── Damping affects transient response
├── Anti-Roll affects body roll/grip transfer
├── Geometry affects contact patch angle
│   ├── Camber: Optimal = Max grip
│   ├── Toe: Affects straight-line vs turning
│   └── Caster: Affects self-centering

AERO FACTORS (Speed Dependent)
├── Downforce increases grip at speed
├── Splitter: Front grip +
├── Wing: Rear grip +
├── Diffuser: Overall grip +
└── Drag penalty for all aero

SURFACE FACTORS
├── Dry Asphalt: 1.00
├── Wet Asphalt: 0.70
├── Damp: 0.85
├── Oil/Debris: 0.50
└── Gravel/Dirt: 0.40
```

**Final Grip Formula:**
```
GripForce = TireGrip × Width × Condition ×
            SuspensionEfficiency × AeroDownforce ×
            SurfaceCondition × WeightOnWheel
```

---

## 3. PARTS DATABASE

### 3.1 Part Tier System

| Tier | Cost | Performance | Durability | Availability |
|------|------|-------------|------------|--------------|
| Stock | - | 1.00 | 100% | Default |
| Street | $ | 1.15 | 85% | Common shops |
| Sport | $$ | 1.30 | 75% | Specialty shops |
| Race | $$$ | 1.50 | 60% | Rare/Events |
| Pro | $$$$ | 1.70 | 50% | Boss drops/Events |
| Legendary | $$$$$ | 1.90 | 40% | Pink Slips only |

**Note:** Lower tier = Longer lifespan. Higher tier = More maintenance required.

### 3.2 Engine Parts

#### Air Filter

| Part | Cost | HP Gain | Durability | Description |
|------|------|---------|------------|-------------|
| Stock Paper | - | - | 90% | OEM replacement |
| High-Flow Panel | $50-150 | +2-5 | 85% | Drop-in performance filter |
| Cold Air Intake | $200-400 | +5-15 | 90% | Relocates filter for cooler air |
| Ram Air Intake | $400-800 | +10-20 | 85% | Pressurized at speed |
| Race Intake | $800-1500 | +15-30 | 70% | Individual throttle bodies |

#### Throttle Body

| Part | Cost | HP Gain |
|------|------|---------|
| Stock | - | - |
| Enlarged (+2-5mm) | $150-300 | +5-10 |
| Ported & Polished | $200-400 | +5-15 |
| Oversized Racing | $400-800 | +10-25 |

#### Intake Manifold

| Part | Cost | HP Gain |
|------|------|---------|
| Stock | - | - |
| Ported Stock | $200-400 | +5-15 |
| Performance Manifold | $500-1000 | +15-30 |
| Race Manifold | $1000-2000 | +25-50 |
| Individual Throttle Bodies | $2000-5000 | +30-60 |

#### Camshaft

| Part | Cost | HP Gain | Effect | Idle |
|------|------|---------|--------|------|
| Stock | - | - | Balanced | Normal |
| Stage 1 | $300-600 | +10-25 | +500 RPM shift | Slightly lumpy |
| Stage 2 | $600-1200 | +20-45 | +1000 RPM shift | Aggressive lope |
| Stage 3 | $1000-2000 | +35-70 | +1500+ RPM shift | Requires tune |
| Race | $1500-3000 | +50-100 | Top-end only | Barely idles |

#### Cylinder Head

| Part | Cost | HP Gain | Effect |
|------|------|---------|--------|
| Stock | - | - | - |
| Ported Stock | $500-1000 | +15-30 | - |
| Performance Head | $1500-3000 | +30-60 | Higher rev ceiling |
| Race Head | $3000-6000 | +50-100 | Significantly higher rev ceiling |
| Full Race CNC | $5000-10000 | +75-150 | Maximum flow potential |

#### Pistons

| Part | Cost | HP Gain | Effect |
|------|------|---------|--------|
| Stock | - | - | - |
| Forged Stock Replacement | $400-800 | +0 | +500 RPM ceiling, boost safe |
| Forged Performance | $800-1500 | +5-10 | +1000 RPM ceiling |
| Forged Race | $1500-3000 | +10-20 | +1500 RPM ceiling |
| Billet Race | $3000-6000 | +15-30 | +2000 RPM ceiling |

#### Connecting Rods

| Part | Cost | HP Gain | Effect |
|------|------|---------|--------|
| Stock | - | - | - |
| Forged H-Beam | $500-1000 | +0 | +500 RPM ceiling |
| Forged I-Beam Race | $1000-2000 | +5 | +1000 RPM ceiling |
| Billet I-Beam | $2000-4000 | +10 | +1500 RPM ceiling |

#### Crankshaft

| Part | Cost | Effect |
|------|------|--------|
| Stock | - | - |
| Rebalanced Stock | $300-600 | Smoother, +250 RPM |
| Forged Replacement | $800-1500 | +500 RPM ceiling |
| Forged Stroker | $1500-3000 | +Displacement, +torque |
| Billet Race | $3000-6000 | +1000 RPM ceiling |

#### Flywheel

| Part | Cost | Weight Reduction | Effect |
|------|------|------------------|--------|
| Stock | - | - | - |
| Lightweight Steel | $200-400 | -5-10 lbs | Faster rev response |
| Aluminum | $400-800 | -10-15 lbs | Very fast rev response |
| Chrome-Moly Race | $600-1200 | -12-18 lbs | Maximum rev response |

#### Engine Block

| Part | Cost | HP Capacity | Requirement |
|------|------|-------------|-------------|
| Stock | - | Stock | - |
| Sleeved Stock | $1500-3000 | +300 HP | Big power builds |
| Performance Block | $3000-6000 | +500 HP | Serious race builds |
| Race Block (Billet) | $8000-15000 | +1000 HP | Ultimate builds only |

### 3.3 Forced Induction

#### Turbocharger Sizing

| Size | Compressor | Boost Threshold | Max Boost | HP Support | Lag | Cost | Best For |
|------|------------|-----------------|-----------|------------|-----|------|----------|
| Small | 50-58mm | 2500-3000 RPM | 12-18 PSI | +50-150 | 0.3-0.5s | $800-1500 | Responsive street |
| Medium | 58-66mm | 3500-4000 RPM | 18-28 PSI | +150-350 | 0.5-0.8s | $1500-3000 | Performance |
| Large | 66-76mm | 4500-5500 RPM | 25-40 PSI | +350-600 | 0.8-1.5s | $3000-5000 | Drag/highway |
| Huge | 76mm+ | 5500+ RPM | 35-60+ PSI | +600-1000+ | 1.5s+ | $5000-10000 | Maximum power |

#### Twin Turbo Configurations

| Config | Description | Cost |
|--------|-------------|------|
| Parallel Twins | Two identical turbos, faster spool | 2x single price |
| Sequential Twins | Small primary + large secondary | 2.5x single price |
| Compound Twins | Small feeds large, eliminates lag | 3x single price |

#### Supporting Components

**Wastegate:**
| Type | Cost | Flow | Control |
|------|------|------|---------|
| Internal | Included | Limited | Limited |
| External Small | $200-400 | 400 HP | Good |
| External Large | $400-700 | 700 HP | Excellent |
| External Race (Dual) | $600-1200 | 1000+ HP | Maximum |

**Intercooler:**
| Type | Cost | Efficiency | Notes |
|------|------|------------|-------|
| Stock | - | 60-70% | - |
| Upgraded Stock Location | $400-800 | 75-85% | - |
| FMIC | $800-1500 | 85-95% | Requires bumper mod |
| Large FMIC | $1200-2500 | 90-98% | Best for high boost |
| Race FMIC | $2000-4000 | 95-99% | Maximum cooling |

#### Supercharger Types

| Type | Characteristics | HP Gain | Cost |
|------|-----------------|---------|------|
| Roots Small | Instant boost, classic whine | +75-150 | $2000-3500 |
| Roots Medium | - | +150-250 | $3500-5500 |
| Roots Large | - | +250-400 | $5500-8000 |
| Twin-Screw Medium | More efficient than roots | +150-300 | $4000-6000 |
| Twin-Screw Large | - | +300-500 | $6000-9000 |
| Centrifugal Small | Boost builds with RPM | +100-200 | $2500-4000 |
| Centrifugal Medium | - | +200-400 | $4000-6000 |
| Centrifugal Large | - | +400-700 | $6000-10000 |

#### Nitrous Oxide Systems

**System Types:**

| Type | Description | HP Options | Cost Range |
|------|-------------|------------|------------|
| Dry | N2O only, ECU compensates | 25-150 HP | $300-1200 |
| Wet | N2O + Fuel together | 50-300 HP | $500-2500 |
| Direct Port | Individual injectors | 150-600 HP | $1500-6000 |

**Bottle Sizes:**
- 5 lb: 3-5 passes
- 10 lb: 6-10 passes (standard)
- 15 lb: 10-15 passes
- 20 lb: 15-20 passes

**Refill Cost:** $3-5 per pound

**Risks:**
- Lean condition = engine damage
- Bottle too cold/hot = problems
- Over-jetting = catastrophic failure
- Improper activation = backfire

### 3.4 Drivetrain Parts

#### Clutch System

| Disc Type | Cost | Torque Capacity | Engagement | Best For |
|-----------|------|-----------------|------------|----------|
| Stock Organic | Stock | Stock rating | Smooth | Daily |
| Performance Organic | $200-400 | +20-30% | Smooth | Mild builds |
| Ceramic/Metallic Puck | $300-600 | +50-75% | Grabby | High power |
| Full Metallic | $500-1000 | +100%+ | Very grabby | Race only |
| Twin/Triple Disc | $1500-3500 | +150-200% | Smooth | Extreme power |

#### Transmission

| Upgrade | Cost | Effect |
|---------|------|--------|
| Stock Rebuild | $500-1000 | Restores condition |
| Strengthened Stock | $1500-3000 | +50% HP capacity |
| Close-Ratio Gearset | $2000-4000 | Tighter spacing, lower top |
| Performance Synchros | $800-1500 | Faster shifts |
| Full Race Trans | $5000-15000 | +200% HP capacity |
| Street Sequential | $8000-15000 | 50-100ms shifts |
| Race Sequential | $15000-30000 | 25-50ms shifts |

#### Gear Ratios (Example 6-Speed)

| Gear | Short (Accel) | Stock | Tall (Top Speed) |
|------|---------------|-------|------------------|
| 1st | 3.50 | 3.20 | 2.90 |
| 2nd | 2.30 | 2.05 | 1.85 |
| 3rd | 1.65 | 1.45 | 1.30 |
| 4th | 1.25 | 1.05 | 0.95 |
| 5th | 0.95 | 0.80 | 0.72 |
| 6th | 0.75 | 0.65 | 0.58 |

#### Limited Slip Differential

| Type | Cost | Lockup | Best For |
|------|------|--------|----------|
| Open | Stock | None | Default |
| 1-Way LSD | $800-1500 | Accel only | FWD, general |
| 1.5-Way LSD | $1000-2000 | Full accel, partial decel | Street/track |
| 2-Way LSD | $1200-2500 | Full accel and decel | Drift, aggressive |
| Torsen | $1500-3000 | Torque-sensing | Street, AWD |
| Spool/Welded | $100-300 | Always 100% | Drift, drag only |

#### Driveshaft Materials

| Material | Cost | Weight Savings |
|----------|------|----------------|
| Stock Steel | - | - |
| Aluminum | $400-800 | -8-15 lbs |
| Carbon Fiber | $800-1500 | -15-25 lbs |
| Chromoly | $500-1000 | Same, stronger |

### 3.5 Suspension Parts

#### Springs

| Type | Cost | Rate Increase | Drop |
|------|------|---------------|------|
| Mild Lowering | $200-400 | +10-20% | 1-1.5" |
| Moderate Lowering | $300-500 | +20-40% | 1.5-2" |
| Aggressive Lowering | $400-700 | +40-60% | 2-3" |
| Race Springs | $500-1000 | Application specific | - |

#### Coilovers

| Level | Cost | Adjustment | Damping |
|-------|------|------------|---------|
| Entry | $600-1200 | Height only | Fixed |
| Performance | $1200-2500 | Height + damping | Single-adjust |
| Premium | $2500-4500 | Height + damping | Two-way adjust |
| Race | $4500-10000 | Full | 3-way adjust |

#### Air Suspension

| Type | Cost | Best For |
|------|------|----------|
| Basic Air | $1500-3000 | Show cars |
| Performance Air | $3000-6000 | Show and go |
| Management System | +$500-1500 | Presets, auto-level |

#### Anti-Roll Bars

| Part | Cost | Effect |
|------|------|--------|
| Front Upgraded | $200-400 | +20-40% stiffer |
| Front Adjustable | $300-600 | Variable |
| Rear Upgraded | $200-400 | +30-50% stiffer |
| Rear Adjustable | $300-600 | Variable |

**Effect:** Stiffer Front = More understeer. Stiffer Rear = More oversteer.

#### Chassis Bracing

| Part | Cost | Effect |
|------|------|--------|
| Front Strut Tower Bar | $100-300 | Sharper turn-in |
| Rear Strut Tower Bar | $100-300 | More planted |
| Front Lower Tie Bar | $100-250 | Reduces subframe flex |
| Rear Lower Tie Bar | $100-250 | Reduces subframe flex |
| Fender Braces | $150-350 | Additional rigidity |

#### Bushings

| Type | Cost | Effect | Trade-off |
|------|------|--------|-----------|
| Polyurethane | $200-600 (kit) | Sharper response | More NVH |
| Spherical Bearings | $400-1200 | Zero deflection | Harsh, wear |

#### Alignment Settings

| Setting | Stock | Street | Track | Drift |
|---------|-------|--------|-------|-------|
| Camber | 0 to -0.5° | -1° to -2° | -2° to -3.5° | -4° to -8° |
| Caster | 4-7° | +1-3° over stock | - | - |
| Front Toe | Varies | - | - | - |
| Rear Toe | Varies | Toe-in for stability | - | - |

### 3.6 Brake Parts

#### Rotors

| Type | Cost (each) | Effect |
|------|-------------|--------|
| Stock Replacement | $50-150 | OEM equivalent |
| Vented Upgrade | $100-250 | Better cooling |
| Drilled | $150-300 | Gas release, looks |
| Slotted | $150-350 | Gas release, pad cleaning |
| Two-Piece Floating | $400-800 | Lighter, better cooling |
| Oversized | $300-600 | More braking power |

#### Calipers

| Type | Cost (each) | Effect |
|------|-------------|--------|
| Rebuilt Stock | $100-200 | Restored function |
| Upgraded OEM | $200-500 | Larger pistons |
| Aftermarket 4-6 piston | $500-1500 | Significantly better |
| Big Brake Kit (Front) | $1500-4000 | Major upgrade |
| Big Brake Kit (Rear) | $1000-2500 | Balanced upgrade |
| Race Calipers | $2000-5000 (pair) | Professional grade |

#### Brake Pads

| Compound | Cost (axle) | Temp Range | Best For |
|----------|-------------|------------|----------|
| Street | $30-80 | Ambient-600°F | Daily |
| Performance Street | $80-150 | Ambient-900°F | Spirited driving |
| Track Day | $150-300 | 200°F-1200°F | Track (cold = noisy) |
| Race | $250-500 | 400°F-1500°F+ | Race (dangerous cold) |

#### Other Brake Parts

| Part | Cost | Effect |
|------|------|--------|
| Stainless Braided Lines | $80-200 | Better pedal feel |
| Upgraded Master Cylinder | $150-400 | Changes pedal ratio |
| Proportioning Valve | $80-200 | Adjustable F/R bias |
| Hydraulic Handbrake | $150-400 | For drift builds |

### 3.7 Wheels & Tires

#### Wheel Specifications

**Diameter Impact:**
- 15-16": Classic/lightweight
- 17-18": Performance sweet spot
- 19-20": Modern aggressive
- 21"+: Show only

**Width Impact:** Wider = More grip, more drag

**Materials:**
| Material | Weight | Cost (each) | Strength |
|----------|--------|-------------|----------|
| Steel | Heavy | Cheap | High |
| Cast Aluminum | Medium | $150-400 | Medium |
| Flow-Formed | Medium-Light | $300-600 | Good |
| Forged Aluminum | Light | $600-2000 | Very high |
| Forged Magnesium | Very light | $1500-3000+ | Good (careful) |

#### Tire Compounds

| Compound | Grip | Wear Rate | Cost (each) | Wet Grip | Best For |
|----------|------|-----------|-------------|----------|----------|
| Economy | 0.70 | 0.5x | $60-100 | 0.65 | Beaters |
| All-Season | 0.78 | 0.6x | $100-150 | 0.72 | Daily |
| Sport | 0.85 | 0.8x | $150-250 | 0.70 | Street performance |
| Performance | 0.95 | 1.2x | $250-400 | 0.65 | Track days |
| Semi-Slick | 1.05 | 2.0x | $300-500 | 0.45 | Track focused |
| Slick | 1.15 | 2.5x | $400-700 | 0.25 | Dry track only |
| Drag Radial | 1.10 | 3.0x | $300-500 | 0.40 | Drag racing |
| Drift Tire | 0.80 | 3.5x | $100-200 | 0.55 | Drift events |

#### Tire Condition System

| Condition | Grip Multiplier |
|-----------|-----------------|
| New (100%) | 1.00 |
| Good (75-99%) | 0.98 |
| Worn (50-74%) | 0.90 |
| Critical (25-49%) | 0.75 |
| Bald (<25%) | 0.50 + failure risk |

---

## 4. VISUAL CUSTOMIZATION

### 4.1 Body Modifications

**Per Vehicle (~15 options each):**
- Front Bumpers
- Rear Bumpers
- Side Skirts
- Fenders

**Spoilers/Wings (~20 options):**
- None (delete)
- OEM Lip/Wing
- Duck Bill
- GT Wing (Low/Medium/High)
- Time Attack Wing
- Chassis-Mount Wing
- Adjustable Wing

**Hoods (~15 options):**
- Stock
- Vented
- Cowl
- Scoop
- Time Attack
- See-Through (Lexan)

**Other:**
- Trunk/Hatch (~6 options)
- Roof (~4 options)
- Mirrors (~5 options)

### 4.2 Lighting

**Headlights/Taillights (~12 options each):**
- Stock, Clear, Smoked, Blacked Out
- Projector/LED Retrofit
- Angel Eyes/Halos
- Sequential Turn Signals

**Underglow:**
- 10 colors × 4 patterns = 40+ combinations
- RGB Color Cycle
- Music Reactive (requires sound system)

### 4.3 Paint & Finish

**Paint Types:**
| Type | Base Cost | Appearance |
|------|-----------|------------|
| Gloss | $500 | Standard shine |
| Matte | $800 | Flat, no reflection |
| Satin | $700 | Between gloss/matte |
| Metallic | $1000 | Metal flake sparkle |
| Pearlescent | $1500 | Color shift |
| Candy | $2000 | Deep, translucent |
| Chrome | $3000 | Mirror finish |
| Color-Shift | $5000 | Changes by angle |

**Color Selection:**
- Full RGB Picker
- 50+ preset period-appropriate colors
- Two-tone options
- 10 custom mix save slots
- Community shared colors

### 4.4 Vinyl/Livery Editor

**Layer System:**
- Maximum 100 layers per side
- Shapes, lines, pre-made graphics (500+)
- Text (multiple fonts)
- Full position/rotation/scale/skew control
- Metallic/Matte per layer
- Mirror to other side

**Wrap Options:**
- Full, Hood, Roof, Trunk, Racing Stripes, Custom zones

### 4.5 Window Tint

| Level | Percentage |
|-------|------------|
| None | Stock |
| Light | 35% |
| Medium | 20% |
| Dark | 5% |
| Limo | 1% |

Colors: Charcoal, Bronze, Blue, Green, Custom

### 4.6 Interior

**Seats:** Stock, Sport, Bucket, Race, Delete
**Steering Wheel:** Stock, Sport, Dish, Flat, Quick Release ($50-400)
**Shift Knob:** Stock, Short Throw, Weighted, Custom shapes ($20-150)
**Gauges:** Stock, Pod Mount, Digital Dash, Holographic

**Roll Cage:**
| Points | Cost |
|--------|------|
| 4-Point | $500-1000 |
| 6-Point | $1000-1500 |
| 8-Point | $1500-2000 |
| 10-Point | $2000-3000 |

---

## 5. VEHICLE CLASSIFICATION

### 5.1 Performance Index (PI)

Vehicles are assigned a Performance Index based on their total performance characteristics. This single number allows for balanced matchmaking and class restrictions.

### 5.2 Performance Classes

| Class | PI Range | Description |
|-------|----------|-------------|
| D | 100-299 | Entry level, beaters |
| C | 300-449 | Street builds |
| B | 450-599 | Serious builds |
| A | 600-749 | High performance |
| S | 750-900 | Elite builds |
| X | 901+ | Unlimited |

### 5.3 Pink Slip Restrictions

Pink slip races require:
- Cars within 50 PI of each other
- Appropriate REP tier for the class
- Cannot wager only car

---

## Related Documents

- [Game Design Document](./GDD.md)
- [Multiplayer Systems Design](./MultiplayerSystems.md)
- [Technical Design Document](../technical/TechnicalDesign.md)

---

**Document Version:** 2.0
**Project:** MIDNIGHT GRIND
**Status:** Development Ready
