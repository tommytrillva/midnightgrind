# MIDNIGHT GRIND - ARCHITECTURE DIAGRAMS

**Version:** 1.0
**Last Updated:** January 28, 2026
**Purpose:** Visual representation of system architecture and data flow

---

## TABLE OF CONTENTS

1. [High-Level Architecture](#1-high-level-architecture)
2. [Subsystem Hierarchy](#2-subsystem-hierarchy)
3. [Data Flow Diagrams](#3-data-flow-diagrams)
4. [State Machines](#4-state-machines)
5. [Component Relationships](#5-component-relationships)
6. [Network Architecture](#6-network-architecture)

---

## 1. High-Level Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           MIDNIGHT GRIND ARCHITECTURE                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                         PRESENTATION LAYER                            │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐     │   │
│  │  │   Race HUD  │ │   Garage    │ │    Shop     │ │   Menus     │     │   │
│  │  │   Widgets   │ │   Widgets   │ │   Widgets   │ │   Widgets   │     │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘     │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                      │                                       │
│                                      ▼                                       │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                         GAMEPLAY LAYER                                │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐             │   │
│  │  │   Race    │ │  Police   │ │   Drift   │ │  Pink Slip│             │   │
│  │  │ Subsystem │ │ Subsystem │ │ Subsystem │ │  Handler  │             │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────┘             │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐             │   │
│  │  │    AI     │ │  Traffic  │ │   Crew    │ │Tournament │             │   │
│  │  │ Subsystem │ │ Subsystem │ │ Subsystem │ │ Subsystem │             │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────┘             │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                      │                                       │
│                                      ▼                                       │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                           CORE LAYER                                  │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐             │   │
│  │  │  Vehicle  │ │  Economy  │ │Progression│ │   Save    │             │   │
│  │  │ Movement  │ │ Subsystem │ │ Subsystem │ │ Subsystem │             │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────┘             │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                      │                                       │
│                                      ▼                                       │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                           DATA LAYER                                  │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐             │   │
│  │  │  Vehicle  │ │   Parts   │ │   Track   │ │    AI     │             │   │
│  │  │  Catalog  │ │  Catalog  │ │  Catalog  │ │  Profiles │             │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────┘             │   │
│  │                      │                                                │   │
│  │                      ▼                                                │   │
│  │              ┌───────────────┐                                        │   │
│  │              │  DataTables   │                                        │   │
│  │              │  (JSON/CSV)   │                                        │   │
│  │              └───────────────┘                                        │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Purpose | Examples |
|-------|---------|----------|
| Presentation | User interface, visual feedback | HUD, menus, widgets |
| Gameplay | Game rules, mechanics | Race, drift, police |
| Core | Fundamental systems | Vehicle physics, economy |
| Data | Data storage and access | Catalogs, DataTables |

---

## 2. Subsystem Hierarchy

### GameInstance Subsystems (Persistent)

```
UGameInstance
│
├── DATA SUBSYSTEMS
│   ├── UMGVehicleCatalogSubsystem      [Vehicle data, pricing]
│   ├── UMGPartsCatalogSubsystem        [Parts data, compatibility]
│   └── UMGTrackCatalogSubsystem        [Track configurations]
│
├── ECONOMY SUBSYSTEMS
│   ├── UMGEconomySubsystem             [Currency, transactions]
│   ├── UMGPlayerMarketSubsystem        [Buying/selling vehicles]
│   ├── UMGMechanicSubsystem            [Part installation]
│   └── UMGTradeSubsystem               [Player trading]
│
├── PROGRESSION SUBSYSTEMS
│   ├── UMGCareerSubsystem              [Career mode]
│   ├── UMGProgressionSubsystem         [XP, levels]
│   ├── UMGReputationSubsystem          [REP system]
│   └── UMGAchievementSubsystem         [Achievements]
│
├── SOCIAL SUBSYSTEMS
│   ├── UMGCrewSubsystem                [Crew management]
│   ├── UMGPlayerSocialSubsystem        [Friends, rivals]
│   ├── UMGLeaderboardSubsystem         [Rankings]
│   └── UMGMeetSpotSubsystem            [Social hubs]
│
├── INVENTORY SUBSYSTEMS
│   ├── UMGGarageSubsystem              [Owned vehicles]
│   ├── UMGInventorySubsystem           [Owned parts]
│   └── UMGCustomizationSubsystem       [Visual customization]
│
├── MULTIPLAYER SUBSYSTEMS
│   ├── UMGSessionSubsystem             [Session management]
│   ├── UMGPartySubsystem               [Party system]
│   ├── UMGVoiceChatSubsystem           [Voice communication]
│   └── UMGMatchmakingSubsystem         [Matchmaking]
│
├── PERSISTENCE SUBSYSTEMS
│   ├── UMGSaveSubsystem                [Save/load]
│   ├── UMGCloudSaveSubsystem           [Cloud sync]
│   └── UMGProfileSubsystem             [Player profile]
│
└── INFRASTRUCTURE SUBSYSTEMS
    ├── UMGTelemetrySubsystem           [Analytics]
    ├── UMGCrashReportingSubsystem      [Error reporting]
    └── UMGAntiCheatSubsystem           [Cheat detection]
```

### World Subsystems (Per-Level)

```
UWorld
│
├── RACE SUBSYSTEMS
│   ├── UMGRaceSubsystem                [Race management]
│   ├── UMGTrackSubsystem               [Track data, checkpoints]
│   ├── UMGLapTimingSubsystem           [Lap timing]
│   └── UMGPositionSubsystem            [Race positions]
│
├── POLICE SUBSYSTEMS
│   ├── UMGPoliceSubsystem              [Police presence]
│   ├── UMGPursuitSubsystem             [Active pursuits]
│   └── UMGHeatSubsystem                [Heat levels]
│
├── AI SUBSYSTEMS
│   ├── UMGAISubsystem                  [AI management]
│   ├── UMGTrafficSubsystem             [Traffic vehicles]
│   └── UMGDynamicDifficultySubsystem   [Difficulty scaling]
│
├── ENVIRONMENT SUBSYSTEMS
│   ├── UMGWeatherSubsystem             [Weather conditions]
│   ├── UMGTimeOfDaySubsystem           [Day/night cycle]
│   └── UMGDestructionSubsystem         [Destructible objects]
│
├── SCORING SUBSYSTEMS
│   ├── UMGDriftSubsystem               [Drift scoring]
│   ├── UMGScoringSubsystem             [General scoring]
│   └── UMGBonusSubsystem               [Score bonuses]
│
└── PRESENTATION SUBSYSTEMS
    ├── UMGCinematicSubsystem           [Cutscenes]
    ├── UMGReplaySubsystem              [Replay system]
    └── UMGPhotoModeSubsystem           [Photo mode]
```

---

## 3. Data Flow Diagrams

### Vehicle Purchase Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Player    │     │  Shop UI    │     │  Vehicle    │
│  Selects    │────▶│  Displays   │◀────│  Catalog    │
│  Vehicle    │     │   Price     │     │ GetPrice()  │
└─────────────┘     └──────┬──────┘     └─────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Economy Subsystem     │
              │  CanAfford(Price)?     │
              └───────────┬────────────┘
                          │
            ┌─────────────┴─────────────┐
            │                           │
            ▼                           ▼
    ┌───────────────┐          ┌───────────────┐
    │   Sufficient  │          │ Insufficient  │
    │    Funds      │          │    Funds      │
    └───────┬───────┘          └───────┬───────┘
            │                          │
            ▼                          ▼
    ┌───────────────┐          ┌───────────────┐
    │ SpendCurrency │          │  Show Error   │
    │ AddToGarage   │          │   Message     │
    └───────┬───────┘          └───────────────┘
            │
            ▼
    ┌───────────────┐
    │  Broadcast    │
    │ OnPurchased   │
    └───────────────┘
```

### Race Start Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           RACE START SEQUENCE                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐                                                        │
│  │ Player Joins │                                                        │
│  │    Race      │                                                        │
│  └──────┬───────┘                                                        │
│         │                                                                │
│         ▼                                                                │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐             │
│  │    Race      │────▶│    Track     │────▶│   Weather    │             │
│  │  Subsystem   │     │  Subsystem   │     │  Subsystem   │             │
│  │ Initialize() │     │  LoadTrack() │     │GetConditions()│            │
│  └──────┬───────┘     └──────────────┘     └──────────────┘             │
│         │                                                                │
│         ▼                                                                │
│  ┌──────────────┐     ┌──────────────┐                                  │
│  │     AI       │────▶│   Traffic    │                                  │
│  │  Subsystem   │     │  Subsystem   │                                  │
│  │SpawnOpponents│     │SpawnTraffic()│                                  │
│  └──────┬───────┘     └──────────────┘                                  │
│         │                                                                │
│         ▼                                                                │
│  ┌──────────────┐                                                        │
│  │  Countdown   │──── 3... 2... 1... GO!                                │
│  │   Manager    │                                                        │
│  └──────┬───────┘                                                        │
│         │                                                                │
│         ▼                                                                │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐             │
│  │    Race      │     │    Drift     │     │   Police     │             │
│  │   Started    │────▶│  Subsystem   │────▶│  Subsystem   │             │
│  │              │     │   Active     │     │  Monitoring  │             │
│  └──────────────┘     └──────────────┘     └──────────────┘             │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Catalog Data Access

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         CATALOG DATA ACCESS                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│                        ┌─────────────────┐                               │
│                        │   DataTable     │                               │
│                        │   (DT_Vehicles) │                               │
│                        └────────┬────────┘                               │
│                                 │                                        │
│                                 │ Load at Initialize()                   │
│                                 ▼                                        │
│                        ┌─────────────────┐                               │
│                        │    Vehicle      │                               │
│                        │    Catalog      │                               │
│                        │   Subsystem     │                               │
│                        └────────┬────────┘                               │
│                                 │                                        │
│              ┌──────────────────┼──────────────────┐                     │
│              │                  │                  │                     │
│              ▼                  ▼                  ▼                     │
│     ┌────────────────┐ ┌────────────────┐ ┌────────────────┐            │
│     │   VehicleMap   │ │   ClassMap     │ │   TagMap       │            │
│     │   (By ID)      │ │   (By Class)   │ │   (By Tag)     │            │
│     │   O(1) lookup  │ │   O(1) lookup  │ │   O(1) lookup  │            │
│     └────────────────┘ └────────────────┘ └────────────────┘            │
│              │                  │                  │                     │
│              └──────────────────┼──────────────────┘                     │
│                                 │                                        │
│                                 ▼                                        │
│     ┌───────────────────────────────────────────────────────────────┐   │
│     │                     API FUNCTIONS                              │   │
│     │                                                                │   │
│     │  GetVehicleData(ID)        → FMGVehicleData                   │   │
│     │  GetVehiclesByClass(Class) → TArray<FMGVehicleData>           │   │
│     │  GetVehiclesByTag(Tag)     → TArray<FMGVehicleData>           │   │
│     │  GetVehicleBasePrice(ID)   → float                            │   │
│     │  GetAllVehicles()          → TArray<FMGVehicleData>           │   │
│     │                                                                │   │
│     └───────────────────────────────────────────────────────────────┘   │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 4. State Machines

### Race State Machine

```
                              ┌─────────────┐
                              │   PRERACE   │
                              │  (Setup)    │
                              └──────┬──────┘
                                     │ All players ready
                                     ▼
                              ┌─────────────┐
                              │  COUNTDOWN  │
                              │   (3..1)    │
                              └──────┬──────┘
                                     │ Timer complete
                                     ▼
                    ┌────────────────────────────────┐
                    │            RACING              │
                    │   (Active race in progress)    │
                    └────────────────┬───────────────┘
                                     │
              ┌──────────────────────┼──────────────────────┐
              │                      │                      │
              ▼                      ▼                      ▼
      ┌─────────────┐        ┌─────────────┐        ┌─────────────┐
      │  FINISHED   │        │   PAUSED    │        │   ABORTED   │
      │  (Winner)   │        │   (Menu)    │        │  (Disconn.) │
      └──────┬──────┘        └──────┬──────┘        └─────────────┘
             │                      │
             │                      │ Resume
             │                      ▼
             │               ┌─────────────┐
             │               │   RACING    │
             │               └─────────────┘
             │
             ▼
      ┌─────────────┐
      │  POSTRACE   │
      │  (Results)  │
      └──────┬──────┘
             │
             ▼
      ┌─────────────┐
      │    IDLE     │
      │  (Waiting)  │
      └─────────────┘
```

### Police Pursuit State Machine

```
                    ┌─────────────────────────────────────┐
                    │           PATROL STATE              │
                    │   (Normal police presence)          │
                    └──────────────────┬──────────────────┘
                                       │ Crime detected
                                       ▼
                    ┌─────────────────────────────────────┐
                    │          PURSUIT STATE              │
                    │   (Active chase in progress)        │
                    └──────────────────┬──────────────────┘
                                       │
        ┌──────────────────────────────┼──────────────────────────────┐
        │                              │                              │
        ▼                              ▼                              ▼
┌───────────────┐            ┌───────────────┐            ┌───────────────┐
│   COOLDOWN    │            │     BUST      │            │   ESCAPED     │
│ (Lost visual) │            │  (Arrested)   │            │   (Evaded)    │
└───────┬───────┘            └───────┬───────┘            └───────┬───────┘
        │                            │                            │
        │ Visual regained            │                            │
        ▼                            │                            │
┌───────────────┐                    │                            │
│    PURSUIT    │                    │                            │
│    (Resume)   │                    │                            │
└───────────────┘                    │                            │
                                     │                            │
                                     ▼                            ▼
                            ┌───────────────┐            ┌───────────────┐
                            │   PENALTIES   │            │    REWARD     │
                            │  (Fine, REP-) │            │    (REP+)     │
                            └───────┬───────┘            └───────┬───────┘
                                    │                            │
                                    └────────────┬───────────────┘
                                                 │
                                                 ▼
                                    ┌─────────────────────┐
                                    │       PATROL        │
                                    │    (Heat decays)    │
                                    └─────────────────────┘
```

### Pink Slip State Machine

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      PINK SLIP STATE MACHINE                             │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│                          ┌─────────────┐                                 │
│                          │    IDLE     │                                 │
│                          │  (Waiting)  │                                 │
│                          └──────┬──────┘                                 │
│                                 │ Challenge issued                       │
│                                 ▼                                        │
│                          ┌─────────────┐                                 │
│                          │  CHALLENGE  │                                 │
│                          │  (Pending)  │                                 │
│                          └──────┬──────┘                                 │
│                                 │                                        │
│              ┌──────────────────┼──────────────────┐                     │
│              │ Declined         │ Accepted         │ Expired             │
│              ▼                  ▼                  ▼                     │
│       ┌─────────────┐    ┌─────────────┐    ┌─────────────┐             │
│       │   IDLE      │    │ NEGOTIATION │    │   IDLE      │             │
│       │  (Return)   │    │  (Terms)    │    │  (Timeout)  │             │
│       └─────────────┘    └──────┬──────┘    └─────────────┘             │
│                                 │                                        │
│                                 │ Terms agreed                           │
│                                 ▼                                        │
│                          ┌─────────────┐                                 │
│                          │CONFIRMATION │                                 │
│                          │  (Triple)   │                                 │
│                          └──────┬──────┘                                 │
│                                 │                                        │
│              ┌──────────────────┼──────────────────┐                     │
│              │ Cancelled        │ Confirmed        │                     │
│              ▼                  ▼                  │                     │
│       ┌─────────────┐    ┌─────────────┐          │                     │
│       │   IDLE      │    │   RACING    │          │                     │
│       │  (Backed)   │    │  (Active)   │          │                     │
│       └─────────────┘    └──────┬──────┘          │                     │
│                                 │                  │                     │
│                                 │ Race finished    │                     │
│                                 ▼                  │                     │
│                          ┌─────────────┐          │                     │
│                          │ RESOLUTION  │          │                     │
│                          │  (Winner)   │          │                     │
│                          └──────┬──────┘          │                     │
│                                 │                  │                     │
│                                 │ Vehicle transfer │                     │
│                                 ▼                  │                     │
│                          ┌─────────────┐          │                     │
│                          │  TRANSFER   │          │                     │
│                          │ (Ownership) │          │                     │
│                          └──────┬──────┘          │                     │
│                                 │                  │                     │
│              ┌──────────────────┴──────────────────┘                     │
│              ▼                                                           │
│       ┌─────────────┐                                                    │
│       │    IDLE     │                                                    │
│       │ (Complete)  │                                                    │
│       └─────────────┘                                                    │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 5. Component Relationships

### Vehicle Actor Components

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         AMGVehicleActor                                  │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │              UMGVehicleMovementComponent (Required)             │     │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐               │     │
│  │  │   Engine    │ │Transmission │ │  Suspension │               │     │
│  │  │   Sim       │ │    Sim      │ │     Sim     │               │     │
│  │  └─────────────┘ └─────────────┘ └─────────────┘               │     │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐               │     │
│  │  │    Tires    │ │   Brakes    │ │    Aero     │               │     │
│  │  │    Sim      │ │    Sim      │ │    Sim      │               │     │
│  │  └─────────────┘ └─────────────┘ └─────────────┘               │     │
│  └────────────────────────────────────────────────────────────────┘     │
│                                                                          │
│  ┌─────────────────────────┐  ┌─────────────────────────┐               │
│  │ UMGVehicleDamageComponent│  │ UMGVehicleSFXComponent │               │
│  │  - Damage tracking       │  │  - Engine sounds       │               │
│  │  - Visual damage         │  │  - Tire sounds         │               │
│  │  - Performance impact    │  │  - Collision sounds    │               │
│  └─────────────────────────┘  └─────────────────────────┘               │
│                                                                          │
│  ┌─────────────────────────┐  ┌─────────────────────────┐               │
│  │ UMGNitrousComponent     │  │ UMGTelemetryComponent   │               │
│  │  - Nitrous system       │  │  - Real-time data       │               │
│  │  - Boost management     │  │  - Lap timing           │               │
│  │  - Purge effects        │  │  - Delta tracking       │               │
│  └─────────────────────────┘  └─────────────────────────┘               │
│                                                                          │
│  ┌─────────────────────────┐  ┌─────────────────────────┐               │
│  │ UMGReplayRecordingComp  │  │ UMGNetworkReplicationComp│              │
│  │  - Ghost data capture   │  │  - Network sync         │               │
│  │  - Position history     │  │  - State replication    │               │
│  │  - Input recording      │  │  - Interpolation        │               │
│  └─────────────────────────┘  └─────────────────────────┘               │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Subsystem Dependencies

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      SUBSYSTEM DEPENDENCIES                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│                     ┌─────────────────────┐                              │
│                     │   VehicleCatalog    │◀──────────────────┐         │
│                     │     Subsystem       │                    │         │
│                     └──────────┬──────────┘                    │         │
│                                │                               │         │
│           ┌────────────────────┼────────────────────┐          │         │
│           │                    │                    │          │         │
│           ▼                    ▼                    ▼          │         │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐   │         │
│  │  PlayerMarket  │  │    Mechanic    │  │   Garage       │   │         │
│  │   Subsystem    │  │   Subsystem    │  │   Subsystem    │   │         │
│  └────────────────┘  └────────────────┘  └────────────────┘   │         │
│           │                    │                               │         │
│           │                    │                               │         │
│           ▼                    ▼                               │         │
│  ┌────────────────┐  ┌────────────────┐                       │         │
│  │    Economy     │◀─┤  PartsCatalog  │───────────────────────┘         │
│  │   Subsystem    │  │   Subsystem    │                                 │
│  └────────┬───────┘  └────────────────┘                                 │
│           │                                                              │
│           ▼                                                              │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐             │
│  │  Progression   │◀─┤     Race       │──▶│     Drift     │             │
│  │   Subsystem    │  │   Subsystem    │  │   Subsystem    │             │
│  └────────────────┘  └────────┬───────┘  └────────────────┘             │
│                               │                                          │
│                               ▼                                          │
│                      ┌────────────────┐                                  │
│                      │     Track      │                                  │
│                      │   Subsystem    │                                  │
│                      └────────────────┘                                  │
│                                                                          │
│  Legend:                                                                 │
│  ────▶  Depends on / Uses                                               │
│  ◀────  Is used by                                                      │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 6. Network Architecture

### Client-Server Model

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      NETWORK ARCHITECTURE                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│                         ┌─────────────────┐                              │
│                         │  DEDICATED      │                              │
│                         │  SERVER         │                              │
│                         │  (Authority)    │                              │
│                         └────────┬────────┘                              │
│                                  │                                       │
│              ┌───────────────────┼───────────────────┐                   │
│              │                   │                   │                   │
│              ▼                   ▼                   ▼                   │
│     ┌────────────────┐  ┌────────────────┐  ┌────────────────┐          │
│     │    CLIENT 1    │  │    CLIENT 2    │  │    CLIENT N    │          │
│     │   (Player)     │  │   (Player)     │  │   (Player)     │          │
│     └────────────────┘  └────────────────┘  └────────────────┘          │
│                                                                          │
│  SERVER RESPONSIBILITIES:                                                │
│  ├── Race state management                                              │
│  ├── Collision detection (authoritative)                                │
│  ├── Position validation                                                │
│  ├── Economy transactions                                               │
│  └── Anti-cheat validation                                              │
│                                                                          │
│  CLIENT RESPONSIBILITIES:                                                │
│  ├── Input capture                                                      │
│  ├── Local prediction                                                   │
│  ├── Visual rendering                                                   │
│  ├── Audio playback                                                     │
│  └── UI presentation                                                    │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Replication Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      REPLICATION FLOW                                    │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  CLIENT                          SERVER                          CLIENT  │
│  (Local)                      (Authority)                       (Remote) │
│                                                                          │
│    │                               │                               │     │
│    │  Input (Throttle, Steering)  │                               │     │
│    │─────────────────────────────▶│                               │     │
│    │                               │                               │     │
│    │                               │  Validate & Apply            │     │
│    │                               │        │                      │     │
│    │                               │        ▼                      │     │
│    │                               │  ┌──────────┐                │     │
│    │                               │  │ Physics  │                │     │
│    │                               │  │ Update   │                │     │
│    │                               │  └────┬─────┘                │     │
│    │                               │       │                      │     │
│    │                               │       ▼                      │     │
│    │    Replicate State            │       │  Replicate State     │     │
│    │◀──────────────────────────────│───────┴─────────────────────▶│     │
│    │  (Position, Velocity, etc.)  │  (Position, Velocity, etc.)  │     │
│    │                               │                               │     │
│    │  Apply Correction            │                               │     │
│    │        │                      │         Apply Interpolation  │     │
│    │        ▼                      │               │               │     │
│    │  ┌──────────┐                │         ┌──────────┐         │     │
│    │  │ Smooth   │                │         │ Smooth   │         │     │
│    │  │ Correct  │                │         │ Interp   │         │     │
│    │  └──────────┘                │         └──────────┘         │     │
│    │                               │                               │     │
│                                                                          │
│  Tick Rate: 60 Hz (Server)                                              │
│  Replication Rate: 30 Hz                                                │
│  Interpolation Delay: 100ms                                             │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Session Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      MULTIPLAYER SESSION FLOW                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌─────────────────┐                                                    │
│  │   Main Menu     │                                                    │
│  └────────┬────────┘                                                    │
│           │                                                              │
│           ▼                                                              │
│  ┌─────────────────┐     ┌─────────────────┐                           │
│  │ Create Session  │     │  Find Session   │                           │
│  └────────┬────────┘     └────────┬────────┘                           │
│           │                       │                                     │
│           ▼                       ▼                                     │
│  ┌─────────────────┐     ┌─────────────────┐                           │
│  │  Session Host   │     │ Session Browser │                           │
│  │   (Waiting)     │     │   (List)        │                           │
│  └────────┬────────┘     └────────┬────────┘                           │
│           │                       │ Select & Join                       │
│           │◀──────────────────────┘                                    │
│           │                                                              │
│           ▼                                                              │
│  ┌─────────────────┐                                                    │
│  │     Lobby       │                                                    │
│  │  (Configure)    │                                                    │
│  └────────┬────────┘                                                    │
│           │ All Ready                                                   │
│           ▼                                                              │
│  ┌─────────────────┐                                                    │
│  │   Load Level    │                                                    │
│  │   (Streaming)   │                                                    │
│  └────────┬────────┘                                                    │
│           │                                                              │
│           ▼                                                              │
│  ┌─────────────────┐                                                    │
│  │    In Race      │                                                    │
│  │   (Playing)     │                                                    │
│  └────────┬────────┘                                                    │
│           │ Race Complete                                               │
│           ▼                                                              │
│  ┌─────────────────┐                                                    │
│  │    Results      │                                                    │
│  │   (Post-Race)   │                                                    │
│  └────────┬────────┘                                                    │
│           │                                                              │
│           ▼                                                              │
│  ┌─────────────────┐                                                    │
│  │  Return Lobby   │──── Continue or Exit                              │
│  └─────────────────┘                                                    │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

**Document Version:** 1.0
**Last Updated:** January 28, 2026
**Diagrams Created With:** ASCII Art (Compatible with all systems)
