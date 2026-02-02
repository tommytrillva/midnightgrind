// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * =============================================================================
 * MGTypes.h - Midnight Grind Comprehensive Shared Types Header
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This is the CENTRAL type definition header for the entire Midnight Grind
 * project. All common structs, enums, forward declarations, and type aliases
 * used across multiple systems should be defined here.
 *
 * PURPOSE:
 * --------
 * - Eliminate duplicate type definitions across the codebase
 * - Provide a single source of truth for common types
 * - Reduce compilation dependencies through forward declarations
 * - Establish consistent type usage patterns
 * - Improve code maintainability and reduce redefinition errors
 *
 * USAGE GUIDELINES:
 * -----------------
 * 1. INCLUDE THIS HEADER when you need any of these types
 * 2. DO NOT redefine types that exist here in other headers
 * 3. ADD new common types here if they're used in 3+ different subsystems
 * 4. REMOVE local type definitions and replace with #include "MGTypes.h"
 *
 * ORGANIZATION:
 * -------------
 * This file is organized into the following sections:
 * 1. Forward Declarations - Classes used throughout the project
 * 2. Core Game Types - Essential enums and structs
 * 3. Vehicle Types - Vehicle-related definitions
 * 4. Race Types - Racing and competition definitions
 * 5. Economy Types - Currency and transactions
 * 6. Challenge & Event Types - Progression systems
 * 7. Police & Heat Types - Law enforcement systems
 * 8. Social & Multiplayer Types - Party, crew, matchmaking
 * 9. Gameplay Types - Drift, combos, scoring
 * 10. Environment Types - Weather, time of day, surface
 * 11. UI & Notification Types - User interface
 * 12. Miscellaneous Types - Other shared types
 *
 * MIGRATION NOTES:
 * ----------------
 * This header consolidates types from:
 * - Core/MGSharedTypes.h (enums)
 * - Catalog/MGCatalogTypes.h (vehicle/part definitions)
 * - Various subsystem headers (duplicate definitions)
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MGTypes.generated.h"

// =============================================================================
// SECTION 1: FORWARD DECLARATIONS
// =============================================================================
// Forward declare commonly used classes to reduce header dependencies.
// This allows you to use pointers/references to these classes without
// including their full headers, speeding up compilation.

// Core Classes
class AMGPlayerController;
class AMGPlayerState;
class AMGGameState;
class AMGGameMode;
class UMGGameInstance;

// Vehicle Classes
class AMGVehiclePawn;
class UMGVehicleMovementComponent;
class UMGVehicleModelData;
class UMGVehicleDataAsset;

// Race Classes
class AMGRaceGameMode;
class AMGCheckpoint;
class AMGCheckpointActor;
class AMGRacingAIController;

// Subsystem Classes
class UMGEconomySubsystem;
class UMGGarageSubsystem;
class UMGRaceFlowSubsystem;
class UMGRacingWheelSubsystem;
class UMGWeatherSubsystem;
class UMGWeatherRacingSubsystem;
class UMGAIDriverProfile;

// =============================================================================
// SECTION 2: CORE GAME TYPES
// =============================================================================

/**
 * @brief Types of currency in the Midnight Grind economy
 *
 * Multiple currency types encourage varied gameplay loops. Players earn
 * different currencies through different activities.
 */
UENUM(BlueprintType)
enum class EMGCurrencyType : uint8
{
    Cash            UMETA(DisplayName = "Cash"),           ///< Base currency from races
    GrindCash       UMETA(DisplayName = "Grind Cash"),     ///< Premium earned currency
    Premium         UMETA(DisplayName = "Premium"),         ///< Real-money currency
    NeonCredits     UMETA(DisplayName = "Neon Credits"),   ///< Cosmetic currency
    Reputation      UMETA(DisplayName = "Reputation"),     ///< Street cred points
    Crew            UMETA(DisplayName = "Crew"),           ///< Crew currency
    CrewTokens      UMETA(DisplayName = "Crew Tokens"),    ///< Crew-specific tokens
    Event           UMETA(DisplayName = "Event"),          ///< Limited-time event currency
    SeasonPoints    UMETA(DisplayName = "Season Points"),  ///< Battle pass points
    Legacy          UMETA(DisplayName = "Legacy"),         ///< Long-term currency
    LegacyMarks     UMETA(DisplayName = "Legacy Marks")    ///< Prestige currency
};

