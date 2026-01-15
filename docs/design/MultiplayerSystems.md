# MIDNIGHT GRIND
## Multiplayer Systems Design
### Version 2.0

---

## TABLE OF CONTENTS

1. [Design Philosophy](#1-design-philosophy)
2. [Social Infrastructure](#2-social-infrastructure)
3. [Multiplayer Modes](#3-multiplayer-modes)
4. [Player Economy](#4-player-economy)
5. [Crew System](#5-crew-system)
6. [Anti-Cheat & Security](#6-anti-cheat--security)
7. [Network Architecture](#7-network-architecture)

---

## 1. DESIGN PHILOSOPHY

### 1.1 Multiplayer Pillars

```
UNIFIED DIFFICULTY
├── Same physics for everyone
├── Server-authoritative calculations
├── Assists provide no competitive advantage
└── Skill + build = results

PLAYER-DRIVEN ECONOMY
├── New cars from NPC dealers (MSRP)
├── Used cars from player market (supply/demand)
├── All parts tradeable
├── Currency earned, never purchased
└── Economic sinks prevent inflation

SOCIAL INFRASTRUCTURE
├── Meet Spot social hub (200 players)
├── Free roam multiplayer (50-100 players)
├── Proximity voice chat
├── Personal garages (visitable)
├── Crew system
└── Photo mode and sharing

ANTI-CHEAT PRIORITY
├── Server-authoritative everything
├── Zero tolerance policy
├── Economy protection
├── Statistical analysis
└── Community reporting
```

### 1.2 Core Principles

1. **No Pay-to-Win** - All currency is earned through gameplay
2. **Unified Experience** - Everyone plays the same game with same rules
3. **Social First** - Community features as important as racing
4. **Fair Competition** - Server authority prevents cheating
5. **Persistent World** - Actions have lasting consequences

---

## 2. SOCIAL INFRASTRUCTURE

### 2.1 Meet Spot (Social Hub)

The Meet Spot is the primary social gathering location where players can showcase cars, socialize, and organize activities.

**Specifications:**
- **Capacity:** 200 players per instance
- **Environment:** Large parking lot with scenic backdrop
- **Features:**
  - Multiple parking zones (crew areas)
  - Stage/showcase area
  - Photo spots with lighting
  - NPC vendors
  - Race organization terminals

**Activities:**
```
MEET SPOT ACTIVITIES
├── Static Display
│   ├── Park your car
│   ├── Pop the hood
│   ├── Show off builds
│   └── Engine rev showcase
│
├── Social Interaction
│   ├── Proximity voice chat
│   ├── Emote system
│   ├── Trade initiation
│   └── Crew recruitment
│
├── Organization
│   ├── Form race lobbies
│   ├── Challenge players
│   ├── Browse player market
│   └── Check leaderboards
│
└── Showcase Events
    ├── Car shows (scheduled)
    ├── Build competitions
    ├── Sound-off contests
    └── Photography contests
```

### 2.2 Free Roam

Open world multiplayer where players share the city environment.

**Specifications:**
- **Capacity:** 50-100 players per server
- **Environment:** Full city (all districts)
- **Features:**
  - Seamless player discovery
  - Dynamic race challenges
  - Shared traffic and police
  - Real-time events

**Player Interaction:**
```
FREE ROAM INTERACTIONS
├── Challenge to Race
│   ├── Flash headlights
│   ├── Accept/Decline prompt
│   ├── Stake negotiation
│   └── Race initiation
│
├── Follow/Cruise
│   ├── Form impromptu groups
│   ├── Group cruise mode
│   └── Shared waypoints
│
├── Communication
│   ├── Horn patterns
│   ├── Proximity voice
│   ├── Quick chat options
│   └── Emotes (out of car)
│
└── Emergent Events
    ├── Police chases (shared)
    ├── Impromptu meetups
    ├── Group races
    └── Territory events
```

### 2.3 Personal Garage

Each player has a personal garage space that can be visited by others.

**Features:**
- Display up to 10 cars
- Custom garage layout/theme
- Trophy/achievement display
- Visitor log
- Visit permissions (public/friends/crew/private)

### 2.4 Communication Systems

#### Proximity Voice Chat

```
VOICE CHAT ZONES
├── Short Range (10m)
│   └── Full volume, clear
├── Medium Range (10-30m)
│   └── Reduced volume
├── Long Range (30-50m)
│   └── Faint, ambient
└── Beyond 50m
    └── Inaudible

VOLUME MODIFIERS
├── Inside car: Reduced range
├── Engine rev: Competes with voice
├── Music playing: Affects clarity
└── Weather: Minor effects
```

#### Party/Crew Voice
- Private channels for groups
- Priority over proximity
- Cross-server (crew chat)

#### Text Chat
- Global (server)
- Local (proximity)
- Crew
- Direct message
- Moderation tools

### 2.5 Emote System

**Categories:**
- Greetings (wave, nod, handshake)
- Reactions (impressed, skeptical, laugh)
- Poses (lean on car, arms crossed)
- Actions (check phone, drink coffee)
- Celebrations (fist pump, victory dance)

---

## 3. MULTIPLAYER MODES

### 3.1 Quick Match

Instant matchmaking for various race types.

**Queue Types:**
| Mode | Players | Matching Criteria |
|------|---------|-------------------|
| Street Sprint | 2-8 | PI class, region |
| Circuit | 4-12 | PI class, region |
| Drag | 2 | PI class, region |
| Drift | 2-8 | PI class, region |
| Highway Battle | 2 | PI class |

**Matchmaking Factors:**
- Performance Index (within class)
- Player skill rating (hidden MMR)
- Connection quality
- Region preference

### 3.2 Custom Lobbies

Player-hosted lobbies with full customization.

**Settings:**
```
LOBBY OPTIONS
├── Race Type
│   ├── Sprint
│   ├── Circuit
│   ├── Drag
│   ├── Drift
│   ├── Highway Battle
│   ├── Touge
│   └── Custom
│
├── Track Selection
│   ├── Specific track
│   ├── Random from set
│   └── Vote-based
│
├── Vehicle Restrictions
│   ├── PI range (e.g., 400-500)
│   ├── Class restriction
│   ├── Specific cars only
│   └── No restrictions
│
├── Rules
│   ├── Collision on/off
│   ├── Traffic on/off
│   ├── Cops on/off
│   ├── Weather setting
│   └── Time of day
│
├── Stakes
│   ├── No stakes
│   ├── Cash buy-in
│   └── Pink slip (requires settings)
│
└── Access
    ├── Public
    ├── Friends only
    ├── Crew only
    └── Password protected
```

### 3.3 Ranked Racing

Competitive mode with seasonal rankings.

**Structure:**
```
RANKED SYSTEM
├── Seasons
│   ├── Duration: 3 months
│   ├── Rank reset: Soft (decay to previous tier floor)
│   └── Rewards: Exclusive cosmetics
│
├── Ranks
│   ├── Bronze I-III
│   ├── Silver I-III
│   ├── Gold I-III
│   ├── Platinum I-III
│   ├── Diamond I-III
│   └── Legend (Top 100)
│
├── Progression
│   ├── Win: +20-30 RP
│   ├── Loss: -15-25 RP
│   ├── Performance bonus: +5-15 RP
│   └── Win streak bonus: +5 RP per streak
│
└── Classes
    ├── Separate ranks per PI class
    ├── Overall rank (combined)
    └── Discipline ranks (sprint, drag, drift)
```

### 3.4 Tournaments

Organized competitive events with brackets.

**Types:**
- Daily tournaments (automated)
- Weekly featured events
- Monthly championships
- Special events (holidays, anniversaries)

**Structure:**
```
TOURNAMENT FORMAT
├── Registration
│   ├── Open window (1-24 hours)
│   ├── Entry fee (optional)
│   └── Requirements (PI, REP, etc.)
│
├── Bracket
│   ├── Single elimination
│   ├── Double elimination
│   ├── Round robin (small groups)
│   └── Swiss system (large events)
│
├── Prizes
│   ├── Cash (in-game currency)
│   ├── Exclusive parts
│   ├── Unique cosmetics
│   └── Title/badge
│
└── Spectating
    ├── Live viewing
    ├── Multiple cameras
    ├── Commentator mode
    └── Replay archive
```

### 3.5 Online Pink Slip Races

The ultimate high-stakes multiplayer experience.

**Requirements:**
```
PINK SLIP REQUIREMENTS
├── REP Requirements
│   ├── Class D: REP Tier 2+
│   ├── Class C: REP Tier 2+
│   ├── Class B: REP Tier 3+
│   ├── Class A: REP Tier 4+
│   └── Class S: REP Tier 5
│
├── Vehicle Requirements
│   ├── Within 50 PI of opponent
│   ├── Not player's only car
│   ├── No active trade lock
│   └── Clear ownership history
│
├── Player Requirements
│   ├── Account in good standing
│   ├── No recent disconnects
│   ├── Verified identity (2FA)
│   └── Agreed to terms
│
└── Cooldowns
    ├── 24 hours after loss
    ├── 1 pink slip per opponent per week
    └── Won cars locked 7 days (no re-wager)
```

**Process:**
```
PINK SLIP FLOW
1. Challenge Issued
   └── Challenger selects their car

2. Challenge Accepted
   └── Defender selects their car

3. Verification
   ├── Both cars meet requirements
   ├── Both players confirm
   └── PI matching validated

4. Race Setup
   ├── Race type selection (mutual agreement)
   ├── Track selection (random from 5)
   └── Final confirmation (triple)

5. Race
   ├── Server-authoritative
   ├── No pause/quit without loss
   └── Disconnect = loss (with grace period)

6. Resolution
   ├── Winner: Receives opponent's car
   ├── Loser: Car removed from garage
   └── Void: Technical issues (rare, both keep)

7. Post-Race
   ├── History recorded
   ├── REP adjusted
   └── Stats updated
```

**Anti-Quit Measures:**
- Disconnecting = automatic loss
- Alt-F4 detection = loss
- Grace period for verified connection issues (30 seconds)
- Repeated disconnects = temporary ban from pink slips
- Collusion detection = permanent ban

---

## 4. PLAYER ECONOMY

### 4.1 Currency System

**Primary Currency: CASH ($)**
- Earned through races, events, sales
- Used for purchases, repairs, modifications
- Cannot be purchased with real money

**Secondary Currency: REP (Reputation)**
- Earned through racing performance
- Gates content access
- Cannot be traded or purchased

### 4.2 Earning Cash

| Source | Amount | Notes |
|--------|--------|-------|
| Street Race Win | $200-$2,000 | Based on difficulty |
| Ranked Race Win | $500-$5,000 | Based on rank |
| Tournament Prize | $5,000-$100,000 | Based on placement |
| Daily Challenges | $500-$2,000 | 3 per day |
| Weekly Challenges | $5,000-$20,000 | 3 per week |
| Selling Cars | Market value | Player-determined |
| Selling Parts | Market value | Player-determined |
| Achievement Bonus | $1,000-$50,000 | One-time |

### 4.3 Spending Cash

| Category | Range | Notes |
|----------|-------|-------|
| New Cars (NPC) | $5,000-$500,000 | Fixed MSRP |
| Used Cars (Market) | Variable | Supply/demand |
| Parts | $50-$15,000 | Per component |
| Repairs | Variable | Based on damage |
| Modifications | $50-$5,000 | Visual only |
| Race Entry Fees | $100-$10,000 | Some events |
| Impound Retrieval | 5-15% car value | Plus storage |

### 4.4 Player Market

The player-driven marketplace for cars and parts.

**Auction House:**
```
AUCTION SYSTEM
├── Listing
│   ├── Starting bid
│   ├── Buy-out price (optional)
│   ├── Duration (1-72 hours)
│   └── Listing fee (2% of starting bid)
│
├── Bidding
│   ├── Bid increments (5% of current bid)
│   ├── Autobid option
│   ├── Snipe protection (extends 5 min)
│   └── Bid history visible
│
├── Completion
│   ├── Winner pays bid amount
│   ├── Seller receives 95% (5% market fee)
│   └── Item transferred immediately
│
└── Safety
    ├── Minimum price floors
    ├── Maximum price ceilings
    ├── Suspicious activity detection
    └── Transaction logging
```

**Direct Trading:**
- Player-to-player trades
- Both parties must confirm
- Trade window shows all items
- No external transactions

**Classified Listings:**
- Fixed-price listings
- Searchable by filters
- First-come-first-served
- Duration: 24-168 hours

### 4.5 Economic Sinks

To prevent inflation, currency is removed through:

```
ECONOMIC SINKS
├── Consumables
│   ├── Tire wear/replacement
│   ├── Part wear/replacement
│   ├── Nitrous refills
│   └── Fuel (minimal)
│
├── Fees
│   ├── Market listing fees
│   ├── Market transaction fees
│   ├── Race entry fees
│   └── Impound fees
│
├── Damage
│   ├── Collision repair
│   ├── Engine damage
│   ├── Mechanical failure
│   └── Totaled vehicles (rare)
│
├── Stakes
│   ├── Lost race bets
│   ├── Lost pink slips
│   └── Failed challenges
│
└── Services
    ├── Garage slots
    ├── Cosmetic customization
    ├── Premium tuning
    └── Storage fees (inactive accounts)
```

### 4.6 Economic Monitoring

```
ANTI-EXPLOITATION
├── Transaction Monitoring
│   ├── Unusual trade patterns
│   ├── Price manipulation
│   ├── Account boosting
│   └── Real-money trading (RMT)
│
├── Price Controls
│   ├── Min/max price ranges per item
│   ├── Outlier transaction flagging
│   └── Market velocity limits
│
├── Enforcement
│   ├── Warning
│   ├── Transaction reversal
│   ├── Currency adjustment
│   └── Account action
│
└── Reporting
    ├── Daily economic reports
    ├── Inflation tracking
    ├── Velocity metrics
    └── Exploit alerts
```

---

## 5. CREW SYSTEM

### 5.1 Crew Structure

```
CREW HIERARCHY
├── OG/Founder
│   ├── Created the crew
│   ├── Full permissions
│   └── Can transfer ownership
│
├── Officers
│   ├── Promoted by OG
│   ├── Invite/kick members
│   ├── Organize events
│   └── Manage crew garage
│
├── Full Members
│   ├── Voting rights
│   ├── Crew chat access
│   ├── Event participation
│   └── Crew garage access
│
└── Prospects
    ├── Limited permissions
    ├── Trial period (7 days)
    ├── Must be promoted to full
    └── Can be removed easily
```

### 5.2 Crew Features

**Crew Identity:**
- Crew name (unique)
- Crew tag (4 characters)
- Crew colors (primary/secondary)
- Crew logo (from templates)
- Crew description/motto
- Territory claim (home district)

**Crew Garage:**
- Shared showcase space
- Display member builds
- Crew-owned vehicles (for events)
- Trophy display

**Crew Events:**
- Crew vs. Crew races
- Crew challenges
- Territory battles
- Crew tournaments

### 5.3 Crew Progression

```
CREW LEVELS
├── Level 1 (New)
│   ├── Max Members: 10
│   ├── Features: Basic
│   └── Required: Creation
│
├── Level 2 (Established)
│   ├── Max Members: 25
│   ├── Features: Crew garage
│   └── Required: 10 active members
│
├── Level 3 (Known)
│   ├── Max Members: 50
│   ├── Features: Crew events
│   └── Required: 25 members, 10,000 crew REP
│
├── Level 4 (Respected)
│   ├── Max Members: 75
│   ├── Features: Territory claim
│   └── Required: 50 members, 50,000 crew REP
│
└── Level 5 (Legendary)
    ├── Max Members: 100
    ├── Features: All features
    └── Required: 75 members, 200,000 crew REP
```

### 5.4 Crew Benefits

- Shared reputation bonuses
- Crew-exclusive liveries
- Group race bonuses
- Crew leaderboards
- Territory bonuses (in claimed district)
- Crew achievements

---

## 6. ANTI-CHEAT & SECURITY

### 6.1 Server Authority

```
SERVER-AUTHORITATIVE SYSTEMS
├── Physics Calculation
│   ├── All vehicle physics on server
│   ├── Client sends inputs only
│   ├── Server validates and returns state
│   └── Reconciliation for latency
│
├── Race Results
│   ├── Server determines positions
│   ├── Server calculates times
│   ├── Server validates checkpoints
│   └── No client-side race logic
│
├── Economy
│   ├── All transactions server-side
│   ├── Inventory on server
│   ├── Currency on server
│   └── No client modification possible
│
└── Progression
    ├── REP calculated server-side
    ├── Unlocks validated server-side
    ├── Stats tracked server-side
    └── No local save exploitation
```

### 6.2 Detection Systems

```
CHEAT DETECTION
├── Statistical Analysis
│   ├── Impossible times
│   ├── Impossible speeds
│   ├── Impossible win rates
│   └── Unusual patterns
│
├── Client Integrity
│   ├── File hash verification
│   ├── Memory scanning
│   ├── Process monitoring
│   └── Driver detection
│
├── Behavioral Analysis
│   ├── Input pattern analysis
│   ├── Response time analysis
│   ├── Movement prediction deviation
│   └── Humanness scoring
│
└── Community Reports
    ├── Player reports
    ├── Replay review
    ├── Mass report detection
    └── Report accuracy tracking
```

### 6.3 Enforcement

```
PENALTY SYSTEM
├── Warnings
│   ├── First offense (minor)
│   ├── Explanation of violation
│   └── No lasting penalty
│
├── Temporary Restrictions
│   ├── Ranked ban (1-7 days)
│   ├── Pink slip ban (7-30 days)
│   ├── Trade ban (7-30 days)
│   └── Feature restrictions
│
├── Temporary Suspensions
│   ├── 24-hour suspension
│   ├── 7-day suspension
│   ├── 30-day suspension
│   └── Loss of seasonal rewards
│
└── Permanent Actions
    ├── Permanent feature ban
    ├── Account suspension
    ├── Hardware ban
    └── No appeal (extreme cases)
```

### 6.4 Economy Security

```
ECONOMY PROTECTION
├── Transaction Validation
│   ├── All server-side
│   ├── Atomic transactions
│   ├── Rollback capability
│   └── Audit logging
│
├── Rate Limiting
│   ├── Trades per hour
│   ├── Listings per day
│   ├── Purchases per hour
│   └── Suspicious velocity blocks
│
├── Fraud Detection
│   ├── Pattern recognition
│   ├── Account linking
│   ├── IP tracking
│   └── Device fingerprinting
│
└── RMT Prevention
    ├── Trade monitoring
    ├── Price monitoring
    ├── Account relationship mapping
    └── External platform monitoring
```

---

## 7. NETWORK ARCHITECTURE

### 7.1 Server Infrastructure

```
SERVER ARCHITECTURE
├── Game Servers
│   ├── Race servers (dedicated per race)
│   ├── Free roam servers (regional)
│   ├── Hub servers (Meet Spot)
│   └── Instance management
│
├── Services
│   ├── Matchmaking service
│   ├── Account service
│   ├── Economy service
│   ├── Social service (friends, crews)
│   └── Analytics service
│
├── Data Storage
│   ├── Player database
│   ├── Vehicle database
│   ├── Market database
│   ├── Replay storage
│   └── Audit logs
│
└── CDN
    ├── Asset delivery
    ├── Patch distribution
    └── Static content
```

### 7.2 Regional Deployment

| Region | Coverage | Server Locations |
|--------|----------|------------------|
| NA-East | Eastern US/Canada | Virginia, Ohio |
| NA-West | Western US/Canada | Oregon, California |
| EU-West | Western Europe | London, Paris |
| EU-Central | Central Europe | Frankfurt |
| Asia-Pacific | Southeast Asia, Oceania | Singapore, Sydney |

### 7.3 Latency Handling

```
LATENCY COMPENSATION
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
├── Lag Compensation
│   ├── Lag compensation window (150ms)
│   ├── Hit detection adjustment
│   └── Position interpolation
│
└── Connection Quality
    ├── Ping-based matchmaking
    ├── Connection quality indicator
    ├── Auto-disconnect threshold
    └── Reconnection handling
```

### 7.4 Scalability

```
SCALING STRATEGY
├── Horizontal Scaling
│   ├── Game server auto-scaling
│   ├── Service replication
│   └── Load balancing
│
├── Event Handling
│   ├── Surge capacity
│   ├── Queue management
│   ├── Graceful degradation
│   └── Priority systems
│
├── Database Scaling
│   ├── Read replicas
│   ├── Sharding strategy
│   ├── Cache layers
│   └── Archive policies
│
└── Monitoring
    ├── Real-time metrics
    ├── Alerting
    ├── Capacity planning
    └── Performance tracking
```

---

## Related Documents

- [Game Design Document](./GDD.md)
- [Vehicle Systems Specification](./VehicleSystems.md)
- [Technical Design Document](../technical/TechnicalDesign.md)

---

**Document Version:** 2.0
**Project:** MIDNIGHT GRIND
**Status:** Development Ready
