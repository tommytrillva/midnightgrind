# MIDNIGHT GRIND

**Arcade Racing / Car Culture Simulation**

> Build your car from the ground up, prove yourself on the streets, and risk everything in pink slip races where losing means losing your ride forever.

## Overview

MIDNIGHT GRIND is a nostalgic street racing experience that authentically recreates the PS1/PS2 era aesthetic while delivering deep customization, high-stakes racing, and persistent car culture that modern players crave.

| Attribute | Value |
|-----------|-------|
| **Engine** | Unreal Engine 5 |
| **Platforms** | PC (Primary), Console (Secondary) |
| **Genre** | Arcade Racing / Car Culture Simulation |
| **Target Rating** | T for Teen |

## Design Pillars

1. **Authentic Ownership** - Every car represents real investment. Earn, build, and risk losing them.
2. **Deep Mechanical Truth** - 200+ components per vehicle with real interactions.
3. **Meaningful Stakes** - Pink slips are permanent. No retry spam.
4. **Living Car Culture** - Meet spots, crews, player-driven economy.
5. **Unified Challenge** - Same physics for everyone. Skill determines success.

## Documentation

### Design Documents
- [Game Design Document (GDD)](docs/design/GDD.md) - Complete game vision, pillars, and systems
- [Vehicle Systems Specification](docs/design/VehicleSystems.md) - Deep dive into vehicle architecture and parts
- [Multiplayer Systems Design](docs/design/MultiplayerSystems.md) - Social, economy, and online systems

### Technical Documents
- [Technical Design Document](docs/technical/TechnicalDesign.md) - Architecture, implementation, and pipeline
- [Visual Style Guide](docs/technical/VisualStyleGuide.md) - PS1/PS2 retro aesthetic guidelines

### Art & Audio
- [Art Bible](docs/art/ArtBible.md) - Visual direction and asset guidelines
- [Audio Design Document](docs/audio/AudioDesign.md) - Sound design and music systems

## Key Features

### Vehicles & Customization
- Real components with real effects
- 200+ individual parts per vehicle
- Dyno testing with power curves
- Tuning system that rewards knowledge
- Part wear and maintenance

### Racing
- Street Sprint
- Circuit Racing
- Drag Racing
- Highway Battles (Wangan style)
- Touge/Canyon Duels
- Drift Events
- **Pink Slip Races** - Lose the race, lose your car

### World
- Open world city with multiple districts
- Day/night cycle (night-focused)
- Dynamic weather
- Police/heat system
- Meet spots for social gathering

### Multiplayer
- Free roam (50-100 players)
- Social hub (200 players)
- Player-driven economy
- Crew system
- Ranked racing
- Tournaments

## Development Status

**Current Stage:** MVP Complete (Stage 59)

### MVP Implementation Status

| System | Status |
|--------|--------|
| Core Game Loop | Complete |
| Vehicle Physics | Complete |
| Customization System | Complete |
| Race Types (Sprint, Drag, Time Attack, Pink Slip) | Complete |
| Cash Economy | Complete |
| REP System | Complete |
| Save/Load System | Complete |
| Retro Visual Pipeline | Complete |
| AI Opponents | Complete |
| Police/Heat System | Complete |
| Traffic System | Complete |
| HUD & UI | Complete |
| Telemetry | Complete |
| Dev Commands | Complete |

### Subsystem Count: 183

The prototype includes comprehensive implementations across:
- Core gameplay (AI, Input, Physics, Vehicle)
- Racing systems (Race Director, Checkpoints, Scoring)
- Progression (REP, Licenses, Achievements)
- Economy (Cash, Market, Trading)
- Social (Crew, Meet Spots, Chat)
- Live Service (Seasons, Events, Challenges)
- Technical (Streaming, Performance, Analytics)

### Pre-Production Checklist
- [x] Game Design Document
- [x] Vehicle Systems Specification
- [x] Multiplayer Systems Design
- [x] Technical Design Document
- [x] Art Bible / Style Guide
- [x] Audio Design Document
- [x] Vehicle Physics Prototype
- [x] Visual Style Prototype
- [x] Project Setup

## Project Structure

```
midnightgrind/
├── docs/
│   ├── design/
│   │   ├── GDD.md
│   │   ├── VehicleSystems.md
│   │   └── MultiplayerSystems.md
│   ├── technical/
│   │   ├── TechnicalDesign.md
│   │   └── VisualStyleGuide.md
│   ├── art/
│   │   └── ArtBible.md
│   └── audio/
│       └── AudioDesign.md
├── prototype/
│   ├── Source/MidnightGrind/  (292 .cpp files, 293 .h files)
│   ├── Shaders/               (Custom retro post-process)
│   ├── Config/                (Engine/Input/Steam config)
│   └── EditorScripts/         (Setup automation)
├── config/
└── README.md
```

## Target Audience

**Primary:**
- Ages 18-35
- Players of NFS Underground, Midnight Club, Tokyo Xtreme Racer
- Car enthusiasts (JDM, tuner culture)
- Nostalgia-driven gamers

**Secondary:**
- Younger racing fans discovering the genre
- Simulation racing fans seeking arcade alternative
- Content creators

## Building the Project

1. Install Unreal Engine 5
2. Open `prototype/MidnightGrind.uproject`
3. Run Editor Scripts in sequence (00-05) for initial setup
4. Build and run

## License

Proprietary - All Rights Reserved

---

*"It's 2 AM. You're rolling through the city in a car you built with your own hands. Tonight, you're taking their keys."*