/**
 * @brief Player reputation tier - standing in the street racing world
 *
 * Reputation gates content and affects NPC behavior. Higher tiers unlock
 * exclusive races, vehicles, and features.
 */
UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),     ///< Starting tier, nobody knows you
    Rookie      UMETA(DisplayName = "Rookie"),      ///< Learning the streets
    Amateur     UMETA(DisplayName = "Amateur"),     ///< Getting noticed
    Known       UMETA(DisplayName = "Known"),       ///< Local recognition
    Pro         UMETA(DisplayName = "Pro"),         ///< Skilled competitor
    Respected   UMETA(DisplayName = "Respected"),   ///< Community respect
    Expert      UMETA(DisplayName = "Expert"),      ///< Elite racer
    Feared      UMETA(DisplayName = "Feared"),      ///< Intimidating presence
    Legend      UMETA(DisplayName = "Legend"),      ///< Legendary status
    Icon        UMETA(DisplayName = "Icon")         ///< Ultimate icon
};

/**
 * @brief Item rarity classification
 *
 * Used for vehicles, parts, cosmetics, and rewards. Affects visual
 * presentation, pricing, and drop rates.
 */
UENUM(BlueprintType)
enum class EMGItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),      ///< Basic items
    Uncommon    UMETA(DisplayName = "Uncommon"),    ///< Slightly rare
    Rare        UMETA(DisplayName = "Rare"),        ///< Notable items
    Epic        UMETA(DisplayName = "Epic"),        ///< Very rare
    Legendary   UMETA(DisplayName = "Legendary"),   ///< Extremely rare
    Mythic      UMETA(DisplayName = "Mythic"),      ///< Nearly unique
    Exclusive   UMETA(DisplayName = "Exclusive")    ///< One-of-a-kind
};

/**
 * @brief Platform types for cross-play and account linking
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

// =============================================================================
// SECTION 3: VEHICLE TYPES
// =============================================================================

/**
 * @brief Drivetrain configuration
 *
 * Determines which wheels receive power and fundamentally affects
 * handling characteristics.
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
    FWD,    ///< Front-Wheel Drive - understeer tendency, economy
    RWD,    ///< Rear-Wheel Drive - oversteer tendency, sporty
    AWD,    ///< All-Wheel Drive - balanced, all-weather
    MR,     ///< Mid-engine Rear-wheel drive - exotic sports cars
    RR,     ///< Rear-engine Rear-wheel drive - Porsche style
    F4WD    ///< Front-engine 4WD - trucks and SUVs
};

/**
 * @brief Simplified drivetrain type (legacy)
 * @deprecated Use EMGDrivetrainType for full options
 */
UENUM(BlueprintType)
enum class EMGDrivetrain : uint8
{
    FWD,    ///< Front-Wheel Drive
    RWD,    ///< Rear-Wheel Drive
    AWD     ///< All-Wheel Drive
};

/**
 * @brief Vehicle body style classifications
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
 * @brief Engine configuration types
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
 * @brief Transmission types
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
 * @brief Fuel types
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
 * @brief Vehicle era classification
 */
UENUM(BlueprintType)
enum class EMGVehicleEra : uint8
{
    Classic,    ///< Pre-1980
    Retro,      ///< 1980-1999
    Modern,     ///< 2000-2015
    Current,    ///< 2016-present
    Future      ///< Concept/futuristic
};

/**
 * @brief Vehicle manufacturer/make
 *
 * Fictional brands inspired by real manufacturers. Used for categorization,
 * dealership organization, and part compatibility.
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
    Custom          UMETA(DisplayName = "Custom"),
    Unknown         UMETA(DisplayName = "Unknown")
};

/**
 * @brief Vehicle category for filtering
 */
