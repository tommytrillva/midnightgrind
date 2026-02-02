// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// MASTER SHARED TYPES HEADER - All common types defined here
// Other files should include this instead of defining duplicates

#include "CoreMinimal.h"
#include "MGSharedTypes.generated.h"

// ============================================================================
// CHALLENGE TYPES
// ============================================================================

/**
 * @brief Types of challenges available to players
 *
 * Challenges provide goals with rewards. Different types have different
 * reset schedules and availability windows.
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
    Daily,      ///< Resets every 24 hours
    Weekly,     ///< Resets every 7 days
    Seasonal,   ///< Available during a season (typically 3 months)
    Special,    ///< Limited-time special event challenges
    Community,  ///< Community-wide collaborative goals
    Crew,       ///< Crew/clan-specific challenges
    Personal    ///< Player-defined custom challenges
};

/**
 * @brief Difficulty tiers for challenges
 *
 * Higher difficulties offer better rewards but require more skill or time.
 */
UENUM(BlueprintType)
enum class EMGChallengeDifficulty : uint8
{
    Easy,       ///< Suitable for beginners
    Medium,     ///< Standard difficulty
    Hard,       ///< Requires skill and dedication
    Expert,     ///< For experienced players
    Legendary   ///< Extreme difficulty, prestigious rewards
};

// ============================================================================
// EVENT TYPES
// ============================================================================

/**
 * @brief Categories of in-game events
 *
 * Events are time-limited activities with unique rewards and gameplay.
 * Includes both race-type events and seasonal/live service events.
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
    // Race-based event types
    Race,           ///< Standard racing event
    TimeAttack,     ///< Beat-the-clock solo challenge
    TimeTrial,      ///< Time trial challenge (alias for TimeAttack)
    Drift,          ///< Drift scoring competition
    Drag,           ///< Quarter-mile drag racing
    Special,        ///< Unique gameplay mode

    // Live service / seasonal event types
    Daily,          ///< Daily rotating event
    Weekly,         ///< Weekly challenge event
    Weekend,        ///< Weekend special event
    Community,      ///< Community-driven event
    Seasonal,       ///< Tied to real-world seasons
    Holiday,        ///< Holiday-themed limited event
    LimitedTime,    ///< Short-duration limited event
    CrewBattle,     ///< Crew vs crew competition
    Championship,   ///< Multi-race championship series
    Limited         ///< One-time limited availability (legacy)
};

/**
 * @brief Current status of an event
 */
UENUM(BlueprintType)
enum class EMGEventStatus : uint8
{
    Upcoming,   ///< Scheduled but not yet started
    Active,     ///< Currently available to participate
    Completed,  ///< Player has completed this event
    Expired,    ///< Event window has closed
    Cancelled   ///< Event was cancelled
};

// ============================================================================
// VEHICLE TYPES
// ============================================================================

/**
 * @brief Drivetrain configuration types
 *
 * Determines which wheels receive power from the engine and affects
 * handling characteristics significantly.
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
    FWD,    ///< Front-Wheel Drive - power to front wheels
    RWD,    ///< Rear-Wheel Drive - power to rear wheels
    AWD,    ///< All-Wheel Drive - power to all wheels (permanent)
    MR,     ///< Mid-engine Rear-wheel drive
    RR,     ///< Rear-engine Rear-wheel drive
    F4WD    ///< Front-engine 4-Wheel Drive (part-time/selectable)
};

/**
 * @brief Vehicle body style classifications
 *
 * Used for filtering, categorization, and determining physics profiles.
 */
UENUM(BlueprintType)
enum class EMGBodyStyle : uint8
{
    Coupe,      ///< Two-door sports car
    Sedan,      ///< Four-door passenger car
    Hatchback,  ///< Compact with rear hatch
    SUV,        ///< Sport Utility Vehicle
    Truck,      ///< Pickup truck
    Muscle,     ///< American muscle car
    Sports,     ///< Sports car
    Supercar,   ///< High-performance exotic
    Classic,    ///< Vintage/classic vehicle
    JDM,        ///< Japanese Domestic Market
    Wagon,      ///< Station wagon
    Van         ///< Van/MPV
};

