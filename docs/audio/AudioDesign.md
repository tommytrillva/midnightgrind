# MIDNIGHT GRIND
## Audio Design Document
### Version 1.0

---

## TABLE OF CONTENTS

1. [Audio Vision](#1-audio-vision)
2. [Audio Pillars](#2-audio-pillars)
3. [Engine Audio System](#3-engine-audio-system)
4. [Vehicle Audio](#4-vehicle-audio)
5. [Environmental Audio](#5-environmental-audio)
6. [Music System](#6-music-system)
7. [UI Audio](#7-ui-audio)
8. [Voice & Communication](#8-voice--communication)
9. [Technical Implementation](#9-technical-implementation)
10. [Mix Guidelines](#10-mix-guidelines)
11. [Asset Production](#11-asset-production)

---

## 1. AUDIO VISION

### 1.1 Audio Identity Statement

The sound of MIDNIGHT GRIND is visceral, aggressive, and authentically tuner. Every engine tells a story of modification and power. The world hums with nocturnal energy—distant traffic, neon buzz, the anticipation before a race. Our audio should make players *feel* the car beneath them and the city around them.

**The Audio Promise:**
> "You don't just hear the car—you feel every RPM, every boost spike, every shift. The engine isn't just sound; it's a direct line to the machine's soul."

### 1.2 Audio Philosophy

```
CORE PRINCIPLES
├── AUTHENTICITY
│   ├── Engine sounds rooted in real recordings
│   ├── Exhaust character matches visual upgrades
│   ├── Mechanical sounds have physical basis
│   └── Environment sounds from real locations
│
├── RESPONSIVENESS
│   ├── Zero perceptible latency
│   ├── Every input has immediate audio feedback
│   ├── Dynamic response to driving style
│   └── Smooth transitions, no pops or clicks
│
├── EMOTION
│   ├── Tension builds before races
│   ├── Triumph in victory
│   ├── Dread in pink slip stakes
│   └── Satisfaction in perfect shifts
│
└── NOSTALGIA
    ├── Era-appropriate music styles
    ├── Sound effects inspired by classic racing games
    ├── "Arcade" feel in UI and feedback
    └── Subtle retro processing options
```

### 1.3 Reference Audio

**Engine References:**
| Platform | Sound Character | Reference |
|----------|-----------------|-----------|
| JDM I4 Turbo | High-pitched whine, turbo spool, aggressive | EVO, WRX, Civic |
| JDM I6 | Smooth, inline scream, iconic | 2JZ Supra, RB26 Skyline |
| Rotary | Unique buzz, high-rev scream | RX-7, RX-8 |
| V8 Muscle | Deep rumble, lopey cam, thunderous | Mustang, Camaro |
| Euro I6 | Refined, precision, controlled aggression | BMW M engines |

**Game Audio References:**
- Need for Speed: Underground 2 (engine layering)
- Forza Horizon series (environmental audio)
- Gran Turismo (mechanical precision)
- Initial D Arcade (dramatic tension)

---

## 2. AUDIO PILLARS

### 2.1 Pillar 1: The Engine Is the Star

The engine sound is the most important audio element. Players will spend hours listening to their car—it must be satisfying, dynamic, and responsive.

```
ENGINE AUDIO REQUIREMENTS
├── Unique sound per engine configuration
├── Audible difference with every modification
├── Dynamic response to throttle, load, RPM
├── Turbo/supercharger clearly audible
├── Exhaust character changes with mods
├── Interior vs exterior perspective
└── Damage/stress affects sound
```

### 2.2 Pillar 2: The World Breathes

The city is alive. Even in quiet moments, there's ambient life—distant traffic, neon hum, the occasional siren. The soundscape should feel inhabited without being cluttered.

```
ENVIRONMENTAL REQUIREMENTS
├── Distinct audio identity per district
├── Time-of-day variations
├── Weather affects everything
├── Reverb/acoustics match spaces
├── Background activity (traffic, crowds)
└── Seamless transitions between areas
```

### 2.3 Pillar 3: Music Sets the Mood

Music is curated to match the street racing fantasy—aggressive, energetic, era-appropriate. The music system adapts to gameplay, intensifying during races and relaxing in social spaces.

```
MUSIC REQUIREMENTS
├── Genre variety (hip-hop, electronic, rock, J-pop)
├── Era-appropriate (late 90s-early 2000s feel)
├── Dynamic mixing (racing vs cruising)
├── Player control (station selection)
├── Seamless transitions
└── Licensed and original tracks
```

### 2.4 Pillar 4: Feedback Is Instant

Every player action has immediate audio feedback. UI sounds are snappy and satisfying. Racing feedback (position changes, near misses, checkpoints) is clear and unambiguous.

```
FEEDBACK REQUIREMENTS
├── <20ms latency for all game audio
├── Distinct sounds for different feedback types
├── Non-intrusive but noticeable
├── Consistent audio language
├── Accessibility options (subtitles, cues)
└── Never annoying with repetition
```

---

## 3. ENGINE AUDIO SYSTEM

### 3.1 Engine Audio Architecture

Engine audio is built using Unreal Engine 5's MetaSounds system, allowing for procedural, granular control over every aspect of the engine sound.

```
ENGINE AUDIO LAYERS
├── LAYER 1: BASE ENGINE LOOP
│   ├── Source: Recorded engine samples
│   ├── Granular playback based on RPM
│   ├── Multiple samples across RPM range
│   └── Crossfaded seamlessly
│
├── LAYER 2: LOAD LAYER
│   ├── Source: On-throttle recordings
│   ├── Blended based on throttle position
│   ├── Adds aggression under load
│   └── Subtle at idle, dominant at WOT
│
├── LAYER 3: EXHAUST CHARACTER
│   ├── Source: Various exhaust recordings
│   ├── Swapped based on exhaust part
│   ├── Affects overall tone
│   └── From muffled stock to raw race
│
├── LAYER 4: INTAKE/INDUCTION
│   ├── Source: Intake recordings
│   ├── Throttle body response
│   ├── Cold air intake whoosh
│   └── Velocity stack trumpet
│
├── LAYER 5: FORCED INDUCTION
│   ├── Turbo: Spool whine, flutter, blow-off
│   ├── Supercharger: Belt whine, boost
│   ├── Intercooler: Pressure sounds
│   └── Boost referenced, not RPM
│
├── LAYER 6: MECHANICAL
│   ├── Valve train clatter
│   ├── Timing chain/belt
│   ├── Accessory drive
│   └── Increases with RPM and cam upgrade
│
└── LAYER 7: STRESS/DAMAGE
    ├── Knock (lean condition)
    ├── Misfire (damage)
    ├── Overheating (stress)
    └── Progressive degradation
```

### 3.2 Engine Type Specifications

#### Inline 4-Cylinder (I4)

```
I4 ENGINE AUDIO PROFILE
├── Character: Buzzy, high-pitched, energetic
├── Base samples: 8-12 across RPM range
├── Redline: 7,000-9,000 RPM typical
├── Notable features:
│   ├── VTEC engagement (Honda): Tone shift at ~5,500 RPM
│   ├── Turbo: Prominent spool whine
│   └── Exhaust: Raspier character
│
├── RPM RANGES:
│   ├── Idle (800-1200): Smooth, subtle
│   ├── Low (1200-3000): Building, slight buzz
│   ├── Mid (3000-5500): Full tone developing
│   ├── High (5500-7500): Screaming, intense
│   └── Redline (7500+): Limiter bark
│
└── MOD VARIATIONS:
    ├── Stock: Muffled, quiet, refined
    ├── Intake: Added induction whistle
    ├── Exhaust: Progressively louder/raspier
    ├── Cam: Changes tone at high RPM
    └── Turbo: Adds spool/flutter layer
```

#### Inline 6-Cylinder (I6)

```
I6 ENGINE AUDIO PROFILE
├── Character: Smooth, linear, iconic scream
├── Base samples: 10-14 across RPM range
├── Redline: 7,000-8,500 RPM typical
├── Notable features:
│   ├── 2JZ: Deep, smooth, legendary
│   ├── RB26: Higher pitched, metallic
│   └── BMW: Refined, precision
│
├── RPM RANGES:
│   ├── Idle (700-1100): Smooth burble
│   ├── Low (1100-2500): Gathering strength
│   ├── Mid (2500-5000): Full, rich tone
│   ├── High (5000-7500): Building scream
│   └── Redline (7500+): Iconic wail
│
└── MOD VARIATIONS:
    ├── Stock: Refined, controlled
    ├── Exhaust: Opens up the scream
    ├── Twin turbo: Dual spool character
    └── Built motor: Higher pitch ceiling
```

#### V8 (American Muscle)

```
V8 ENGINE AUDIO PROFILE
├── Character: Deep, rumbling, thunderous
├── Base samples: 10-14 across RPM range
├── Redline: 6,000-7,500 RPM typical
├── Notable features:
│   ├── Lopey cam idle: Signature rhythm
│   ├── Exhaust burble: Deceleration pops
│   └── Supercharger: Roots whine
│
├── RPM RANGES:
│   ├── Idle (600-900): Lopey, rhythmic (cam dependent)
│   ├── Low (900-2500): Deep rumble
│   ├── Mid (2500-4500): Building thunder
│   ├── High (4500-6500): Full roar
│   └── Redline (6500+): Fury
│
├── CAM PROFILES (Idle character):
│   ├── Stock: Smooth, even
│   ├── Stage 1: Slight lope
│   ├── Stage 2: Noticeable chop
│   ├── Stage 3: Aggressive lope
│   └── Race: Barely idles, wild
│
└── MOD VARIATIONS:
    ├── Headers: Sharper, more aggressive
    ├── Supercharger: Adds whine layer
    ├── No mufflers: Raw, loud, neighbor-hating
    └── Built: Higher revving capability
```

#### Rotary (Wankel)

```
ROTARY ENGINE AUDIO PROFILE
├── Character: Unique buzz, smooth, high-revving
├── Base samples: 8-10 across RPM range
├── Redline: 8,000-10,000 RPM typical
├── Notable features:
│   ├── Distinctive "brap" sound
│   ├── Very smooth power delivery
│   ├── No piston slap or valve noise
│   └── Bridge port changes character dramatically
│
├── RPM RANGES:
│   ├── Idle (900-1200): Smooth, distinct burble
│   ├── Low (1200-3500): Building whir
│   ├── Mid (3500-6500): Sweet spot, full tone
│   ├── High (6500-9000): Screaming
│   └── Redline (9000+): Banshee wail
│
└── MOD VARIATIONS:
    ├── Street port: OEM character
    ├── Bridge port: More aggressive, lumpy
    ├── Peripheral port: Full race, raw
    └── Turbo: Adds significant spool
```

### 3.3 Forced Induction Audio

#### Turbocharger

```
TURBO AUDIO COMPONENTS
├── SPOOL
│   ├── Source: Whine frequency sweep
│   ├── Trigger: Throttle + RPM
│   ├── Character: Rising pitch as boost builds
│   ├── Duration: Turbo size dependent
│   │   ├── Small turbo: 0.3-0.5 sec
│   │   ├── Medium turbo: 0.5-1.0 sec
│   │   ├── Large turbo: 1.0-2.0 sec
│   │   └── Huge turbo: 2.0+ sec
│   └── Volume: Tied to boost pressure
│
├── SUSTAINED BOOST
│   ├── Source: Constant whine at frequency
│   ├── Pitch: Boost pressure dependent
│   ├── Modulation: Slight flutter
│   └── Blends with engine sound
│
├── FLUTTER (Compressor surge)
│   ├── Trigger: Throttle lift at boost
│   ├── Character: "Stututu" sound
│   ├── Frequency: Rapid oscillation
│   ├── Duration: 0.3-1.0 sec
│   └── Intensity: Boost level dependent
│
├── BLOW-OFF VALVE
│   ├── Trigger: Throttle lift at boost
│   ├── Type 1 (Recirculating): Subtle whoosh
│   ├── Type 2 (Atmospheric): "Psshh" sound
│   ├── Type 3 (Aggressive): Sharp, loud
│   └── Duration: 0.2-0.5 sec
│
└── WASTEGATE
    ├── Source: Exhaust bypass sound
    ├── Character: Raspy, harsh
    ├── Trigger: Boost limit reached
    └── Volume: Tied to boost pressure
```

#### Supercharger

```
SUPERCHARGER AUDIO COMPONENTS
├── ROOTS TYPE
│   ├── Character: Classic muscle whine
│   ├── Pitch: Pulley ratio × RPM
│   ├── Volume: Always present when running
│   ├── Note: Instant response, no spool
│   └── Example: Hellcat, classic muscle
│
├── TWIN-SCREW
│   ├── Character: Higher pitched whine
│   ├── Pitch: RPM dependent
│   ├── Volume: Increases with throttle
│   └── More efficient sound
│
├── CENTRIFUGAL
│   ├── Character: Similar to turbo
│   ├── Pitch: RPM dependent (belt-driven)
│   ├── No spool lag
│   ├── Increasing whine with RPM
│   └── Common on Mustangs, Corvettes
│
└── BYPASS VALVE (when equipped)
    ├── Trigger: Light throttle / idle
    ├── Character: Slight air release
    └── Reduces parasitic loss sound
```

### 3.4 Exhaust System Audio

```
EXHAUST CONFIGURATION EFFECTS
├── STOCK EXHAUST
│   ├── Character: Muffled, refined, quiet
│   ├── Volume: Low
│   ├── Tone: Bassy, no harshness
│   └── Pops/burbles: Minimal
│
├── SPORT EXHAUST
│   ├── Character: Opens up, deeper
│   ├── Volume: Medium
│   ├── Tone: Fuller, some raspiness
│   └── Pops/burbles: Noticeable on decel
│
├── RACE EXHAUST
│   ├── Character: Aggressive, loud
│   ├── Volume: High
│   ├── Tone: Raw, significant rasp
│   └── Pops/burbles: Prominent
│
├── STRAIGHT PIPE (No muffler)
│   ├── Character: Raw, unfiltered
│   ├── Volume: Very high
│   ├── Tone: Engine character exposed
│   └── Pops/burbles: Extreme
│
├── CATLESS
│   ├── Effect: +rasp, +volume
│   ├── Adds metallic edge
│   └── Cumulative with muffler choice
│
└── HEADER UPGRADE
    ├── Effect: Sharper throttle response
    ├── Slightly higher pitch
    └── More "alive" sound

EXHAUST TIP RESONANCE (Minor)
├── Single tip: Focused
├── Dual tip: Slightly wider
├── Quad tip: Perception of power
└── Note: Mostly cosmetic audio effect
```

### 3.5 Transmission Audio

```
MANUAL TRANSMISSION
├── SHIFT SOUNDS
│   ├── Upshift: Mechanical clunk + engine drop
│   ├── Downshift: Clunk + engine rise + rev match
│   ├── Character: Varies by transmission type
│   │   ├── Stock: Notchy, defined
│   │   ├── Short throw: Quicker, tighter
│   │   └── Sequential: Rapid, pneumatic
│   └── Synchro whine: Subtle under load
│
├── GEAR WHINE
│   ├── Source: Gear mesh frequency
│   ├── Pitch: Speed × gear ratio
│   ├── Volume: Subtle in most gears
│   └── Note: More prominent in lower gears
│
├── CLUTCH
│   ├── Engagement: Grab + slight lurch
│   ├── Slip: Subtle friction sound
│   ├── Dump: Hard engagement thunk
│   └── Upgraded clutch: More aggressive grab
│
└── SEQUENTIAL TRANSMISSION
    ├── Upshift: Sharp "clack" + pneumatic
    ├── Downshift: Similar + auto-blip
    ├── No clutch sound (clutchless)
    └── Premium feel
```

---

## 4. VEHICLE AUDIO

### 4.1 Tire Audio

```
TIRE SOUNDS
├── ROLLING
│   ├── Source: Tire/road contact
│   ├── Pitch: Speed dependent
│   ├── Character: Surface dependent
│   │   ├── Smooth asphalt: Low hum
│   │   ├── Rough asphalt: Louder texture
│   │   ├── Concrete: Rhythmic joints
│   │   ├── Wet: Added spray sound
│   │   └── Gravel: Crunch and ping
│   └── Volume: Speed dependent
│
├── SQUEAL (Cornering)
│   ├── Trigger: Slip angle threshold
│   ├── Pitch: Slip intensity
│   ├── Character: Rubber protest
│   └── Duration: While slipping
│
├── BURNOUT
│   ├── Trigger: Wheel spin + stationary
│   ├── Character: Intense, sustained squeal
│   ├── Smoke audio: Accompanying hiss
│   └── Duration: While burning out
│
├── DRIFT
│   ├── Trigger: Sustained slip angle
│   ├── Character: Modulated squeal
│   ├── Pitch: Varies with angle
│   └── Blend: Squeal + smoke
│
├── LOCKUP
│   ├── Trigger: Full brake + slip
│   ├── Character: Harsh screech
│   ├── Duration: Until release/ABS
│   └── Distinct from cornering squeal
│
└── IMPACT
    ├── Pothole: Thump + suspension
    ├── Curb: Sharp impact
    ├── Rumble strip: Rapid vibration
    └── Surface transition: Texture change
```

### 4.2 Suspension Audio

```
SUSPENSION SOUNDS
├── COMPRESSION
│   ├── Bump: Progressive thump
│   ├── Big hit: Louder, longer
│   ├── Bottoming out: Hard clunk
│   └── Character: Damper dependent
│
├── REBOUND
│   ├── Recovery: Subtle creak
│   ├── Rapid rebound: Quick pop
│   └── Over-extended: Clunk
│
├── BODY ROLL
│   ├── Source: Chassis flex
│   ├── Character: Creaks, groans
│   ├── Trigger: Hard cornering
│   └── More prominent: Older cars
│
├── UPGRADED SUSPENSION
│   ├── Coilovers: Tighter, less noise
│   ├── Air suspension: Compressor pump
│   └── Racing: Very stiff, precise
│
└── DAMAGE
    ├── Worn bushings: Clunks
    ├── Broken spring: Scraping
    └── Shock failure: Bouncy resonance
```

### 4.3 Brake Audio

```
BRAKE SOUNDS
├── LIGHT BRAKING
│   ├── Character: Subtle, controlled
│   ├── Pad contact: Light friction
│   └── Volume: Minimal
│
├── MODERATE BRAKING
│   ├── Character: Firm grip
│   ├── Pad contact: Defined friction
│   └── Weight transfer: Nose dive creak
│
├── HARD BRAKING
│   ├── Character: Maximum grip
│   ├── ABS: Rapid pulse (if equipped)
│   ├── Thermal: Slight hiss (hot brakes)
│   └── Tire contribution: Begins to squeal
│
├── BRAKE SQUEAL
│   ├── Trigger: Light braking + specific conditions
│   ├── Character: High-pitched whine
│   └── Note: Race pads more prone
│
├── ROTOR CONDITION
│   ├── New: Clean, quiet
│   ├── Worn: Slight grinding
│   ├── Warped: Pulsing, vibration
│   └── Damaged: Harsh grinding
│
└── HANDBRAKE
    ├── Engagement: Cable/lever click + grab
    ├── Lockup: Immediate rear squeal
    └── Hydraulic: Sharper, instant
```

### 4.4 Wind & Aerodynamics

```
WIND AUDIO
├── CABIN WIND NOISE
│   ├── Source: Air movement around car
│   ├── Pitch: Speed dependent
│   ├── Volume: Increases with speed
│   │   ├── 0-40 mph: Minimal
│   │   ├── 40-80 mph: Building
│   │   ├── 80-120 mph: Prominent
│   │   └── 120+ mph: Roaring
│   └── Window open: Significant increase
│
├── AERODYNAMIC BUFFETING
│   ├── Trigger: High speed + open windows
│   ├── Character: Rhythmic pressure pulses
│   └── Frequency: Vehicle dependent
│
├── WING/SPOILER
│   ├── Large wing: Adds turbulence note
│   ├── Speed dependent
│   └── Subtle contribution
│
└── BODY GAPS
    ├── Panel gaps: Whistle at speed
    ├── Damage: Increased noise
    └── Wide body: Different airflow
```

### 4.5 Impact & Collision Audio

```
COLLISION SOUNDS
├── LIGHT CONTACT (Scrape)
│   ├── Metal on concrete: Grinding
│   ├── Metal on metal: Scraping
│   ├── Plastic on concrete: Scratching
│   └── Duration: Contact length
│
├── MODERATE IMPACT
│   ├── Character: Thump + crumple
│   ├── Glass: Possible crack
│   ├── Body panel: Dent sound
│   └── Suspension stress: Clunk
│
├── HARD IMPACT
│   ├── Character: Loud crash
│   ├── Glass: Shatter (if broken)
│   ├── Metal: Tearing, folding
│   ├── Air bag: Deployment pop
│   └── Debris: Scattering sounds
│
├── WALL/BARRIER
│   ├── Concrete: Hard, resonant
│   ├── Metal guardrail: Ringing, scraping
│   ├── Tire barrier: Softer, absorbing
│   └── Chain fence: Rattling
│
├── VEHICLE-VEHICLE
│   ├── Sideswipe: Grinding + panel contact
│   ├── Rear-end: Impact + glass potential
│   ├── T-bone: Significant crash
│   └── Other car audio: Their damage sounds
│
└── DEBRIS
    ├── Parts falling: Clattering
    ├── Glass pieces: Tinkling
    ├── Sparks: Accompanying sizzle
    └── Fluids: Dripping/spraying
```

---

## 5. ENVIRONMENTAL AUDIO

### 5.1 District Soundscapes

#### Downtown

```
DOWNTOWN AUDIO PROFILE
├── BASE AMBIENCE
│   ├── Traffic: Constant low rumble
│   ├── Crowd: Distant voices, footsteps
│   ├── Neon: Electric hum/buzz
│   ├── HVAC: Building systems
│   └── Volume: Medium-high
│
├── SPOT SOUNDS
│   ├── Car horns: Occasional
│   ├── Sirens: Distant, periodic
│   ├── Music: Leaking from venues
│   ├── Pedestrians: Conversations
│   ├── Subway: Rumble (if applicable)
│   └── Construction: Night work
│
├── REVERB CHARACTER
│   ├── Type: Urban canyon
│   ├── Decay: Long (building reflections)
│   ├── Early reflections: Strong
│   └── Note: Engine sounds echo
│
└── TIME VARIATIONS
    ├── 9PM-12AM: Peak activity
    ├── 12AM-3AM: Reduced, intimate
    └── 3AM-6AM: Quiet, sparse
```

#### Industrial Zone

```
INDUSTRIAL AUDIO PROFILE
├── BASE AMBIENCE
│   ├── Machinery: Distant hum, occasional bangs
│   ├── Traffic: Sparse, mostly trucks
│   ├── Wind: More prominent (open spaces)
│   └── Volume: Low-medium
│
├── SPOT SOUNDS
│   ├── Forklifts: Beeping, engine
│   ├── Loading: Metal clangs
│   ├── Generators: Constant hum
│   ├── Steam: Releases, hisses
│   └── Dogs: Occasional barking
│
├── REVERB CHARACTER
│   ├── Type: Open + warehouse echoes
│   ├── Decay: Variable (inside vs outside)
│   ├── Metal resonance: Prominent
│   └── Note: Drag sounds incredible here
│
└── ACOUSTIC ZONES
    ├── Container yard: Muffled, enclosed
    ├── Open lots: Wide, dissipating
    └── Warehouses: Reverberant
```

#### Canyon/Hills

```
CANYON AUDIO PROFILE
├── BASE AMBIENCE
│   ├── Wind: Constant presence
│   ├── Nature: Crickets, rustling leaves
│   ├── Wildlife: Distant animals
│   └── Volume: Low
│
├── SPOT SOUNDS
│   ├── Owls: Night calls
│   ├── Wind gusts: Through trees
│   ├── Falling rocks: Occasional
│   └── Water: Streams (if present)
│
├── REVERB CHARACTER
│   ├── Type: Natural canyon echo
│   ├── Decay: Medium-long
│   ├── Character: Natural, organic
│   └── Note: Engine echoes off cliffs
│
├── TUNNEL SECTIONS
│   ├── Enter: Sound closes in
│   ├── Inside: Maximum echo
│   ├── Exit: Sound opens up
│   └── Effect: Dramatic
│
└── ELEVATION CHANGES
    ├── Low areas: More enclosed
    ├── Summit: Open, wind prominent
    └── Valleys: Echo traps
```

### 5.2 Weather Audio

```
RAIN AUDIO SYSTEM
├── LIGHT RAIN
│   ├── Drops on car: Light pattering
│   ├── Road spray: Subtle
│   ├── Ambience: Soft rainfall
│   └── Wipers: Slow rhythm
│
├── HEAVY RAIN
│   ├── Drops on car: Loud, dense
│   ├── Road spray: Prominent rooster tail
│   ├── Ambience: Loud rainfall
│   ├── Thunder: Occasional
│   ├── Visibility: Muffled distant sounds
│   └── Wipers: Fast rhythm
│
├── SURFACE INTERACTION
│   ├── Standing water: Splash
│   ├── Puddles: Impact
│   ├── Flowing water: Tire cutting
│   └── Hydroplaning: Rushing water
│
└── INTERIOR PERSPECTIVE
    ├── Roof: Rain on metal
    ├── Windows: Streaking
    ├── Reduced exterior: Isolation
    └── Climate control: Fan noise

FOG
├── Character: Muffled, isolated
├── Distant sounds: Attenuated
├── Fog horns: Harbor (if near port)
└── Own car sounds: More prominent
```

### 5.3 Traffic & NPC Audio

```
TRAFFIC AUDIO
├── NPC VEHICLES
│   ├── Passing: Doppler effect
│   ├── Idle: Engine at rest
│   ├── Acceleration: Power buildup
│   └── Horn: Angry response (if provoked)
│
├── VEHICLE VARIETY
│   ├── Economy cars: Quiet, small
│   ├── Trucks: Diesel, loud
│   ├── Sports cars: Revving, exhaust
│   ├── Motorcycles: High-pitched
│   └── Emergency: Sirens, different types
│
├── TRAFFIC DENSITY
│   ├── Light: Occasional passes
│   ├── Medium: Regular flow
│   ├── Heavy: Constant presence
│   └── Jam: Idling, horns
│
└── HIGHWAY SPECIFIC
    ├── High-speed passes: Fast Doppler
    ├── Truck convoys: Rumbling
    ├── Wind wake: From large vehicles
    └── Elevated: Sound below/around
```

### 5.4 Location-Specific Audio

```
SPECIAL LOCATIONS
├── MEET SPOT
│   ├── Engines: Multiple, idling/revving
│   ├── Music: Various cars playing
│   ├── Crowd: Enthusiast conversations
│   ├── Photos: Camera shutters
│   └── Atmosphere: Social, energetic
│
├── GAS STATION
│   ├── Pumps: Fuel flowing
│   ├── Store: Door chime
│   ├── Lights: Electric hum
│   └── Customers: Brief interactions
│
├── GARAGE/SHOP
│   ├── Tools: Air ratchets, welding
│   ├── Radio: Background music
│   ├── Lifts: Hydraulic
│   ├── Metal work: Clanging
│   └── Ventilation: Fan hum
│
├── DRAG STRIP
│   ├── Staging: Burnouts
│   ├── Launch: Full throttle assault
│   ├── PA system: Announcer
│   └── Crowd: Cheering
│
└── POLICE STATION (To avoid)
    ├── Radios: Dispatch chatter
    ├── Sirens: Occasional departure
    └── Activity: Patrol movement
```

---

## 6. MUSIC SYSTEM

### 6.1 Music Philosophy

Music in MIDNIGHT GRIND sets the emotional tone and reinforces the late 90s/early 2000s street racing fantasy. The music system is dynamic, responding to gameplay state.

### 6.2 Genre Mix

```
MUSIC GENRES
├── HIP-HOP / RAP (35%)
│   ├── Era: 1998-2005 style
│   ├── Character: Aggressive, bass-heavy
│   ├── Usage: General driving, meets
│   └── Reference: NFS Underground vibes
│
├── ELECTRONIC / DRUM & BASS (25%)
│   ├── Era: Late 90s - early 2000s
│   ├── Character: High energy, driving beats
│   ├── Usage: Racing, high-speed
│   └── Reference: Wipeout, Ridge Racer
│
├── ROCK / METAL (20%)
│   ├── Era: Nu-metal, alt-rock
│   ├── Character: Aggressive, guitar-driven
│   ├── Usage: Intense moments, muscle culture
│   └── Reference: Era-appropriate sound
│
├── J-POP / EUROBEAT (15%)
│   ├── Era: Initial D style
│   ├── Character: Energetic, dramatic
│   ├── Usage: Touge, drift events
│   └── Reference: Authentic JDM culture
│
└── AMBIENT / CHILL (5%)
    ├── Character: Atmospheric
    ├── Usage: Garage, menus, quiet moments
    └── Note: Late-night vibes
```

### 6.3 Dynamic Music System

```
MUSIC STATES
├── CRUISING
│   ├── Energy: Low-medium
│   ├── Full tracks: Yes
│   ├── Volume: Medium
│   └── Station DJ: Between songs
│
├── MEET/SOCIAL
│   ├── Energy: Medium
│   ├── Full tracks: Yes
│   ├── Volume: Medium (competing with crowd)
│   └── Source: In-world (other cars, speakers)
│
├── RACE STARTING
│   ├── Energy: Building
│   ├── Transition: Music builds tension
│   ├── Volume: Rising
│   └── Effect: Anticipation
│
├── RACING
│   ├── Energy: High
│   ├── Track selection: High-energy only
│   ├── Volume: High
│   ├── Mix: Sidechained to engine
│   └── Intensifiers: Position changes, near misses
│
├── RACE WON
│   ├── Energy: Peak then release
│   ├── Track: Victory sting + continues
│   ├── Volume: High then normalizing
│   └── Mood: Triumphant
│
├── RACE LOST
│   ├── Energy: Drop
│   ├── Track: Somber transition
│   ├── Volume: Reduced
│   └── Mood: Disappointment
│
├── PINK SLIP
│   ├── Energy: Maximum tension
│   ├── Track: Special tension music
│   ├── Volume: High
│   └── Heart rate: Elevated
│
├── POLICE PURSUIT
│   ├── Energy: High, urgent
│   ├── Track: Pursuit-specific
│   ├── Volume: High
│   └── Character: Tense, driving
│
└── GARAGE
    ├── Energy: Low, contemplative
    ├── Track: Chill selections
    ├── Volume: Background
    └── Mood: Focus on car
```

### 6.4 Radio Station System

```
RADIO STATIONS
├── MIDNIGHT FM (Hip-Hop/Rap)
│   ├── DJ: Personality, street culture
│   ├── Tracks: 20-30
│   └── Ads: In-game businesses
│
├── STREET FREQUENCY (Electronic)
│   ├── DJ: Club culture, energetic
│   ├── Tracks: 20-30
│   └── Ads: Parts shops, events
│
├── OCTANE RADIO (Rock)
│   ├── DJ: Aggressive, muscle culture
│   ├── Tracks: 20-30
│   └── Ads: Muscle car focused
│
├── TOKYO NIGHTS (J-Pop/Eurobeat)
│   ├── DJ: Import culture, authentic
│   ├── Tracks: 15-25
│   └── Ads: JDM parts, import shops
│
├── AFTER HOURS (Chill/Ambient)
│   ├── DJ: Minimal, smooth
│   ├── Tracks: 15-20
│   └── Ads: Minimal
│
└── CUSTOM PLAYLIST (Player)
    ├── Tracks: Player selection
    ├── No DJ
    └── Volume control
```

---

## 7. UI AUDIO

### 7.1 UI Audio Philosophy

UI audio should be satisfying, consistent, and never annoying. Sounds reinforce actions and provide feedback. The aesthetic leans into the era-appropriate arcade/game feel.

### 7.2 UI Sound Categories

```
NAVIGATION SOUNDS
├── CURSOR MOVE
│   ├── Character: Quick, subtle tick
│   ├── Pitch: Slight variation
│   └── Volume: Low
│
├── SELECTION CONFIRM
│   ├── Character: Satisfying click/beep
│   ├── Pitch: Rising
│   ├── Variants: Context-dependent
│   └── Volume: Medium
│
├── BACK/CANCEL
│   ├── Character: Lower, receding
│   ├── Pitch: Falling
│   └── Volume: Medium-low
│
├── ERROR/INVALID
│   ├── Character: Buzzer, negative
│   ├── Pitch: Dissonant
│   └── Usage: Insufficient funds, locked content
│
└── SCROLL
    ├── Character: Light scroll ticks
    ├── Rate: Matched to scroll speed
    └── Volume: Low

ACTION SOUNDS
├── PURCHASE
│   ├── Character: Cash register + success
│   ├── Pitch: Positive
│   └── Satisfaction: High
│
├── PART INSTALL
│   ├── Character: Mechanical click + wrench
│   ├── Pitch: Solid, heavy
│   └── Variants: Part type specific
│
├── PART REMOVE
│   ├── Character: Uninstall sound
│   ├── Pitch: Lower than install
│   └── Distinct from install
│
├── PAINT APPLY
│   ├── Character: Spray can / smooth wash
│   └── Satisfying reveal
│
├── SAVE
│   ├── Character: Confirmation chime
│   └── Brief, non-intrusive
│
└── LEVEL UP / UNLOCK
    ├── Character: Celebratory sting
    ├── Duration: 1-2 seconds
    └── Impact: Noticeable achievement
```

### 7.3 Notification Sounds

```
NOTIFICATIONS
├── RACE INVITE
│   ├── Character: Alert, attention-grabbing
│   ├── Duration: Short
│   └── Urgency: Medium
│
├── CHALLENGE RECEIVED
│   ├── Character: Competitive ping
│   ├── Duration: Short
│   └── Urgency: Medium-high
│
├── PINK SLIP CHALLENGE
│   ├── Character: Dramatic, ominous
│   ├── Duration: Longer sting
│   └── Urgency: High
│
├── MESSAGE RECEIVED
│   ├── Character: Social ping
│   ├── Duration: Very short
│   └── Urgency: Low
│
├── POLICE ALERT
│   ├── Character: Warning, tension
│   ├── Duration: Medium
│   └── Urgency: High
│
└── ACHIEVEMENT
    ├── Character: Celebratory
    ├── Duration: Medium
    └── Importance: High visibility
```

### 7.4 Racing UI Sounds

```
RACING FEEDBACK
├── COUNTDOWN
│   ├── 3-2-1: Building beeps
│   ├── GO: Powerful start sound
│   └── False start: Buzzer
│
├── CHECKPOINT
│   ├── Pass: Quick confirmation
│   ├── Miss: Warning
│   └── Distinct from position sounds
│
├── POSITION CHANGE
│   ├── Gain position: Upward sting
│   ├── Lose position: Downward sting
│   └── Quick, non-intrusive
│
├── LAP COMPLETE
│   ├── Character: Confirmation
│   ├── Final lap: Urgent variant
│   └── Best lap: Celebratory variant
│
├── NEAR MISS
│   ├── Character: Woosh + reward sting
│   ├── Duration: Very short
│   └── Impact: Encouraging
│
├── COLLISION WARNING
│   ├── Character: Alert tone
│   ├── Duration: Instant
│   └── Usage: Damage taken
│
└── FINISH
    ├── Win: Victory fanfare
    ├── Podium: Positive sting
    ├── Loss: Somber tone
    └── Duration: Transitions to results
```

---

## 8. VOICE & COMMUNICATION

### 8.1 Voice Chat

```
VOICE CHAT SYSTEM
├── PROXIMITY VOICE
│   ├── Range: 50m maximum
│   ├── Falloff: Natural distance attenuation
│   ├── Processing: Light compression
│   └── Spatialization: 3D positioned
│
├── PARTY/CREW VOICE
│   ├── Range: Unlimited (same party)
│   ├── Processing: Clear, consistent
│   ├── Priority: Over proximity
│   └── Volume: Adjustable
│
├── IN-CAR EFFECT
│   ├── Inside own car: Clear
│   ├── Others in cars: Slightly muffled
│   ├── Window open/closed: Varies
│   └── Engine: Competes with voice
│
└── QUALITY OPTIONS
    ├── Codec: Opus
    ├── Bitrate: 24-64 kbps
    ├── Noise gate: Optional
    └── Echo cancellation: Enabled
```

### 8.2 Rival Voice Lines

```
RIVAL VO SYSTEM
├── CHALLENGE
│   ├── Lines: 5-10 per rival
│   ├── Context: Before race
│   ├── Character: Personality-dependent
│   └── Example: "Think that thing can keep up?"
│
├── RACING
│   ├── Lines: 10-15 per rival
│   ├── Context: During race
│   ├── Triggers: Pass, crash, boost, etc.
│   └── Example: "Where'd you learn to drive?"
│
├── VICTORY
│   ├── Lines: 5-10 per rival
│   ├── Context: They win
│   ├── Character: Gloating to respectful
│   └── Example: "Better luck next time."
│
├── DEFEAT
│   ├── Lines: 5-10 per rival
│   ├── Context: They lose
│   ├── Character: Sore to gracious
│   └── Example: "You got lucky..."
│
└── PINK SLIP
    ├── Stakes lines: Unique tension
    ├── Win: Devastated
    ├── Lose: Triumphant
    └── Memorable moments
```

---

## 9. TECHNICAL IMPLEMENTATION

### 9.1 MetaSounds Architecture

```
METASOUNDS STRUCTURE
├── ENGINE SYSTEM
│   ├── MSGraph_EngineCore
│   │   ├── Input: RPM, Throttle, Load
│   │   ├── Granular players (per RPM range)
│   │   ├── Crossfade logic
│   │   └── Output: Base engine sound
│   │
│   ├── MSGraph_ExhaustLayer
│   │   ├── Input: Engine output, Exhaust type
│   │   ├── Filtering based on muffler
│   │   └── Output: Exhaust-colored engine
│   │
│   ├── MSGraph_ForcedInduction
│   │   ├── Input: Boost, RPM, Type
│   │   ├── Spool/flutter/BOV logic
│   │   └── Output: Turbo/SC layer
│   │
│   └── MSGraph_EngineMaster
│       ├── Combines all layers
│       ├── Distance attenuation
│       ├── Interior/exterior switch
│       └── Final mix output
│
├── ENVIRONMENT SYSTEM
│   ├── MSGraph_AmbientBed
│   │   ├── District-based selection
│   │   ├── Time-of-day variation
│   │   └── Weather modulation
│   │
│   └── MSGraph_DynamicSpots
│       ├── Random interval triggers
│       ├── Position-based selection
│       └── Event-driven sounds
│
└── MUSIC SYSTEM
    ├── MSGraph_RadioSystem
    │   ├── Track queue management
    │   ├── DJ interjections
    │   ├── State-based mixing
    │   └── Crossfading
    │
    └── MSGraph_DynamicMix
        ├── Sidechain to gameplay
        ├── State transitions
        └── Ducking priorities
```

### 9.2 Audio Occlusion & Propagation

```
OCCLUSION SYSTEM
├── VEHICLE INTERIOR
│   ├── Windows up: Full occlusion
│   ├── Windows down: Partial occlusion
│   └── Convertible: Minimal occlusion
│
├── ENVIRONMENT
│   ├── Buildings: Full occlusion
│   ├── Tunnels: Reverb change + occlusion
│   ├── Underpasses: Brief occlusion
│   └── Open areas: No occlusion
│
├── RAYCAST SYSTEM
│   ├── Rays from listener to source
│   ├── Material-based absorption
│   ├── Update rate: 30Hz
│   └── Smoothed transitions
│
└── REVERB ZONES
    ├── Type: Convolution impulses
    ├── Zones: Per area/building
    ├── Blend: Distance-based
    └── Performance: Zone limiting
```

### 9.3 Performance Optimization

```
AUDIO PERFORMANCE BUDGET
├── VOICE COUNT
│   ├── Real voices: 32 max
│   ├── Virtual voices: 64
│   ├── Priority system: Active
│   └── Distance culling: Enabled
│
├── MEMORY
│   ├── Streaming pool: 128MB
│   ├── Resident: 64MB
│   ├── Per-engine data: ~10MB
│   └── Environment: ~20MB
│
├── CPU
│   ├── Target: <5% main thread
│   ├── Async processing: Enabled
│   ├── MetaSounds: Graph optimization
│   └── Occlusion: Spatial hashing
│
└── STREAMING
    ├── Prefetch: Zone-based
    ├── Unload: Distance-based
    ├── Priority: Gameplay critical first
    └── Buffer: 2 seconds ahead
```

### 9.4 Platform Considerations

```
PLATFORM SPECIFICS
├── PC
│   ├── Sample rate: 48kHz
│   ├── Channels: 7.1 support
│   ├── Headphone mode: HRTF
│   └── Output: WASAPI/DirectSound
│
├── CONSOLE (Future)
│   ├── Sample rate: 48kHz
│   ├── Channels: Platform native
│   ├── 3D audio: Platform APIs
│   └── Memory: Platform constraints
│
└── ACCESSIBILITY
    ├── Subtitles: All dialogue
    ├── Visual cues: For critical audio
    ├── Volume sliders: Per category
    └── Mono mode: Option
```

---

## 10. MIX GUIDELINES

### 10.1 Volume Hierarchy

```
MIX PRIORITY (Loudest to quietest)
├── 1. PLAYER ENGINE
│   ├── Always audible
│   ├── Central to experience
│   └── dB: Reference (0dB)
│
├── 2. CRITICAL FEEDBACK
│   ├── Checkpoint, position
│   ├── Warnings, alerts
│   └── dB: -3 to 0dB
│
├── 3. COLLISION / IMPACT
│   ├── Player collisions
│   ├── Important feedback
│   └── dB: -6 to 0dB
│
├── 4. OTHER VEHICLES
│   ├── Opponents, traffic
│   ├── Distance attenuated
│   └── dB: -12 to -6dB
│
├── 5. MUSIC
│   ├── Sidechained to engine
│   ├── State dependent
│   └── dB: -18 to -9dB
│
├── 6. ENVIRONMENT
│   ├── Ambient bed
│   ├── Spot sounds
│   └── dB: -24 to -12dB
│
└── 7. UI
    ├── Non-intrusive
    ├── Clear feedback
    └── dB: -18 to -6dB
```

### 10.2 Dynamic Range

```
DYNAMIC RANGE TARGETS
├── OVERALL
│   ├── Target: -14 LUFS (integrated)
│   ├── Peak: -1dB true peak
│   └── Range: 8-12 LU
│
├── MUSIC
│   ├── Target: -16 LUFS per track
│   ├── Normalized on import
│   └── Headroom for dynamics
│
├── SFX
│   ├── Transients: Preserved
│   ├── Light limiting only
│   └── No heavy compression
│
└── DIALOGUE/VO
    ├── Target: -18 LUFS
    ├── Consistent levels
    └── Clear intelligibility
```

### 10.3 Frequency Balance

```
FREQUENCY ALLOCATION
├── SUB BASS (20-60Hz)
│   ├── Engine fundamental
│   ├── Subwoofer hits
│   └── Music bass
│
├── BASS (60-250Hz)
│   ├── Engine body
│   ├── Exhaust fullness
│   ├── Music bass
│   └── Road rumble
│
├── LOW MIDS (250-500Hz)
│   ├── Engine warmth
│   ├── Body resonance
│   └── Careful: Mud zone
│
├── MIDS (500Hz-2kHz)
│   ├── Engine character
│   ├── Exhaust tone
│   ├── Vocal clarity
│   └── Primary information
│
├── HIGH MIDS (2-6kHz)
│   ├── Engine presence
│   ├── Turbo whine
│   ├── Squeal/screech
│   └── Attack transients
│
└── HIGHS (6-20kHz)
    ├── Air, sparkle
    ├── Sibilance
    ├── High-freq detail
    └── Roll off gently
```

---

## 11. ASSET PRODUCTION

### 11.1 Recording Standards

```
RECORDING SPECIFICATIONS
├── FORMAT
│   ├── Sample rate: 96kHz (source)
│   ├── Bit depth: 24-bit
│   ├── Channels: Mono (most), Stereo (ambience)
│   └── Delivery: 48kHz/24-bit
│
├── MICROPHONES (Engine recording)
│   ├── Exhaust: Dynamic (SM57, MD421)
│   ├── Intake: Condenser (small diaphragm)
│   ├── Engine bay: Condenser
│   ├── Interior: Binaural or stereo pair
│   └── Distant: Shotgun
│
├── RECORDING SESSION
│   ├── RPM sweeps: Slow, full range
│   ├── Load variations: On/off throttle
│   ├── Specific sounds: Startup, shutdown, rev
│   ├── Mods: Each configuration recorded
│   └── Environment: Isolated if possible
│
└── POST-PROCESSING
    ├── Noise reduction: Minimal, careful
    ├── EQ: Corrective only
    ├── Normalization: Peak normalized
    └── Editing: Clean starts/ends
```

### 11.2 File Naming Convention

```
AUDIO FILE NAMING
[Category]_[SubCategory]_[Name]_[Variant]_[Detail]

EXAMPLES:
├── VH_Engine_I4Turbo_2000RPM_OnLoad
├── VH_Engine_V8Muscle_Idle_CamStage2
├── VH_Exhaust_Stock_I4
├── VH_Turbo_Spool_Medium
├── VH_Tire_Squeal_Cornering
├── ENV_Downtown_Ambience_Night
├── ENV_Rain_Heavy_Interior
├── UI_Navigation_Select
├── UI_Purchase_Success
├── MUS_Track_HipHop_01
└── VO_Rival_Ken_Challenge_01

CATEGORIES:
├── VH_ : Vehicle
├── ENV_ : Environment
├── UI_ : User Interface
├── MUS_ : Music
├── VO_ : Voice Over
└── FX_ : Special Effects
```

### 11.3 Folder Structure

```
/Content/Audio
├── /Vehicle
│   ├── /Engines
│   │   ├── /I4
│   │   ├── /I6
│   │   ├── /V8
│   │   ├── /Rotary
│   │   └── /Shared
│   ├── /Exhaust
│   ├── /Turbo
│   ├── /Supercharger
│   ├── /Transmission
│   ├── /Tires
│   ├── /Suspension
│   ├── /Brakes
│   ├── /Collision
│   └── /Misc
│
├── /Environment
│   ├── /Downtown
│   ├── /Industrial
│   ├── /Port
│   ├── /Highway
│   ├── /Canyon
│   ├── /Suburbs
│   ├── /Weather
│   └── /Shared
│
├── /UI
│   ├── /Navigation
│   ├── /Actions
│   ├── /Notifications
│   └── /Racing
│
├── /Music
│   ├── /Tracks
│   ├── /DJ
│   ├── /Stings
│   └── /Ambient
│
├── /VO
│   ├── /Rivals
│   ├── /DJ
│   └── /System
│
└── /MetaSounds
    ├── /Engine
    ├── /Environment
    ├── /Music
    └── /Shared
```

### 11.4 Quality Checklist

```
AUDIO QA CHECKLIST
├── TECHNICAL
│   ☐ Correct sample rate (48kHz)
│   ☐ Correct bit depth (24-bit or 16-bit final)
│   ☐ No clipping
│   ☐ No unwanted noise
│   ☐ Clean edits (no clicks/pops)
│   ☐ Appropriate length
│   ☐ Loop points clean (if looping)
│
├── CREATIVE
│   ☐ Matches design intent
│   ☐ Appropriate character/tone
│   ☐ Works in context
│   ☐ Not fatiguing on repeat
│   ☐ Reviewed by lead
│
├── INTEGRATION
│   ☐ Correct folder location
│   ☐ Correct naming convention
│   ☐ Metadata correct
│   ☐ Working in engine
│   ☐ Attenuation appropriate
│   ☐ No memory issues
│
└── FINAL
    ☐ Plays correctly in all contexts
    ☐ Mix balance verified
    ☐ No conflicts with other sounds
    ☐ Approved for ship
```

---

## Related Documents

- [Game Design Document](../design/GDD.md)
- [Technical Design Document](../technical/TechnicalDesign.md)
- [Art Bible & Style Guide](../art/ArtBible.md)

---

**Document Version:** 1.0
**Project:** MIDNIGHT GRIND
**Status:** Development Ready