UENUM(BlueprintType)
enum class EMGVehicleCategory : uint8
{
    JDM,        ///< Japanese imports
    American,   ///< US muscle and sports
    European,   ///< Euro performance
    Korean,     ///< Korean tuners
    Exotic      ///< Rare supercars
};

/**
 * @brief Performance class tiers
 *
 * Based on Performance Index (PI). Used for matchmaking and class restrictions.
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

/**
 * @brief Damage zones on vehicle body
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
 * @brief Damage state severity
 */
UENUM(BlueprintType)
enum class EMGDamageState : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Critical    UMETA(DisplayName = "Critical"),
    Destroyed   UMETA(DisplayName = "Destroyed")
};

// =============================================================================
// SECTION 4: PARTS & CUSTOMIZATION TYPES
// =============================================================================

/**
 * @brief Part upgrade tier levels
 */
UENUM(BlueprintType)
enum class EMGPartTier : uint8
{
    Stock       UMETA(DisplayName = "Stock"),       ///< Factory original
    Street      UMETA(DisplayName = "Street"),      ///< Entry aftermarket (~10-15% gain)
    Sport       UMETA(DisplayName = "Sport"),       ///< Mid-tier (~20-30% gain)
    Race        UMETA(DisplayName = "Race"),        ///< Serious upgrades (~40-60% gain)
    Elite       UMETA(DisplayName = "Elite"),       ///< Top-tier (~70-90% gain)
    Legendary   UMETA(DisplayName = "Legendary")    ///< Best possible (maximum)
};

/**
 * @brief Part category for organization
 */
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

/**
 * @brief Customization categories
 */
UENUM(BlueprintType)
enum class EMGCustomizationCategory : uint8
{
    Paint,          ///< Paint colors
    Wrap,           ///< Vinyl wraps
    Wheels,         ///< Wheel styles
    Body,           ///< Body kits
    Interior,       ///< Interior items
    Lights,         ///< Lighting
    Audio,          ///< Audio systems
    Performance     ///< Performance parts
};

/**
 * @brief Tuning categories
 */
UENUM(BlueprintType)
enum class EMGTuningCategory : uint8
{
    Engine,         ///< Engine tuning
    Transmission,   ///< Gear ratios
    Suspension,     ///< Ride height, stiffness
    Brakes,         ///< Brake balance
    Differential,   ///< Diff settings
    Aero,           ///< Downforce
    Weight          ///< Weight distribution
};

// =============================================================================
// SECTION 5: RACE TYPES
// =============================================================================

/**
 * @brief Race type classifications
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
    Circuit,        ///< Multi-lap closed circuit
    Sprint,         ///< Point-to-point
    Drag,           ///< Straight-line drag race
    Drift,          ///< Drift scoring
    TimeAttack,     ///< Beat the clock
    Elimination,    ///< Last place eliminated
    PinkSlip,       ///< Winner takes loser's car
    Touge,          ///< Mountain pass downhill
    Knockout,       ///< Last car standing
    Checkpoint,     ///< Checkpoint race
    FreeRoam        ///< Open-world exploration
};

/**
 * @brief Race state progression
 */
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
 * @brief Race phase tracking
 */
UENUM(BlueprintType)
enum class EMGRacePhase : uint8
{
    None        UMETA(DisplayName = "None"),
    PreStart    UMETA(DisplayName = "Pre-Start"),
    Countdown   UMETA(DisplayName = "Countdown"),
    Racing      UMETA(DisplayName = "Racing"),
    Finishing   UMETA(DisplayName = "Finishing"),
    Complete    UMETA(DisplayName = "Complete")
};

/**
 * @brief Traffic density for open-world races
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
    None,   ///< No traffic
    Light,  ///< Minimal traffic
    Medium, ///< Moderate traffic
    Heavy,  ///< Dense traffic
    Rush    ///< Rush hour
};

/**
 * @brief Checkpoint types
 */
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

// =============================================================================
// SECTION 6: ECONOMY & REWARDS
// =============================================================================

/**
 * @brief Reward types
 */
UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
    Cash,       ///< In-game currency
    Premium,    ///< Premium currency
    Vehicle,    ///< Complete vehicle
    Part,       ///< Performance part
    Cosmetic,   ///< Visual item
    XP,         ///< Experience points
    Reputation, ///< Reputation points
    Crate,      ///< Loot crate
    Bundle      ///< Item bundle
};

/**
 * @brief Transaction types
 */
UENUM(BlueprintType)
enum class EMGTransactionType : uint8
{
    Purchase,   ///< Buying
    Sale,       ///< Selling
    Trade,      ///< Trading
    Reward,     ///< Earned reward
    Refund,     ///< Refunded purchase
    Gift        ///< Gifted item
};

/**
 * @brief Marketplace listing types
 */
UENUM(BlueprintType)
enum class EMGListingType : uint8
{
    FixedPrice, ///< Buy-it-now
    Auction,    ///< Timed bidding
    Trade       ///< Item-for-item
};

/**
 * @brief Listing status
 */
UENUM(BlueprintType)
enum class EMGListingStatus : uint8
{
    Active,     ///< Available
    Sold,       ///< Completed
    Expired,    ///< Time expired
    Cancelled,  ///< Seller cancelled
    Pending     ///< Awaiting confirmation
};

// =============================================================================
// SECTION 7: CHALLENGES & EVENTS
// =============================================================================

/**
 * @brief Challenge types
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
    Daily,      ///< Resets every 24h
    Weekly,     ///< Resets every 7 days
    Seasonal,   ///< Available during season
    Special,    ///< Limited-time special
    Community,  ///< Community-wide goals
    Crew,       ///< Crew-specific
    Personal    ///< Player-defined
};

/**
 * @brief Challenge difficulty tiers
 */
UENUM(BlueprintType)
enum class EMGChallengeDifficulty : uint8
{
    Easy,       ///< Beginner-friendly
    Medium,     ///< Standard difficulty
    Hard,       ///< Requires skill
    Expert,     ///< For experienced players
    Legendary   ///< Extreme difficulty
};

/**
 * @brief Event types
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
    Race,           ///< Standard race
    TimeAttack,     ///< Beat-the-clock
    TimeTrial,      ///< Time trial
    Drift,          ///< Drift competition
    Drag,           ///< Drag race
    Special,        ///< Unique mode
    Daily,          ///< Daily event
    Weekly,         ///< Weekly event
    Weekend,        ///< Weekend event
    Community,      ///< Community event
    Seasonal,       ///< Seasonal event
    Holiday,        ///< Holiday themed
    LimitedTime,    ///< Limited duration
    CrewBattle,     ///< Crew vs crew
    Championship,   ///< Multi-race series
    Limited         ///< Legacy one-time
};

/**
 * @brief Event status
 */
UENUM(BlueprintType)
enum class EMGEventStatus : uint8
{
    Upcoming,   ///< Scheduled
    Active,     ///< Available now
    Completed,  ///< Player finished
    Expired,    ///< Window closed
    Cancelled   ///< Event cancelled
};

// =============================================================================
// SECTION 8: POLICE & HEAT
// =============================================================================

/**
 * @brief Police heat/wanted levels
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
    None,   ///< No attention
    Level1, ///< Minor attention
    Level2, ///< Active pursuit
    Level3, ///< Aggressive pursuit
    Level4, ///< Roadblocks
    Level5  ///< Maximum response
};

/**
 * @brief Pursuit state
 */
UENUM(BlueprintType)
enum class EMGPursuitState : uint8
{
    None,       ///< No pursuit
    Detected,   ///< Spotted
    Pursuit,    ///< Active chase
    Evading,    ///< Lost visual
    Escaped,    ///< Successfully evaded
    Busted,     ///< Caught
    Cooldown    ///< Post-pursuit cooldown
};

/**
 * @brief Police unit types
 */
UENUM(BlueprintType)
enum class EMGPoliceUnitType : uint8
{
    Patrol,     ///< Standard patrol
    Interceptor,///< High-speed pursuit
    SUV,        ///< Heavy pursuit
    Helicopter, ///< Air support
    Roadblock,  ///< Stationary block
    Spike       ///< Spike strips
};