/**
 * @brief Engine configuration types for vehicles and audio
 *
 * Determines the base character of the engine. Different engine types
 * have distinct performance and sound signatures.
 *
 * @note Turbocharged, Supercharged, and TwinTurbo represent forced induction
 *       configurations and can be combined with a base engine type.
 */
UENUM(BlueprintType)
enum class EMGEngineType : uint8
{
    I4              UMETA(DisplayName = "Inline-4"),
    I6              UMETA(DisplayName = "Inline-6"),
    V6              UMETA(DisplayName = "V6"),
    V8              UMETA(DisplayName = "V8"),
    V10             UMETA(DisplayName = "V10"),
    V12             UMETA(DisplayName = "V12"),
    Flat4           UMETA(DisplayName = "Flat-4 Boxer"),
    Flat6           UMETA(DisplayName = "Flat-6 Boxer"),
    Rotary          UMETA(DisplayName = "Rotary/Wankel"),
    Electric        UMETA(DisplayName = "Electric"),
    Hybrid          UMETA(DisplayName = "Hybrid"),
    Turbocharged    UMETA(DisplayName = "Turbocharged"),
    Supercharged    UMETA(DisplayName = "Supercharged"),
    TwinTurbo       UMETA(DisplayName = "Twin Turbo")
};

/**
 * @brief Transmission types affecting shift behavior and control
 */
UENUM(BlueprintType)
enum class EMGTransmissionType : uint8
{
    Manual,     ///< Traditional manual with clutch
    Automatic,  ///< Torque converter automatic
    DCT,        ///< Dual-Clutch Transmission
    CVT,        ///< Continuously Variable Transmission
    Sequential  ///< Sequential manual (paddle shift)
};

/**
 * @brief Available fuel types with different performance characteristics
 *
 * Each fuel type affects power output, efficiency, and consumption rate.
 * Some vehicles may only be compatible with specific fuel types.
 */
UENUM(BlueprintType)
enum class EMGFuelType : uint8
{
    Regular         UMETA(DisplayName = "Regular"),
    Gasoline        UMETA(DisplayName = "Gasoline"),
    Premium         UMETA(DisplayName = "Premium"),
    Racing          UMETA(DisplayName = "Racing"),
    Diesel          UMETA(DisplayName = "Diesel"),
    Electric        UMETA(DisplayName = "Electric"),
    Hybrid          UMETA(DisplayName = "Hybrid"),
    Nitromethane    UMETA(DisplayName = "Nitromethane"),
    E85             UMETA(DisplayName = "E85 Ethanol")
};

/**
 * @brief Vehicle era classification for filtering and theming
 */
UENUM(BlueprintType)
enum class EMGVehicleEra : uint8
{
    Classic,    ///< Pre-1980 vehicles
    Retro,      ///< 1980-1999 vehicles
    Modern,     ///< 2000-2015 vehicles
    Current,    ///< 2016-present vehicles
    Future      ///< Concept/futuristic vehicles
};

// ============================================================================
// RACE TYPES
// ============================================================================

