# MIDNIGHT GRIND MVP - Setup Guide

## What Was Built via MCP

### Blueprints Created (/Game/Blueprints/)
**Core:**
- BP_MG_GameMode (GameModeBase)
- BP_MG_PlayerController (PlayerController)
- BP_MG_GameState (GameStateBase)
- BP_MG_HUD (HUD)
- BP_MG_PlayerState (PlayerState)

**Vehicles:**
- BP_MG_Vehicle (WheeledVehiclePawn) - Player car
- BP_MG_AIVehicle (WheeledVehiclePawn) - AI opponents
- BP_PlayerCar, BP_SakuraGTR, BP_TrafficVehicle, BP_PoliceVehicle

**Racing:**
- BP_MG_RaceManager (Actor)
- BP_MG_Checkpoint (Actor)
- BP_MG_StartLine (Actor)
- BP_MG_FinishLine (Actor)
- BP_Checkpoint, BP_RaceManager, etc.

**AI:**
- BP_MG_RaceAI (AIController)

### Level Layout
- Oval race track (~5km)
- 20 road sections
- 8 checkpoints around circuit
- 40+ barriers/walls
- 16 downtown buildings
- 44+ street/neon lights
- PlayerStart at starting line
- SkyLight + DirectionalLight

---

## To Make It Playable

### 1. Set Up Game Mode
1. Edit → Project Settings → Maps & Modes
2. Default GameMode = `BP_MG_GameMode`
3. Default Pawn Class = `BP_MG_Vehicle`

### 2. Configure BP_MG_Vehicle
1. Open BP_MG_Vehicle in Content Browser
2. Add Components:
   - **Skeletal Mesh Component** (for car body mesh)
   - **Spring Arm** + **Camera** (for third-person view)
   - **Chaos Vehicle Movement Component** (physics)
3. In Class Defaults, set:
   - Auto Possess Player = Player 0

### 3. Import a Car Mesh
Option A: Use Engine Content
- Enable "Show Engine Content" in Content Browser
- Search for "Vehicle" or "Car"
- Assign to your Skeletal Mesh component

Option B: Import from Marketplace/Fab
- Download a free vehicle from Fab.com
- Import and assign

### 4. Set Up Input
1. Edit → Project Settings → Input
2. Add Axis Mappings:
   - Throttle: W (+1.0), S (-1.0)
   - Steering: A (-1.0), D (+1.0)
   - Brake: Space
   - Handbrake: Left Shift

### 5. Play!
- Press Play in Editor
- Drive around the track!

---

## Next Steps for Full Game

1. **Vehicle Physics Tuning** - Adjust Chaos Vehicle settings
2. **Checkpoint System** - Wire up overlap events to track laps
3. **Race Timer** - Add UI for lap times
4. **AI Opponents** - Configure BP_MG_RaceAI with behavior trees
5. **Night Lighting** - Reduce DirectionalLight, increase Point Lights
6. **Materials** - Add road textures, building materials
7. **Audio** - Engine sounds, music, ambient city noise

---

Built with ❤️ by Clawd via Unreal MCP