/**
 * @brief Traffic violation types
 */
UENUM(BlueprintType)
enum class EMGAntiCheatViolationType : uint8
{
    None,            ///< No violation
    Speeding,        ///< Excessive speed
    RecklessDriving, ///< Dangerous driving
    PropertyDamage,  ///< Destroying property
    HitAndRun,       ///< Hit and run
    Evasion,         ///< Fleeing police
    Racing           ///< Illegal racing
};

// =============================================================================
// SECTION 9: SOCIAL & MULTIPLAYER
// =============================================================================

/**
 * @brief Party state
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
 * @brief Party role hierarchy
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
 * @brief Crew rank hierarchy
 */
UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
    Recruit,    ///< New member
    Member,     ///< Standard member
    Veteran,    ///< Experienced
    Officer,    ///< Can invite/kick
    Leader,     ///< Admin control
    Founder     ///< Original creator
};

/**
 * @brief Rivalry intensity levels
 */
UENUM(BlueprintType)
enum class EMGRivalryIntensity : uint8
{
    Neutral,       ///< Just another racer
    Acquaintance,  ///< Raced a few times
    Competitor,    ///< Regular opponent
    Rival,         ///< True rivalry
    Nemesis        ///< Ultimate rival
};

/**
 * @brief Mute reasons
 */
UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
    None,       ///< Not muted
    Manual,     ///< Player choice
    Reported,   ///< Due to reports
    Toxic,      ///< Toxic behavior
    Spam,       ///< Spam detected
    System      ///< System-enforced
};

/**
 * @brief Match types
 */
UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
    QuickMatch, ///< Fast casual
    Ranked,     ///< Competitive ranked
    Custom,     ///< Custom rules
    Private,    ///< Invite-only
    Tournament  ///< Tournament bracket
};

/**
 * @brief Matchmaking state
 */
UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
    Idle,       ///< Not searching
    Searching,  ///< Looking for match
    Found,      ///< Match found
    Joining,    ///< Connecting
    Cancelled,  ///< Cancelled
    Failed      ///< Failed
};

/**
 * @brief Match session state
 */
UENUM(BlueprintType) 
enum class EMGMatchState : uint8
{
    Setup,      ///< Initial setup
    Lobby,      ///< Waiting for players
    Loading,    ///< Loading race
    InProgress, ///< Race active
    Finished,   ///< Complete
    Cancelled   ///< Cancelled
};

// =============================================================================
// SECTION 10: GAMEPLAY MECHANICS
// =============================================================================

/**
 * @brief Drift grade scoring
 */
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
 * @brief Ghost recording types
 */
UENUM(BlueprintType)
enum class EMGGhostType : uint8
{
    Personal,   ///< Player's best
    Friend,     ///< Friend's best
    World,      ///< Global record
    Developer,  ///< Dev target
    Rival       ///< Rival's time
};

/**
 * @brief Ghost system state
 */
UENUM(BlueprintType)
enum class EMGGhostState : uint8
{
    Hidden,     ///< Not visible
    Visible,    ///< Visible during race
    Recording,  ///< Recording now
    Playback    ///< Playing back
};

/**
 * @brief Trick/stunt types
 */