/**
 * @brief Types of races available in the game
 *
 * Each race type has unique rules, scoring, and win conditions.
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
    Circuit,        ///< Multi-lap closed circuit race
    Sprint,         ///< Point-to-point single-run race
    Drag,           ///< Straight-line acceleration race
    Drift,          ///< Drift scoring competition
    TimeAttack,     ///< Beat-the-clock solo challenge
    Elimination,    ///< Last place eliminated each lap
    PinkSlip,       ///< Winner takes loser's vehicle
    Touge,          ///< Mountain pass downhill race
    Knockout,       ///< Last car standing wins
    Checkpoint,     ///< Race through checkpoints against time
    FreeRoam        ///< Open-world exploration mode
};

UENUM(BlueprintType)
enum class EMGRaceState : uint8
{
    None        UMETA(DisplayName = "None"),
    PreRace     UMETA(DisplayName = "Pre-Race"),
    Lobby       UMETA(DisplayName = "Lobby"),
    Loading     UMETA(DisplayName = "Loading"),
    Countdown   UMETA(DisplayName = "Countdown"),
    Racing      UMETA(DisplayName = "Racing"),
    Paused      UMETA(DisplayName = "Paused"),
    Finishing   UMETA(DisplayName = "Finishing"),
    Finished    UMETA(DisplayName = "Finished"),
    Results     UMETA(DisplayName = "Results"),
    Returning   UMETA(DisplayName = "Returning"),
    Aborted     UMETA(DisplayName = "Aborted")
};

/**
 * @brief Traffic density levels for open-world races
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
    None,   ///< No civilian traffic
    Light,  ///< Minimal traffic
    Medium, ///< Moderate traffic
    Heavy,  ///< Dense traffic
    Rush    ///< Rush hour conditions
};

/**
 * @brief Performance class tiers for vehicle classification
 *
 * Vehicles are grouped into classes based on their Performance Index.
 * Used for matchmaking and class-restricted events.
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
    D,      ///< Entry-level (PI 100-399)
    C,      ///< Street (PI 400-499)
    B,      ///< Sport (PI 500-599)
    A,      ///< Super (PI 600-699)
    S,      ///< Supercar (PI 700-799)
    SPlus,  ///< Elite (PI 800-899)
    Hyper,  ///< Hypercar (PI 900-998)
    Legend  ///< Unrestricted (PI 999+)
};

// ============================================================================
// ECONOMY TYPES
// ============================================================================

/**
 * @brief Types of currency in the Midnight Grind economy.
 *
 * Each currency serves a distinct purpose and is earned through
 * different activities, encouraging varied gameplay.
 */
UENUM(BlueprintType)
enum class EMGCurrencyType : uint8
{
    Cash            UMETA(DisplayName = "Cash"),
    GrindCash       UMETA(DisplayName = "Grind Cash"),
    Premium         UMETA(DisplayName = "Premium"),
    NeonCredits     UMETA(DisplayName = "Neon Credits"),
    Reputation      UMETA(DisplayName = "Reputation"),
    Crew            UMETA(DisplayName = "Crew"),
    CrewTokens      UMETA(DisplayName = "Crew Tokens"),
    Event           UMETA(DisplayName = "Event"),
    SeasonPoints    UMETA(DisplayName = "Season Points"),
    Legacy          UMETA(DisplayName = "Legacy"),
    LegacyMarks     UMETA(DisplayName = "Legacy Marks")
};

/**
 * @brief Types of rewards that can be earned or purchased
 */
UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
    Cash,       ///< In-game currency (GrindCash)
    Premium,    ///< Premium currency (Neon Credits)
    Vehicle,    ///< Complete vehicle
    Part,       ///< Performance part
    Cosmetic,   ///< Visual customization item
    XP,         ///< Experience points
    Reputation, ///< Reputation points
    Crate,      ///< Loot crate/mystery box
    Bundle      ///< Collection of multiple items
};

/**
 * @brief Types of marketplace listings
 */
UENUM(BlueprintType)
enum class EMGListingType : uint8
{
    FixedPrice, ///< Buy-it-now price
    Auction,    ///< Timed bidding
    Trade       ///< Item-for-item exchange
};

/**
 * @brief Status of a marketplace listing
 */
UENUM(BlueprintType)
enum class EMGListingStatus : uint8
{
    Active,     ///< Currently available
    Sold,       ///< Transaction completed
    Expired,    ///< Listing time ended
    Cancelled,  ///< Seller cancelled
    Pending     ///< Awaiting confirmation
};

// ============================================================================
// POLICE/HEAT TYPES
// ============================================================================

/**
 * @brief Police heat/wanted levels
 *
 * Higher levels bring more aggressive pursuit units and tactics.
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
    None,   ///< No police attention
    Level1, ///< Minor attention, patrol cars
    Level2, ///< Active pursuit, interceptors
    Level3, ///< Aggressive pursuit, SUVs join
    Level4, ///< Roadblocks and spike strips
    Level5  ///< Maximum response, helicopters
};

/**
 * @brief Current state of police pursuit
 */
