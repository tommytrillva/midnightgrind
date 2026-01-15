# MIDNIGHT GRIND
## Technical Design Document
### Version 1.0

---

## TABLE OF CONTENTS

1. [Technical Overview](#1-technical-overview)
2. [Engine & Platform](#2-engine--platform)
3. [Core Systems Architecture](#3-core-systems-architecture)
4. [Vehicle System Implementation](#4-vehicle-system-implementation)
5. [Rendering & Visual Pipeline](#5-rendering--visual-pipeline)
6. [Physics System](#6-physics-system)
7. [Audio System](#7-audio-system)
8. [Network Architecture](#8-network-architecture)
9. [Data Management](#9-data-management)
10. [Performance Targets](#10-performance-targets)
11. [Development Pipeline](#11-development-pipeline)

---

## 1. TECHNICAL OVERVIEW

### 1.1 Technology Stack

| Component | Technology |
|-----------|------------|
| **Game Engine** | Unreal Engine 5.3+ |
| **Primary Language** | C++ / Blueprints |
| **Physics** | Chaos Vehicle Physics (customized) |
| **Networking** | UE5 Replication + Dedicated Servers |
| **Audio** | MetaSounds |
| **UI Framework** | UMG (Unreal Motion Graphics) |
| **Build System** | UnrealBuildTool |
| **Version Control** | Git with Git LFS |

### 1.2 Target Platforms

| Platform | Priority | Notes |
|----------|----------|-------|
| PC (Windows) | Primary | Steam, Epic Games Store |
| PC (Linux) | Secondary | Proton compatibility |
| PlayStation 5 | Future | Console release |
| Xbox Series X/S | Future | Console release |

### 1.3 System Requirements

**Minimum (Target 60 FPS at 1080p Low):**
| Component | Specification |
|-----------|---------------|
| CPU | Intel i5-6600K / AMD Ryzen 5 1500X |
| GPU | NVIDIA GTX 1060 / AMD RX 580 |
| RAM | 8 GB |
| VRAM | 4 GB |
| Storage | 30 GB SSD |
| OS | Windows 10 64-bit |

**Recommended (Target 60 FPS at 1440p High):**
| Component | Specification |
|-----------|---------------|
| CPU | Intel i7-9700K / AMD Ryzen 7 3700X |
| GPU | NVIDIA RTX 2070 / AMD RX 6700 XT |
| RAM | 16 GB |
| VRAM | 8 GB |
| Storage | 30 GB NVMe SSD |
| OS | Windows 10/11 64-bit |

**Ultra (Target 60+ FPS at 4K):**
| Component | Specification |
|-----------|---------------|
| CPU | Intel i9-12900K / AMD Ryzen 9 5900X |
| GPU | NVIDIA RTX 3080 / AMD RX 6800 XT |
| RAM | 32 GB |
| VRAM | 10 GB+ |
| Storage | 50 GB NVMe SSD |

---

## 2. ENGINE & PLATFORM

### 2.1 Unreal Engine 5 Justification

**Selection Rationale:**
- Developer experience with UE5
- Robust vehicle physics (Chaos)
- Strong networking infrastructure
- Cross-platform deployment
- Active community and support
- Modding potential

### 2.2 Key UE5 Systems Utilized

```
UE5 SYSTEMS
├── Chaos Vehicle Physics
│   ├── Customized for arcade-sim balance
│   ├── Tire model modifications
│   ├── Suspension simulation
│   └── Collision response tuning
│
├── World Partition
│   ├── Open world streaming
│   ├── Level instance management
│   ├── Distance-based loading
│   └── Memory optimization
│
├── Niagara
│   ├── Exhaust particles
│   ├── Tire smoke
│   ├── Weather effects
│   ├── Nitrous flames
│   └── Damage sparks
│
├── Lumen (Modified)
│   ├── Global illumination
│   ├── Real-time reflections
│   └── Night lighting
│
├── MetaSounds
│   ├── Procedural engine audio
│   ├── Dynamic exhaust
│   ├── Environmental audio
│   └── Music system
│
├── Enhanced Input System
│   ├── Gamepad support
│   ├── Wheel support
│   ├── Keyboard/mouse
│   └── Input remapping
│
├── Gameplay Ability System (GAS)
│   ├── Progression system
│   ├── Buff/debuff management
│   ├── Nitrous activation
│   └── Special abilities
│
└── Dedicated Server Support
    ├── Server builds
    ├── Replication
    └── Authority management
```

### 2.3 Engine Modifications

**Chaos Vehicle Customizations:**
- Modified tire friction model for arcade feel
- Adjusted weight transfer calculations
- Custom drift physics implementation
- Handbrake behavior tuning
- Collision response dampening

**Rendering Pipeline:**
- Custom post-process chain for retro aesthetic
- Resolution scaling with point filtering
- Color banding/dithering shaders
- CRT/VHS effect passes (optional)

---

## 3. CORE SYSTEMS ARCHITECTURE

### 3.1 High-Level Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      GAME INSTANCE                           │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌────────────────┐     ┌────────────────┐                  │
│  │  GAME MODE     │     │    WORLD       │                  │
│  │  - Race Mode   │     │  - Streaming   │                  │
│  │  - Free Roam   │     │  - Traffic AI  │                  │
│  │  - Social Hub  │     │  - Police AI   │                  │
│  └───────┬────────┘     └───────┬────────┘                  │
│          │                      │                            │
│          ▼                      ▼                            │
│  ┌────────────────────────────────────────┐                 │
│  │            PLAYER CONTROLLER            │                 │
│  └───────────────────┬────────────────────┘                 │
│                      │                                       │
│          ┌───────────┼───────────┐                          │
│          ▼           ▼           ▼                          │
│  ┌────────────┐ ┌─────────┐ ┌──────────┐                   │
│  │  VEHICLE   │ │CHARACTER│ │PROGRESSION│                   │
│  │  SYSTEM    │ │ SYSTEM  │ │  SYSTEM   │                   │
│  └─────┬──────┘ └────┬────┘ └─────┬─────┘                   │
│        │             │            │                          │
│        ▼             │            ▼                          │
│  ┌────────────┐      │     ┌──────────┐                     │
│  │CUSTOMIZATION│     │     │ SAVE/    │                     │
│  │  SYSTEM    │      │     │ ECONOMY  │                     │
│  └────────────┘      │     └──────────┘                     │
│                      │                                       │
└──────────────────────┼───────────────────────────────────────┘
                       │
                       ▼
             ┌─────────────────┐
             │  NETWORK LAYER  │
             │  - Replication  │
             │  - Server Auth  │
             │  - Anti-Cheat   │
             └─────────────────┘
```

### 3.2 Subsystem Breakdown

```
GAME SUBSYSTEMS
├── Core
│   ├── GameInstance (persistent data, save management)
│   ├── GameMode (rules, spawning, win conditions)
│   ├── GameState (shared game state)
│   └── PlayerState (per-player state)
│
├── Vehicle
│   ├── VehiclePawn (physical representation)
│   ├── VehicleMovementComponent (physics)
│   ├── VehicleDataComponent (configuration)
│   └── VehicleAudioComponent (sound)
│
├── Customization
│   ├── PartDatabase (all available parts)
│   ├── VehicleConfiguration (installed parts)
│   ├── StatCalculator (performance calculation)
│   └── VisualCustomization (paint, livery, etc.)
│
├── Racing
│   ├── RaceManager (race flow control)
│   ├── CheckpointSystem (position tracking)
│   ├── RaceAI (opponent behavior)
│   └── ResultCalculator (payouts, REP)
│
├── World
│   ├── TrafficManager (AI traffic)
│   ├── PoliceManager (heat system, pursuits)
│   ├── WeatherManager (conditions)
│   └── TimeManager (day/night cycle)
│
├── Economy
│   ├── WalletManager (cash management)
│   ├── MarketManager (player trading)
│   ├── TransactionLog (audit trail)
│   └── PriceCalculator (dynamic pricing)
│
├── Progression
│   ├── REPManager (reputation system)
│   ├── UnlockManager (content gating)
│   ├── AchievementManager (achievements)
│   └── StatTracker (player statistics)
│
├── Social
│   ├── CrewManager (crew system)
│   ├── FriendManager (friend list)
│   ├── ChatManager (communication)
│   └── VoiceManager (voice chat)
│
└── UI
    ├── HUDManager (in-race UI)
    ├── GarageUI (customization interface)
    ├── MenuManager (menus, navigation)
    └── MapManager (world map, navigation)
```

### 3.3 Game Modes

```cpp
// Game Mode Hierarchy
class AMGGameModeBase : public AGameModeBase
{
    // Base functionality for all modes
};

class AMGRaceGameMode : public AMGGameModeBase
{
    // Race-specific logic
    // Checkpoint management
    // Position tracking
    // Win condition evaluation
};

class AMGFreeRoamGameMode : public AMGGameModeBase
{
    // Open world logic
    // Player discovery
    // Dynamic events
    // Police/traffic management
};

class AMGHubGameMode : public AMGGameModeBase
{
    // Social hub (Meet Spot)
    // High player count
    // Showcase features
    // Trade terminals
};
```

---

## 4. VEHICLE SYSTEM IMPLEMENTATION

### 4.1 Vehicle Data Structure

```cpp
// Core vehicle data structure
USTRUCT(BlueprintType)
struct FVehicleData
{
    GENERATED_BODY()

    // Identification
    UPROPERTY()
    FGuid VehicleID;

    UPROPERTY()
    FString VIN;  // Unique per instance

    UPROPERTY()
    FName ModelID;  // Reference to base model

    // Ownership
    UPROPERTY()
    FGuid OwnerID;

    UPROPERTY()
    TArray<FOwnershipRecord> OwnerHistory;

    // Configuration (installed parts)
    UPROPERTY()
    FEngineConfiguration Engine;

    UPROPERTY()
    FDrivetrainConfiguration Drivetrain;

    UPROPERTY()
    FSuspensionConfiguration Suspension;

    UPROPERTY()
    FBrakeConfiguration Brakes;

    UPROPERTY()
    FWheelTireConfiguration WheelsTires;

    UPROPERTY()
    FBodyConfiguration Body;

    UPROPERTY()
    FInteriorConfiguration Interior;

    UPROPERTY()
    FAeroConfiguration Aero;

    // Calculated Stats
    UPROPERTY()
    FVehicleStats CurrentStats;

    UPROPERTY()
    float PerformanceIndex;

    UPROPERTY()
    EPerformanceClass Class;

    // Condition
    UPROPERTY()
    TMap<FName, float> PartConditions;

    UPROPERTY()
    int32 TotalMileage;

    UPROPERTY()
    int32 AccidentCount;

    // Visual
    UPROPERTY()
    FPaintConfiguration Paint;

    UPROPERTY()
    FVinylConfiguration Vinyls;

    UPROPERTY()
    FLightingConfiguration Lighting;

    // History
    UPROPERTY()
    FRaceHistory RaceStats;

    UPROPERTY()
    TArray<FNotableEvent> NotableEvents;

    UPROPERTY()
    float EstimatedValue;
};
```

### 4.2 Engine Configuration

```cpp
USTRUCT(BlueprintType)
struct FEngineConfiguration
{
    GENERATED_BODY()

    // Block
    UPROPERTY()
    FName EngineBlockID;

    // Head
    UPROPERTY()
    FName CylinderHeadID;

    // Valvetrain
    UPROPERTY()
    FName CamshaftID;

    // Aspiration
    UPROPERTY()
    FName IntakeManifoldID;

    UPROPERTY()
    FName ThrottleBodyID;

    UPROPERTY()
    FName AirFilterID;

    // Exhaust
    UPROPERTY()
    FName ExhaustManifoldID;

    UPROPERTY()
    FName ExhaustSystemID;

    // Rotating Assembly
    UPROPERTY()
    FName PistonsID;

    UPROPERTY()
    FName ConnectingRodsID;

    UPROPERTY()
    FName CrankshaftID;

    UPROPERTY()
    FName FlywheelID;

    // Forced Induction (Optional)
    UPROPERTY()
    TOptional<FForcedInductionConfig> ForcedInduction;

    // Fuel System
    UPROPERTY()
    FName FuelInjectorsID;

    UPROPERTY()
    FName FuelPumpID;

    // Ignition
    UPROPERTY()
    FName SparkPlugsID;

    // Management
    UPROPERTY()
    FName ECUID;

    UPROPERTY()
    FECUTuneData TuneData;

    // Nitrous (Optional)
    UPROPERTY()
    TOptional<FNitrousConfig> Nitrous;
};
```

### 4.3 Calculated Vehicle Stats

```cpp
USTRUCT(BlueprintType)
struct FVehicleStats
{
    GENERATED_BODY()

    // Primary Stats
    UPROPERTY(BlueprintReadOnly)
    float Horsepower;

    UPROPERTY(BlueprintReadOnly)
    float Torque;

    UPROPERTY(BlueprintReadOnly)
    float Weight;

    UPROPERTY(BlueprintReadOnly)
    float GripFront;

    UPROPERTY(BlueprintReadOnly)
    float GripRear;

    UPROPERTY(BlueprintReadOnly)
    float Handling;

    UPROPERTY(BlueprintReadOnly)
    float Braking;

    // Derived Stats
    UPROPERTY(BlueprintReadOnly)
    float PowerToWeight;

    UPROPERTY(BlueprintReadOnly)
    float Acceleration0to60;

    UPROPERTY(BlueprintReadOnly)
    float Acceleration0to100;

    UPROPERTY(BlueprintReadOnly)
    float QuarterMileET;

    UPROPERTY(BlueprintReadOnly)
    float QuarterMileTrap;

    UPROPERTY(BlueprintReadOnly)
    float TopSpeed;

    // Engine Stats
    UPROPERTY(BlueprintReadOnly)
    float ThrottleResponse;

    UPROPERTY(BlueprintReadOnly)
    float TurboLag;

    UPROPERTY(BlueprintReadOnly)
    float BoostPSI;

    UPROPERTY(BlueprintReadOnly)
    int32 RevLimit;

    UPROPERTY(BlueprintReadOnly)
    FFloatRange PowerBandRPM;

    // Drivetrain Stats
    UPROPERTY(BlueprintReadOnly)
    float ShiftSpeed;

    // Reliability
    UPROPERTY(BlueprintReadOnly)
    float Durability;

    UPROPERTY(BlueprintReadOnly)
    float CoolingEfficiency;
};
```

### 4.4 Performance Calculation System

```cpp
// Stat Calculator Interface
class IStatCalculator
{
public:
    virtual FVehicleStats CalculateStats(const FVehicleData& VehicleData) = 0;
    virtual float CalculatePerformanceIndex(const FVehicleStats& Stats) = 0;
    virtual FPowerCurve CalculatePowerCurve(const FEngineConfiguration& Engine) = 0;
};

// Power Calculation Formula (Pseudocode)
float CalculateHorsepower(const FEngineConfiguration& Config)
{
    float BasePower = GetBaseEnginePower(Config.EngineBlockID);

    // Aspiration multipliers
    float AspirationMult = 1.0f;
    AspirationMult *= GetPartMultiplier(Config.AirFilterID);
    AspirationMult *= GetPartMultiplier(Config.IntakeManifoldID);
    AspirationMult *= GetPartMultiplier(Config.ThrottleBodyID);
    AspirationMult *= GetPartMultiplier(Config.ExhaustManifoldID);
    AspirationMult *= GetPartMultiplier(Config.ExhaustSystemID);

    // Internal multipliers
    float InternalMult = 1.0f;
    InternalMult *= GetPartMultiplier(Config.CylinderHeadID);
    InternalMult *= GetPartMultiplier(Config.CamshaftID);

    // Forced induction
    float FIMult = 1.0f;
    if (Config.ForcedInduction.IsSet())
    {
        FIMult = CalculateForcedInductionMultiplier(Config.ForcedInduction.GetValue());
    }

    // ECU efficiency
    float ECUEfficiency = GetTuneEfficiency(Config.TuneData);

    // Fuel system cap
    float FuelCap = CalculateFuelSystemCap(Config);

    // Condition modifier
    float ConditionMult = GetAveragePartCondition(Config);

    return BasePower * AspirationMult * InternalMult * FIMult *
           ECUEfficiency * FuelCap * ConditionMult;
}
```

### 4.5 Part Database System

```cpp
// Part data asset
UCLASS(BlueprintType)
class UPartDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName PartID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EPartCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EPartTier Tier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 BaseCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DurabilityRating;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, float> StatModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> CompatibleVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> RequiredParts;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh> VisualMesh;
};

// Part tier enum
UENUM(BlueprintType)
enum class EPartTier : uint8
{
    Stock,
    Street,
    Sport,
    Race,
    Pro,
    Legendary
};
```

---

## 5. RENDERING & VISUAL PIPELINE

### 5.1 Retro Visual Pipeline

The game uses a custom post-process chain to achieve the PS1/PS2 era aesthetic.

```
POST-PROCESS STACK (Order of Execution)
├── 1. Resolution Scaling
│   ├── Render at lower internal resolution
│   ├── Configurable scale (0.5x - 1.0x native)
│   └── Point filtering upscale (pixelated)
│
├── 2. Color Processing
│   ├── Color banding (reduce to 15-bit color)
│   ├── Optional dithering (PS1 style)
│   ├── Saturation boost (1.1x - 1.3x)
│   └── Contrast adjustment
│
├── 3. CRT/VHS Effects (Toggle)
│   ├── Scanlines (horizontal)
│   ├── Screen curvature (barrel distortion)
│   ├── VHS tracking noise (animated)
│   ├── Chromatic aberration (subtle)
│   └── Era-appropriate bloom
│
├── 4. Lighting Post-Process
│   ├── Neon glow enhancement
│   ├── Volumetric light shafts
│   └── Night atmosphere
│
└── 5. Final Output
    ├── Vignette (subtle)
    ├── Film grain (optional)
    └── Output transform
```

### 5.2 Material System

```
MATERIAL APPROACH
├── Base Materials
│   ├── Vehicle Paint (PBR with retro adjustments)
│   ├── Chrome/Metal (environment mapped)
│   ├── Glass (transparent with reflections)
│   ├── Rubber/Tire (dark, low reflection)
│   └── Interior (varied per surface)
│
├── Environment Materials
│   ├── Asphalt (wet/dry variants)
│   ├── Concrete
│   ├── Building facades
│   ├── Neon signs (emissive)
│   └── Foliage (simple)
│
├── Retro Modifications
│   ├── Reduced texture filtering
│   ├── Lower mip bias (sharper at distance)
│   ├── Simplified lighting response
│   └── Emissive enhancement for neon
│
└── Special Effects
    ├── Underglow (emissive planes)
    ├── Headlight cones
    ├── Brake light glow
    └── Nitrous flame
```

### 5.3 Lighting Strategy

```
LIGHTING APPROACH
├── Time of Day
│   ├── Sun/Moon directional light
│   ├── Sky light (ambient)
│   ├── Atmospheric fog
│   └── Exposure adjustment per time
│
├── Night Lighting (Primary)
│   ├── Street lights (point lights, instanced)
│   ├── Neon signs (rect lights + emissive)
│   ├── Vehicle headlights (spot lights)
│   ├── Building windows (emissive)
│   └── Moon illumination (weak directional)
│
├── Optimization
│   ├── Light culling by distance
│   ├── Shadow cascade optimization
│   ├── Light importance volumes
│   └── Pre-baked where possible
│
└── Reflections
    ├── Screen-space reflections (wet roads)
    ├── Reflection captures (strategic)
    └── Vehicle-specific captures
```

### 5.4 Particle Systems (Niagara)

```
PARTICLE EFFECTS
├── Vehicle Effects
│   ├── Exhaust Smoke
│   │   ├── Idle: Light wisps
│   │   ├── Acceleration: Dense burst
│   │   ├── Backfire: Flash + smoke
│   │   └── Turbo flutter: Characteristic puff
│   │
│   ├── Tire Smoke
│   │   ├── Burnout: Dense white
│   │   ├── Drift: Trail based on angle
│   │   ├── Launch: Wheel spin smoke
│   │   └── Lockup: Skid smoke
│   │
│   ├── Nitrous
│   │   ├── Activation flash
│   │   ├── Continuous flame
│   │   └── Purge (cosmetic)
│   │
│   └── Damage
│       ├── Sparks (collision)
│       ├── Fluid leaks
│       └── Smoke (overheating)
│
├── Weather Effects
│   ├── Rain drops (GPU particles)
│   ├── Rain splash (on surfaces)
│   ├── Fog (volumetric)
│   └── Wet road spray
│
└── Environment
    ├── Traffic exhaust
    ├── Steam vents
    ├── Dust (construction areas)
    └── Leaves (canyon areas)
```

---

## 6. PHYSICS SYSTEM

### 6.1 Chaos Vehicle Configuration

```cpp
// Custom vehicle movement component
UCLASS()
class UMGVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
    GENERATED_BODY()

public:
    // Override tire friction for arcade feel
    virtual float GetTireFriction(int32 WheelIndex) const override;

    // Custom weight transfer calculation
    virtual FVector CalculateWeightTransfer() const;

    // Drift physics
    UPROPERTY(EditAnywhere, Category = "Drift")
    float DriftAngleThreshold;

    UPROPERTY(EditAnywhere, Category = "Drift")
    float DriftFrictionMultiplier;

    UPROPERTY(EditAnywhere, Category = "Drift")
    float DriftCounterSteerAssist;

    // Handbrake override
    UPROPERTY(EditAnywhere, Category = "Handbrake")
    float HandbrakeFrictionMultiplier;

    UPROPERTY(EditAnywhere, Category = "Handbrake")
    float HandbrakeRearBias;
};
```

### 6.2 Tire Model

```
TIRE FRICTION MODEL
├── Base Grip (from tire compound)
│   ├── Economy: 0.70
│   ├── Sport: 0.85
│   ├── Performance: 0.95
│   ├── Semi-Slick: 1.05
│   └── Slick: 1.15
│
├── Slip Curve
│   ├── Linear region (low slip)
│   ├── Peak grip (optimal slip angle)
│   ├── Transition region
│   └── Sliding region (reduced grip)
│
├── Modifiers
│   ├── Temperature (cold = less grip)
│   ├── Wear (worn = less grip)
│   ├── Surface (wet = less grip)
│   └── Load (more weight = more grip, diminishing)
│
└── Special Behaviors
    ├── Drift mode (modified slip curve)
    ├── Burnout (locked wheels, spinning)
    └── Hydroplaning (high speed + wet)
```

### 6.3 Weight Transfer

```
WEIGHT TRANSFER SYSTEM
├── Longitudinal
│   ├── Acceleration: Weight to rear
│   ├── Braking: Weight to front
│   └── Magnitude based on G-force
│
├── Lateral
│   ├── Cornering: Weight to outside
│   ├── Affects grip per wheel
│   └── Roll bar adjustment
│
├── Factors
│   ├── Wheelbase
│   ├── Track width
│   ├── CoG height
│   ├── Spring rates
│   └── Anti-roll bar stiffness
│
└── Gameplay Effect
    ├── Determines available grip per tire
    ├── Affects handling balance
    └── Influences drift initiation
```

### 6.4 Collision System

```
COLLISION HANDLING
├── Vehicle-Vehicle
│   ├── Impulse-based response
│   ├── Damage calculation
│   ├── REP/heat consequences
│   └── Anti-ramming measures (online)
│
├── Vehicle-Environment
│   ├── Wall scrapes (cosmetic damage)
│   ├── Hard impacts (mechanical damage)
│   ├── Barriers (bounce response)
│   └── Destructibles (signs, cones)
│
├── Damage Model
│   ├── Visual deformation (mesh morph)
│   ├── Mechanical degradation
│   ├── Part condition reduction
│   └── Critical failure (extreme)
│
└── Physics Response
    ├── Tuned for arcade feel
    ├── Reduced flip tendency
    ├── Quick recovery
    └── Forgiving wall interactions
```

---

## 7. AUDIO SYSTEM

### 7.1 Engine Audio (MetaSounds)

```
ENGINE AUDIO SYSTEM
├── Components
│   ├── Base engine loop (RPM-driven)
│   ├── Load layer (throttle position)
│   ├── Exhaust character (muffler type)
│   ├── Intake sound (throttle body)
│   └── Forced induction (turbo/supercharger)
│
├── Parameters
│   ├── RPM (0 - redline)
│   ├── Throttle (0.0 - 1.0)
│   ├── Load (engine load %)
│   ├── Gear (current gear)
│   └── Speed (vehicle speed)
│
├── Engine Types
│   ├── I4 (inline 4)
│   ├── I6 (inline 6)
│   ├── V6
│   ├── V8
│   ├── Rotary
│   └── Each with unique character
│
├── Exhaust Variants
│   ├── Stock (muffled)
│   ├── Sport (enhanced)
│   ├── Race (loud)
│   ├── Straight pipe (raw)
│   └── Each affects sound character
│
└── Special Sounds
    ├── Backfire (decel)
    ├── Turbo spool/flutter
    ├── Supercharger whine
    ├── Nitrous activation
    └── Rev limiter
```

### 7.2 Vehicle Audio

```
NON-ENGINE VEHICLE AUDIO
├── Tire Sounds
│   ├── Rolling (surface-dependent)
│   ├── Skid/slide
│   ├── Burnout
│   └── Hydroplaning
│
├── Transmission
│   ├── Shift clunk
│   ├── Gear whine
│   └── Sequential shift
│
├── Brakes
│   ├── Light braking
│   ├── Hard braking
│   ├── ABS cycling
│   └── Brake squeal
│
├── Suspension
│   ├── Bump response
│   ├── Bottoming out
│   └── Body roll creak
│
└── Wind/Aero
    ├── Speed-dependent wind
    ├── Open window
    └── Spoiler turbulence
```

### 7.3 Environmental Audio

```
ENVIRONMENTAL AUDIO
├── Ambient Layers
│   ├── City (traffic, crowds, distant)
│   ├── Industrial (machinery, sparse)
│   ├── Canyon (nature, wind)
│   └── Highway (continuous traffic)
│
├── Dynamic Elements
│   ├── Other vehicles (proximity-based)
│   ├── Police sirens
│   ├── Crowd reactions (meets)
│   └── Weather sounds
│
├── Music System
│   ├── Radio stations (genre-based)
│   ├── Dynamic mixing
│   ├── Race intensity adjustment
│   └── User music support (TBD)
│
└── UI Audio
    ├── Menu navigation
    ├── Part installation
    ├── Transaction sounds
    └── Notification sounds
```

---

## 8. NETWORK ARCHITECTURE

### 8.1 Server Infrastructure

```
SERVER ARCHITECTURE
├── Game Servers (Dedicated)
│   ├── Race Servers
│   │   ├── Spawned per race
│   │   ├── Authoritative physics
│   │   ├── 2-12 players per instance
│   │   └── Short-lived (race duration)
│   │
│   ├── Free Roam Servers
│   │   ├── Regional deployment
│   │   ├── 50-100 players
│   │   ├── World state sync
│   │   └── Long-running
│   │
│   └── Hub Servers
│       ├── Meet Spot instances
│       ├── 200 players
│       ├── Social features
│       └── Trade terminals
│
├── Backend Services
│   ├── Account Service
│   │   ├── Authentication
│   │   ├── Profile management
│   │   └── Entitlements
│   │
│   ├── Economy Service
│   │   ├── Transaction processing
│   │   ├── Inventory management
│   │   ├── Market operations
│   │   └── Audit logging
│   │
│   ├── Matchmaking Service
│   │   ├── Queue management
│   │   ├── Skill matching
│   │   ├── Server allocation
│   │   └── Region routing
│   │
│   ├── Social Service
│   │   ├── Friends/blocks
│   │   ├── Crews
│   │   ├── Messaging
│   │   └── Presence
│   │
│   └── Analytics Service
│       ├── Telemetry collection
│       ├── Anti-cheat signals
│       └── Economic monitoring
│
└── Data Storage
    ├── Player Database (primary)
    ├── Vehicle Database
    ├── Market Database
    ├── Replay Storage (object storage)
    └── Audit Logs (append-only)
```

### 8.2 Replication Strategy

```cpp
// Replicated vehicle state
USTRUCT()
struct FReplicatedVehicleState
{
    GENERATED_BODY()

    // Position/Rotation (compressed)
    UPROPERTY()
    FVector_NetQuantize Location;

    UPROPERTY()
    FRotator Rotation;

    // Velocity
    UPROPERTY()
    FVector_NetQuantize LinearVelocity;

    UPROPERTY()
    FVector_NetQuantize AngularVelocity;

    // Input state
    UPROPERTY()
    float Throttle;

    UPROPERTY()
    float Brake;

    UPROPERTY()
    float Steering;

    UPROPERTY()
    bool bHandbrake;

    // Visual state
    UPROPERTY()
    int32 CurrentGear;

    UPROPERTY()
    float EngineRPM;

    UPROPERTY()
    bool bNitrousActive;

    // Timestamp for interpolation
    UPROPERTY()
    float ServerTimestamp;
};
```

### 8.3 Server Authority

```
SERVER-AUTHORITATIVE SYSTEMS
├── Physics Simulation
│   ├── Client sends input only
│   ├── Server simulates physics
│   ├── Server sends state updates
│   └── Client reconciles/interpolates
│
├── Race Logic
│   ├── Checkpoint validation
│   ├── Position calculation
│   ├── Time tracking
│   └── Result determination
│
├── Economy
│   ├── All transactions server-side
│   ├── No client-side currency
│   ├── Atomic operations
│   └── Rollback capability
│
├── Progression
│   ├── REP calculation server-side
│   ├── Unlock validation
│   └── Achievement tracking
│
└── Anti-Cheat
    ├── Input validation
    ├── State validation
    ├── Statistical analysis
    └── Behavioral monitoring
```

### 8.4 Latency Compensation

```
LATENCY HANDLING
├── Client-Side Prediction
│   ├── Input prediction
│   ├── Movement extrapolation
│   └── Visual smoothing
│
├── Server Reconciliation
│   ├── State correction
│   ├── Rewind/replay
│   └── Authoritative override
│
├── Interpolation
│   ├── Other player positions
│   ├── Smooth visual updates
│   └── Buffer size tuning
│
└── Lag Compensation
    ├── Compensation window (150ms max)
    ├── Collision detection
    └── Position rewinding
```

---

## 9. DATA MANAGEMENT

### 9.1 Save System

```
SAVE DATA STRUCTURE
├── Player Profile
│   ├── Account ID
│   ├── Display name
│   ├── Settings/preferences
│   └── Statistics
│
├── Progression
│   ├── REP total
│   ├── Cash balance
│   ├── Achievements
│   └── Unlock states
│
├── Garage
│   ├── Owned vehicles (array)
│   ├── Garage configuration
│   └── Active vehicle ID
│
├── Vehicles (per vehicle)
│   ├── Full FVehicleData struct
│   ├── Serialized configuration
│   └── History records
│
└── Social
    ├── Crew membership
    ├── Friends list
    └── Recent players
```

### 9.2 Cloud Save

```
CLOUD SAVE STRATEGY
├── Automatic Sync
│   ├── On significant change
│   ├── Periodic backup (5 min)
│   └── On session end
│
├── Conflict Resolution
│   ├── Server timestamp priority
│   ├── Merge where possible
│   └── User prompt for conflicts
│
├── Data Validation
│   ├── Schema validation
│   ├── Value range checks
│   └── Integrity verification
│
└── Rollback
    ├── Last N saves stored
    ├── Support-initiated rollback
    └── Corruption recovery
```

### 9.3 Asset Management

```
ASSET ORGANIZATION
├── Vehicles
│   ├── /Content/Vehicles/{VehicleID}/
│   │   ├── Mesh/
│   │   ├── Materials/
│   │   ├── Textures/
│   │   ├── Animations/
│   │   └── Audio/
│   └── Data assets in Data Tables
│
├── Parts
│   ├── /Content/Parts/{Category}/
│   │   ├── Meshes/
│   │   └── Data/
│   └── Part database (Data Table)
│
├── Environment
│   ├── /Content/Environment/{District}/
│   │   ├── Props/
│   │   ├── Buildings/
│   │   ├── Roads/
│   │   └── Lighting/
│   └── World partition maps
│
├── UI
│   ├── /Content/UI/
│   │   ├── Widgets/
│   │   ├── Icons/
│   │   └── Fonts/
│   └── Style guide assets
│
└── Audio
    ├── /Content/Audio/
    │   ├── Engines/
    │   ├── Environment/
    │   ├── UI/
    │   └── Music/
    └── MetaSound graphs
```

---

## 10. PERFORMANCE TARGETS

### 10.1 Frame Rate Targets

| Scenario | Minimum Spec | Recommended | Ultra |
|----------|--------------|-------------|-------|
| Garage (static) | 60 FPS | 60 FPS | 120 FPS |
| Free Roam (solo) | 60 FPS | 60 FPS | 120 FPS |
| Race (8 players) | 60 FPS | 60 FPS | 60 FPS |
| Hub (200 players) | 30 FPS | 60 FPS | 60 FPS |

### 10.2 Loading Times

| Operation | Target |
|-----------|--------|
| Initial boot | < 30 seconds |
| Load save | < 5 seconds |
| Enter race | < 10 seconds |
| Stream district | Seamless |
| Fast travel | < 5 seconds |

### 10.3 Memory Budget

| Category | Budget |
|----------|--------|
| Total RAM | 6 GB (min), 10 GB (recommended) |
| VRAM | 3 GB (min), 6 GB (recommended) |
| Streaming pool | 2 GB |
| Audio | 512 MB |
| UI | 256 MB |

### 10.4 Network Performance

| Metric | Target |
|--------|--------|
| Update rate | 30 Hz (client), 60 Hz (server) |
| Bandwidth (race) | < 50 KB/s per player |
| Bandwidth (free roam) | < 100 KB/s |
| Latency tolerance | < 150 ms playable |

---

## 11. DEVELOPMENT PIPELINE

### 11.1 Build Pipeline

```
BUILD PROCESS
├── Source Control
│   ├── Git with Git LFS
│   ├── Branch strategy: GitFlow
│   └── PR-based merges
│
├── Continuous Integration
│   ├── Trigger: PR merge to develop
│   ├── Build: Full project compile
│   ├── Test: Automated tests
│   └── Package: Development build
│
├── Build Types
│   ├── Development (debugging enabled)
│   ├── Testing (optimized, debug symbols)
│   ├── Shipping (final optimization)
│   └── Server (dedicated server build)
│
└── Artifacts
    ├── Client builds
    ├── Server builds
    ├── Symbol files
    └── Build metadata
```

### 11.2 Version Control Strategy

```
BRANCH STRATEGY
├── main
│   └── Production releases only
│
├── develop
│   └── Integration branch
│
├── feature/*
│   └── Feature development
│
├── bugfix/*
│   └── Bug fixes
│
├── release/*
│   └── Release preparation
│
└── hotfix/*
    └── Production hotfixes
```

### 11.3 Testing Strategy

```
TESTING LEVELS
├── Unit Tests
│   ├── Stat calculation
│   ├── Economy logic
│   └── Data validation
│
├── Integration Tests
│   ├── Save/load cycle
│   ├── Network replication
│   └── Part installation
│
├── Functional Tests
│   ├── Race completion
│   ├── Customization flow
│   └── Progression systems
│
├── Performance Tests
│   ├── Frame rate benchmarks
│   ├── Memory profiling
│   └── Network stress tests
│
└── Playtests
    ├── Internal (weekly)
    ├── External (milestone)
    └── Beta (pre-release)
```

### 11.4 Asset Pipeline

```
ASSET WORKFLOW
├── Modeling
│   ├── Tool: Blender/Maya
│   ├── Export: FBX
│   └── Import: UE5 with settings preset
│
├── Texturing
│   ├── Tool: Substance Painter
│   ├── Export: PNG/TGA
│   └── Import: Texture compression per type
│
├── Audio
│   ├── Tool: Various DAWs
│   ├── Export: WAV
│   └── Import: UE5 Sound/MetaSound
│
└── Quality Gates
    ├── Poly count limits
    ├── Texture resolution limits
    ├── Naming conventions
    └── Review process
```

---

## Related Documents

- [Game Design Document](../design/GDD.md)
- [Vehicle Systems Specification](../design/VehicleSystems.md)
- [Multiplayer Systems Design](../design/MultiplayerSystems.md)

---

**Document Version:** 1.0
**Project:** MIDNIGHT GRIND
**Status:** Development Ready