UENUM(BlueprintType)
enum class EMGTrickType : uint8
{
    None            UMETA(DisplayName = "None"),
    Drift           UMETA(DisplayName = "Drift"),
    DriftCombo      UMETA(DisplayName = "Drift Combo"),
    DriftChain      UMETA(DisplayName = "Drift Chain"),
    TandemDrift     UMETA(DisplayName = "Tandem Drift"),
    Jump            UMETA(DisplayName = "Jump"),
    BigAir          UMETA(DisplayName = "Big Air"),
    Barrel          UMETA(DisplayName = "Barrel Roll"),
    Flip            UMETA(DisplayName = "Flip"),
    NearMiss        UMETA(DisplayName = "Near Miss"),
    Oncoming        UMETA(DisplayName = "Oncoming Traffic"),
    SlipStream      UMETA(DisplayName = "Slipstream"),
    CleanSection    UMETA(DisplayName = "Clean Section"),
    Overtake        UMETA(DisplayName = "Overtake"),
    DraftOvertake   UMETA(DisplayName = "Draft Overtake"),
    Burnout         UMETA(DisplayName = "Burnout"),
    Donut           UMETA(DisplayName = "Donut"),
    Wheelie         UMETA(DisplayName = "Wheelie"),
    Takedown        UMETA(DisplayName = "Takedown"),
    PitManeuver     UMETA(DisplayName = "PIT Maneuver"),
    PerfectLanding  UMETA(DisplayName = "Perfect Landing"),
    Combo           UMETA(DisplayName = "Combo")
};

/**
 * @brief Combo chain types
 */
UENUM(BlueprintType)
enum class EMGComboType : uint8
{
    Drift,      ///< Drifting combo
    NearMiss,   ///< Near miss chain
    Airtime,    ///< Airtime combo
    Nitro,      ///< Nitro chain
    Overtake,   ///< Overtake chain
    Chain       ///< Mixed combo
};

/**
 * @brief Catch-up/rubber-banding modes
 */
UENUM(BlueprintType)
enum class EMGCatchUpMode : uint8
{
    None,   ///< No catch-up
    Rubber, ///< Rubber-band AI
    Boost,  ///< Speed boost to trailing
    Both    ///< Combined rubber + boost
};

// =============================================================================
// SECTION 11: ENVIRONMENT
// =============================================================================

/**
 * @brief Surface types affecting grip
 */
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

/**
 * @brief Time of day periods
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
    Dawn,       ///< Early morning
    Morning,    ///< Morning hours
    Noon,       ///< Midday
    Afternoon,  ///< Afternoon
    Dusk,       ///< Evening sunset
    Night,      ///< Nighttime
    Midnight    ///< Deep night
};

/**
 * @brief Weather conditions
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
    Clear,      ///< Clear skies
    Cloudy,     ///< Partly cloudy
    Overcast,   ///< Full cloud cover
    LightRain,  ///< Light rain
    HeavyRain,  ///< Heavy rain
    Storm,      ///< Thunderstorm
    Fog,        ///< Fog
    Snow        ///< Snow
};

// =============================================================================
// SECTION 12: UI & NOTIFICATIONS
// =============================================================================

/**
 * @brief Notification priority levels
 */
UENUM(BlueprintType)
enum class EMGNotificationPriority : uint8
{
    Low,        ///< Background info
    Normal,     ///< Standard notification
    High,       ///< Important
    Critical    ///< Urgent
};

/**
 * @brief Input action types
 */
UENUM(BlueprintType)
enum class EMGInputAction : uint8
{
    Throttle,   ///< Accelerate
    Brake,      ///< Brake/reverse
    Steer,      ///< Steering
    Handbrake,  ///< E-brake
    Nitro,      ///< Nitrous boost
    ShiftUp,    ///< Upshift
    ShiftDown,  ///< Downshift
    Horn,       ///< Horn
    LookBack,   ///< Rear view
    Pause       ///< Pause menu
};

/**
 * @brief Achievement rarity tiers
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
    Common,     ///< Easy to unlock
    Uncommon,   ///< Requires some effort
    Rare,       ///< Difficult
    Epic,       ///< Very challenging
    Legendary   ///< Extremely rare
};

/**
 * @brief Achievement categories
 */
UENUM(BlueprintType)
enum class EMGAchievementCategory : uint8
{
    Racing      UMETA(DisplayName = "Racing"),
    Drifting    UMETA(DisplayName = "Drifting"),
    Collection  UMETA(DisplayName = "Collection"),
    Tuning      UMETA(DisplayName = "Tuning"),
    Social      UMETA(DisplayName = "Social"),
    Progression UMETA(DisplayName = "Progression"),
    Special     UMETA(DisplayName = "Special"),
    Hidden      UMETA(DisplayName = "Hidden")
};