UENUM(BlueprintType)
enum class EMGPursuitState : uint8
{
    None,       ///< No pursuit active
    Detected,   ///< Police have spotted you
    Pursuit,    ///< Active chase in progress
    Evading,    ///< Lost visual, searching
    Escaped,    ///< Successfully evaded
    Busted,     ///< Caught by police
    Cooldown    ///< Post-pursuit cooldown period
};

/**
 * @brief Types of police units that can pursue
 */
UENUM(BlueprintType)
enum class EMGPoliceUnitType : uint8
{
    Patrol,     ///< Standard patrol car
    Interceptor,///< High-speed pursuit vehicle
    SUV,        ///< Heavy pursuit SUV
    Helicopter, ///< Air support helicopter
    Roadblock,  ///< Stationary roadblock unit
    Spike       ///< Spike strip deployment unit
};

/**
 * @brief Types of traffic violations that increase heat
 */
UENUM(BlueprintType)
enum class EMGAntiCheatViolationType : uint8
{
    None,            ///< No specific type (used as default/filter)
    Speeding,        ///< Excessive speed
    RecklessDriving, ///< Dangerous driving behavior
    PropertyDamage,  ///< Destroying property
    HitAndRun,       ///< Collision with civilians
    Evasion,         ///< Fleeing from police
    Racing           ///< Illegal street racing
};

// ============================================================================
// SOCIAL/PARTY TYPES
// ============================================================================

/**
 * @brief State of a party
 */
UENUM(BlueprintType)
enum class EMGPartyState : uint8
{
    None            UMETA(DisplayName = "None"),
    Forming         UMETA(DisplayName = "Forming"),
    Ready           UMETA(DisplayName = "Ready"),
    InMatch         UMETA(DisplayName = "In Match"),
    InMatchmaking   UMETA(DisplayName = "In Matchmaking"),
    InSession       UMETA(DisplayName = "In Session"),
    Returning       UMETA(DisplayName = "Returning"),
    Disbanded       UMETA(DisplayName = "Disbanded")
};

/**
 * @brief Roles within a party
 */
UENUM(BlueprintType)
enum class EMGPartyRole : uint8
{
    Member          UMETA(DisplayName = "Member"),
    Officer         UMETA(DisplayName = "Officer"),
    Moderator       UMETA(DisplayName = "Moderator"),
    Leader          UMETA(DisplayName = "Leader")
};

/**
 * @brief Ranks within a crew/clan hierarchy
 */
UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
    Recruit,    ///< New member, limited permissions
    Member,     ///< Standard member
    Veteran,    ///< Experienced member
    Officer,    ///< Can invite/kick members
    Leader,     ///< Full administrative control
    Founder     ///< Original crew creator
};

/**
 * @brief Reasons a player may be muted
 */
UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
    None,       ///< Not muted
    Manual,     ///< Player chose to mute
    Reported,   ///< Muted due to reports
    Toxic,      ///< Toxic behavior detected
    Spam,       ///< Spam detected
    System      ///< System-enforced mute
};

// ============================================================================
// GAMEPLAY TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EMGDriftGrade : uint8
{
    None    UMETA(DisplayName = "None"),
    D       UMETA(DisplayName = "D"),
    C       UMETA(DisplayName = "C"),
    B       UMETA(DisplayName = "B"),
    A       UMETA(DisplayName = "A"),
    S       UMETA(DisplayName = "S"),
    SS      UMETA(DisplayName = "SS"),
    SSS     UMETA(DisplayName = "SSS")
};

/**
 * @brief Types of ghost recordings for time attack
 */
UENUM(BlueprintType)
enum class EMGGhostType : uint8
{
    Personal,   ///< Player's own best time
    Friend,     ///< Friend's best time
    World,      ///< Global leaderboard record
    Developer,  ///< Developer-set target time
    Rival       ///< Designated rival's time
};

/**
 * @brief Current state of ghost system
 */
UENUM(BlueprintType)
enum class EMGGhostState : uint8
{
    Hidden,     ///< Ghost not visible
    Visible,    ///< Ghost visible during race
    Recording,  ///< Currently recording ghost data
    Playback    ///< Playing back recorded ghost
};

UENUM(BlueprintType)
enum class EMGCheckpointType : uint8
{
    Standard    UMETA(DisplayName = "Standard"),
    Start       UMETA(DisplayName = "Start"),
    Finish      UMETA(DisplayName = "Finish"),
    Lap         UMETA(DisplayName = "Lap"),
    Sector      UMETA(DisplayName = "Sector"),
    Split       UMETA(DisplayName = "Split"),
    Secret      UMETA(DisplayName = "Secret"),
    Shortcut    UMETA(DisplayName = "Shortcut")
};

UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
    Asphalt     UMETA(DisplayName = "Asphalt"),
    Concrete    UMETA(DisplayName = "Concrete"),
    Gravel      UMETA(DisplayName = "Gravel"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Sand        UMETA(DisplayName = "Sand"),
    Snow        UMETA(DisplayName = "Snow"),
    Ice         UMETA(DisplayName = "Ice"),
    Water       UMETA(DisplayName = "Water"),
    Metal       UMETA(DisplayName = "Metal"),
    Wood        UMETA(DisplayName = "Wood"),
    Wet         UMETA(DisplayName = "Wet"),
    Oil         UMETA(DisplayName = "Oil")
};

// ============================================================================
// MATCHMAKING TYPES
// ============================================================================

/**
 * @brief Types of multiplayer matches
 */
UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
    QuickMatch, ///< Fast casual matchmaking
    Ranked,     ///< Competitive ranked match
    Custom,     ///< Custom rules lobby
    Private,    ///< Invite-only private match
    Tournament  ///< Tournament bracket match
};

/**
 * @brief Current state of matchmaking process
 */
UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
    Idle,       ///< Not searching
    Searching,  ///< Looking for match
    Found,      ///< Match found, confirming
    Joining,    ///< Connecting to session
    Cancelled,  ///< Player cancelled matchmaking
    Failed      ///< Matchmaking failed
};

// ============================================================================
// MISC TYPES
// ============================================================================

/**
 * @brief Time of day periods affecting lighting and atmosphere
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
    Dawn,       ///< Early morning sunrise
    Morning,    ///< Morning hours
    Noon,       ///< Midday bright sun
    Afternoon,  ///< Afternoon hours
    Dusk,       ///< Evening sunset
    Night,      ///< Nighttime
    Midnight    ///< Deep night, darkest
};

/**
 * @brief Weather conditions affecting visuals and handling
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
    Clear,      ///< Clear skies
    Cloudy,     ///< Partly cloudy
    Overcast,   ///< Full cloud cover
    LightRain,  ///< Light rain, wet roads
    HeavyRain,  ///< Heavy rain, reduced visibility
    Storm,      ///< Thunderstorm
    Fog,        ///< Fog, limited visibility
    Snow        ///< Snow, icy conditions
};

/**
 * @brief Achievement rarity tiers
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
    Common,     ///< Easy to unlock
    Uncommon,   ///< Requires some effort
    Rare,       ///< Difficult to obtain
    Epic,       ///< Very challenging
    Legendary   ///< Extremely rare
};

/**
 * @brief Priority levels for in-game notifications
 */
UENUM(BlueprintType)
enum class EMGNotificationPriority : uint8
{
    Low,        ///< Background info
    Normal,     ///< Standard notification
    High,       ///< Important notification
    Critical    ///< Urgent, cannot be missed
};

/**
 * @brief Input actions for control binding
 */
UENUM(BlueprintType)
enum class EMGInputAction : uint8
{
    Throttle,   ///< Accelerate
    Brake,      ///< Brake/reverse
    Steer,      ///< Steering input
    Handbrake,  ///< E-brake/handbrake
    Nitro,      ///< Nitrous boost
    ShiftUp,    ///< Upshift gear
    ShiftDown,  ///< Downshift gear
    Horn,       ///< Horn/honk
    LookBack,   ///< Rear view
    Pause       ///< Pause menu
};

UENUM(BlueprintType)
enum class EMGItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
    Mythic      UMETA(DisplayName = "Mythic"),
    Exclusive   UMETA(DisplayName = "Exclusive")
};

/**
 * @brief Catch-up/rubber-banding modes for AI balance
 */