/**
 * @brief Achievement stat tracking types
 */
UENUM(BlueprintType)
enum class EMGAchievementStatType : uint8
{
    Counter     UMETA(DisplayName = "Counter"),
    Maximum     UMETA(DisplayName = "Maximum"),
    Progress    UMETA(DisplayName = "Progress"),
    Completion  UMETA(DisplayName = "Completion")
};

/**
 * @brief Replay highlight types
 */
UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
    Overtake,   ///< Passing another racer
    Drift,      ///< Impressive drift
    NearMiss,   ///< Close call
    Crash,      ///< Collision
    Finish,     ///< Finish line
    Record,     ///< New record
    Epic        ///< Epic moment
};

/**
 * @brief Replay recording state
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
 * @brief Replay quality levels
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
 * @brief Share operation status
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

// =============================================================================
// SECTION 13: COMMON STRUCTS
// =============================================================================

/**
 * @brief Base vehicle statistics
 *
 * Stock (unmodified) specifications for a vehicle. Used as baseline
 * for the parts system to modify.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGVehicleBaseStats
{
    GENERATED_BODY()

    /** Engine horsepower (HP) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Power = 200;

    /** Engine torque in lb-ft */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Torque = 200;

    /** Curb weight in pounds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Weight = 2800;

    /** Weight distribution - % over front wheels */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 WeightDistributionFront = 55;

    /** Drivetrain configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EMGDrivetrain Drivetrain = EMGDrivetrain::RWD;

    /** Engine displacement in cc */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Displacement = 2000;

    /** Maximum engine RPM */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Redline = 7000;

    /** Top speed in MPH (stock) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TopSpeed = 150.0f;

    /** 0-60 MPH time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float Acceleration0to60 = 6.0f;
};

/**
 * @brief Vehicle economy data
 *
 * Pricing and cost multipliers for vehicle ownership.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGVehicleEconomy
{
    GENERATED_BODY()

    /** Dealership purchase price */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 BasePurchasePrice = 25000;

    /** Base resale value */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 StreetValue = 30000;

    /** Maximum value when fully built */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 LegendaryValue = 60000;

    /** Repair/maintenance cost multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    float MaintenanceCostMultiplier = 1.0f;

    /** Aftermarket parts price multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    float PartsPriceMultiplier = 1.0f;

    /** Insurance tier (A=expensive, D=cheap) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    FString InsuranceClass = TEXT("C");
};

/**
 * @brief Vehicle performance index data
 *
 * PI is a single number representing overall capability.
 * Used for matchmaking and class restrictions.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGVehiclePerformanceIndex
{
    GENERATED_BODY()

    /** Stock PI rating */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 Base = 500;

    /** Maximum achievable PI with upgrades */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxPotential = 800;

    /** Performance class based on stock PI */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EMGPerformanceClass Class = EMGPerformanceClass::C;
};

/**
 * @brief Simplified vehicle pricing info
 *
 * Lightweight struct for price lookups.
 */
USTRUCT(BlueprintType)
struct FMGVehiclePricingInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    int32 BasePurchasePrice = 25000;

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    int32 StreetValue = 30000;

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    int32 LegendaryValue = 60000;

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    float MaintenanceCostMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    float PartsPriceMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Pricing")
    bool bIsValid = false;
};

/**
 * @brief Race results data
 *
 * Complete results from a finished race.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGRaceResults
{
    GENERATED_BODY()

    /** Final placement (1 = winner) */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    int32 FinalPosition = 0;

    /** Total race time */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    float TotalTime = 0.0f;

    /** Best lap time */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    float BestLapTime = 0.0f;

    /** Total distance driven */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    float TotalDistance = 0.0f;

    /** Cash earned */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    int32 CashEarned = 0;

    /** Reputation earned */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    int32 ReputationEarned = 0;

    /** Experience points earned */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    int32 XPEarned = 0;

    /** Was this a personal best? */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    bool bPersonalBest = false;

    /** Did player win? */
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    bool bVictory = false;
};

// =============================================================================
// END OF MGTypes.h
// =============================================================================