UENUM(BlueprintType)
enum class EMGCatchUpMode : uint8
{
    None,   ///< No catch-up assistance
    Rubber, ///< Rubber-band AI to pack
    Boost,  ///< Give trailing AI speed boost
    Both    ///< Combined rubber + boost
};

/**
 * @brief Types of replay highlights for auto-capture
 */
UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
    Overtake,   ///< Passing another racer
    Drift,      ///< Impressive drift
    NearMiss,   ///< Close call with traffic
    Crash,      ///< Collision/crash
    Finish,     ///< Crossing finish line
    Record,     ///< New personal/track record
    Epic        ///< Epic moment (multiple combined)
};

/**
 * @brief Player reputation tier - standing in the street racing world
 *
 * Each tier unlocks new locations, race types, and features.
 */
UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Rookie      UMETA(DisplayName = "Rookie"),
    Amateur     UMETA(DisplayName = "Amateur"),
    Known       UMETA(DisplayName = "Known"),
    Pro         UMETA(DisplayName = "Pro"),
    Respected   UMETA(DisplayName = "Respected"),
    Expert      UMETA(DisplayName = "Expert"),
    Feared      UMETA(DisplayName = "Feared"),
    Legend      UMETA(DisplayName = "Legend"),
    Icon        UMETA(DisplayName = "Icon")
};

/**
 * @brief Simple drivetrain type
 * @deprecated Use EMGDrivetrainType for full options including MR, RR, F4WD
 */
UENUM(BlueprintType)
enum class EMGDrivetrain : uint8
{
    FWD,    ///< Front-Wheel Drive
    RWD,    ///< Rear-Wheel Drive
    AWD     ///< All-Wheel Drive
};

UENUM(BlueprintType)
enum class EMGPartCategory : uint8
{
    Engine          UMETA(DisplayName = "Engine"),
    Transmission    UMETA(DisplayName = "Transmission"),
    Drivetrain      UMETA(DisplayName = "Drivetrain"),
    Suspension      UMETA(DisplayName = "Suspension"),
    Brakes          UMETA(DisplayName = "Brakes"),
    Wheels          UMETA(DisplayName = "Wheels"),
    Tires           UMETA(DisplayName = "Tires"),
    Aero            UMETA(DisplayName = "Aero"),
    Body            UMETA(DisplayName = "Body"),
    Interior        UMETA(DisplayName = "Interior"),
    Exterior        UMETA(DisplayName = "Exterior"),
    Nitro           UMETA(DisplayName = "Nitro"),
    Nitrous         UMETA(DisplayName = "Nitrous"),
    Electronics     UMETA(DisplayName = "Electronics")
};

UENUM(BlueprintType)
enum class EMGCustomizationCategory : uint8
{
    Paint, Wrap, Wheels, Body, Interior, Lights, Audio, Performance
};

UENUM(BlueprintType)
enum class EMGTuningCategory : uint8
{
    Engine, Transmission, Suspension, Brakes, Differential, Aero, Weight
};

/**
 * @brief Damage zones dividing the vehicle body.
 *
 * The vehicle is divided into zones for localized damage tracking.
 * Each zone has its own health pool and can affect different components.
 */
UENUM(BlueprintType)
enum class EMGDamageZone : uint8
{
    None            UMETA(DisplayName = "None"),
    Front           UMETA(DisplayName = "Front"),
    FrontLeft       UMETA(DisplayName = "Front Left"),
    FrontCenter     UMETA(DisplayName = "Front Center"),
    FrontRight      UMETA(DisplayName = "Front Right"),
    Left            UMETA(DisplayName = "Left"),
    SideLeft        UMETA(DisplayName = "Side Left"),
    Right           UMETA(DisplayName = "Right"),
    SideRight       UMETA(DisplayName = "Side Right"),
    Rear            UMETA(DisplayName = "Rear"),
    RearLeft        UMETA(DisplayName = "Rear Left"),
    RearCenter      UMETA(DisplayName = "Rear Center"),
    RearRight       UMETA(DisplayName = "Rear Right"),
    Top             UMETA(DisplayName = "Top"),
    Roof            UMETA(DisplayName = "Roof"),
    Underbody       UMETA(DisplayName = "Underbody"),
    Engine          UMETA(DisplayName = "Engine"),
    Wheels          UMETA(DisplayName = "Wheels")
};

/**
 * @brief Types of scoring combos that can be chained
 */
UENUM(BlueprintType)
enum class EMGComboType : uint8
{
    Drift,      ///< Drifting combo
    NearMiss,   ///< Near miss chain
    Airtime,    ///< Airtime combo
    Nitro,      ///< Nitro usage chain
    Overtake,   ///< Overtake chain
    Chain       ///< Mixed combo chain
};

/**
 * @brief Types of economic transactions
 */
UENUM(BlueprintType)
enum class EMGTransactionType : uint8
{
    Purchase,   ///< Buying an item
    Sale,       ///< Selling an item
    Trade,      ///< Trading items
    Reward,     ///< Earned as reward
    Refund,     ///< Refunded purchase
    Gift        ///< Gifted from another player
};

/**
 * @brief State of a multiplayer match session
 */
UENUM(BlueprintType) 
enum class EMGMatchState : uint8
{
    Setup,      ///< Initial setup
    Lobby,      ///< Waiting for players
    Loading,    ///< Loading race
    InProgress, ///< Race in progress
    Finished,   ///< Race complete
    Cancelled   ///< Match cancelled
};

/**
 * @brief Replay recording and playback states
 */
UENUM(BlueprintType)
enum class EMGReplayState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Recording       UMETA(DisplayName = "Recording"),
    Playing         UMETA(DisplayName = "Playing"),
    Paused          UMETA(DisplayName = "Paused"),
    Seeking         UMETA(DisplayName = "Seeking"),
    Scrubbing       UMETA(DisplayName = "Scrubbing"),
    Exporting       UMETA(DisplayName = "Exporting")
};

/**
 * @brief Replay recording quality levels
 */
UENUM(BlueprintType)
enum class EMGReplayQuality : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Cinematic   UMETA(DisplayName = "Cinematic")
};

/**
 * @brief Status of a share operation
 */
UENUM(BlueprintType)
enum class EMGShareStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    Processing      UMETA(DisplayName = "Processing"),
    Uploading       UMETA(DisplayName = "Uploading"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

/**
 * @brief Supported gaming platforms for account linking and cross-play
 *
 * Each platform type corresponds to a specific authentication provider
 * and has its own login flow and credential handling.
 */
UENUM(BlueprintType)
enum class EMGPlatformType : uint8
{
    Unknown       UMETA(DisplayName = "Unknown"),
    PC            UMETA(DisplayName = "PC"),
    Steam         UMETA(DisplayName = "Steam"),
    Epic          UMETA(DisplayName = "Epic Games Store"),
    PlayStation   UMETA(DisplayName = "PlayStation"),
    Xbox          UMETA(DisplayName = "Xbox"),
    Nintendo      UMETA(DisplayName = "Nintendo Switch"),
    Switch        UMETA(DisplayName = "Switch"),
    Mobile        UMETA(DisplayName = "Mobile"),
    Mobile_iOS    UMETA(DisplayName = "iOS"),
    Mobile_Android UMETA(DisplayName = "Android"),
    Custom        UMETA(DisplayName = "Custom")
};

// ============================================================================
// VEHICLE MAKE ENUMERATION
// ============================================================================

/**
 * @brief Vehicle manufacturer/make enumeration
 *
 * Fictional brands inspired by real manufacturers for legal reasons.
 * Used for categorization, dealership organization, and part compatibility.
 */
UENUM(BlueprintType)
enum class EMGVehicleMake : uint8
{
    Generic         UMETA(DisplayName = "Generic"),
    
    // Japanese Makes
    Kaze            UMETA(DisplayName = "Kaze"),           // Nissan-inspired
    Raijin          UMETA(DisplayName = "Raijin"),         // Toyota-inspired
    Sakura          UMETA(DisplayName = "Sakura"),         // Honda-inspired
    Tengu           UMETA(DisplayName = "Tengu"),          // Mazda-inspired
    Shogun          UMETA(DisplayName = "Shogun"),         // Mitsubishi-inspired
    Ronin           UMETA(DisplayName = "Ronin"),          // Subaru-inspired
    
    // American Makes
    Venom           UMETA(DisplayName = "Venom"),          // Ford-inspired
    Apex            UMETA(DisplayName = "Apex"),           // Chevrolet-inspired
    Thunder         UMETA(DisplayName = "Thunder"),        // Dodge-inspired
    
    // European Makes
    Blitz           UMETA(DisplayName = "Blitz"),          // BMW-inspired
    Phantom         UMETA(DisplayName = "Phantom"),        // Mercedes-inspired
    Tempest         UMETA(DisplayName = "Tempest"),        // Porsche-inspired
    Pulse           UMETA(DisplayName = "Pulse"),          // Volkswagen-inspired
    
    // Exotic Makes
    Inferno         UMETA(DisplayName = "Inferno"),        // Ferrari-inspired
    Titan           UMETA(DisplayName = "Titan"),          // Lamborghini-inspired
    Spectre         UMETA(DisplayName = "Spectre"),        // McLaren-inspired
    
    // Korean Makes
    Nova            UMETA(DisplayName = "Nova"),           // Hyundai-inspired
    Eclipse         UMETA(DisplayName = "Eclipse"),        // Kia-inspired
    
    // Other
    Custom          UMETA(DisplayName = "Custom"),         // Custom/kit cars
    Unknown         UMETA(DisplayName = "Unknown")
};

// ============================================================================
// TRICK TYPE ENUMERATION
// ============================================================================

/**
 * @brief Types of tricks/stunts that can be performed
 *
 * Used by the scoring system to identify and reward different maneuvers.
 */
UENUM(BlueprintType)
enum class EMGTrickType : uint8
{
    None            UMETA(DisplayName = "None"),
    
    // Drift Tricks
    Drift           UMETA(DisplayName = "Drift"),
    DriftCombo      UMETA(DisplayName = "Drift Combo"),
    DriftChain      UMETA(DisplayName = "Drift Chain"),
    TandemDrift     UMETA(DisplayName = "Tandem Drift"),
    
    // Air Tricks
    Jump            UMETA(DisplayName = "Jump"),
    BigAir          UMETA(DisplayName = "Big Air"),
    Barrel          UMETA(DisplayName = "Barrel Roll"),
    Flip            UMETA(DisplayName = "Flip"),
    
    // Driving Tricks
    NearMiss        UMETA(DisplayName = "Near Miss"),
    Oncoming        UMETA(DisplayName = "Oncoming Traffic"),
    SlipStream      UMETA(DisplayName = "Slipstream"),
    CleanSection    UMETA(DisplayName = "Clean Section"),
    
    // Overtake Tricks
    Overtake        UMETA(DisplayName = "Overtake"),
    DraftOvertake   UMETA(DisplayName = "Draft Overtake"),
    
    // Burnout/Wheelie
    Burnout         UMETA(DisplayName = "Burnout"),
    Donut           UMETA(DisplayName = "Donut"),
    Wheelie         UMETA(DisplayName = "Wheelie"),
    
    // Takedowns
    Takedown        UMETA(DisplayName = "Takedown"),
    PitManeuver     UMETA(DisplayName = "PIT Maneuver"),
    
    // Special
    PerfectLanding  UMETA(DisplayName = "Perfect Landing"),
    Combo           UMETA(DisplayName = "Combo")
};

// ============================================================================
// RIVALRY TYPES
// ============================================================================

/**
 * @brief Intensity level of a rivalry relationship
 * 
 * Moved from MGRivalsSubsystem.h to break circular dependency.
 */
UENUM(BlueprintType)
enum class EMGRivalryIntensity : uint8
{
    Neutral,       ///< Just another racer, no real history
    Acquaintance,  ///< Raced a few times, starting to recognize
    Competitor,    ///< Regular opponent, competitive dynamic forming
    Rival,         ///< True rivalry established, races feel personal
    Nemesis        ///< Ultimate rival - only one can hold this designation
};
