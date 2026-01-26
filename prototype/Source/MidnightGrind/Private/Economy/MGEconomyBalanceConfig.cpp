// Copyright Midnight Grind. All Rights Reserved.
// Task #5: Economy Balance Configuration - Balanced for car culture grind
// Last Updated: Economy System Balance Pass

#include "Economy/MGEconomySubsystem.h"
#include "Balancing/MGBalancingSubsystem.h"

namespace MGEconomyBalance
{
	// ==========================================
	// CORE BALANCE PHILOSOPHY - MIDNIGHT GRIND ECONOMY
	// ==========================================
	//
	// DESIGN PILLARS:
	// 1. "Feel the Grind, Not the Frustration" - Progression should be
	//    satisfying, not punishing. Players should always feel momentum.
	// 2. "Risk = Reward" - Higher stakes races/modes pay proportionally more.
	// 3. "Car Culture Authenticity" - Parts and vehicles priced realistically.
	// 4. "The Build Journey Matters" - Upgrading a car should feel meaningful.
	//
	// PROGRESSION TIMELINE:
	// - Tutorial + First Car:    0-2 hours (learn mechanics, basic upgrades)
	// - Starter -> Mid-Tier:     8-12 hours (~25-35 races, REP Tier 1-2)
	// - Mid-Tier -> High-End:    15-20 hours (~50-70 races, REP Tier 3)
	// - High-End -> Exotic:      20-30 hours (~70-100 races, REP Tier 4)
	// - Exotic -> Legendary:     40+ hours (~150+ races, REP Tier 5)
	//
	// RACE SESSION TARGETS:
	// - Average race duration: 3-5 minutes
	// - Average session: 45 minutes (8-12 races)
	// - Target earnings per session: $15,000-$40,000 (scales with REP/class)
	// - "One more race" should always feel worth it

	// ==========================================
	// STARTING ECONOMY
	// ==========================================
	constexpr int64 StartingCash = 7500;           // Increased: allows meaningful first upgrades
	constexpr int64 StarterVehicleValue = 0;       // Civic Type R, Focus SVT, Corrado VR6 are free
	constexpr int64 TutorialCompletionBonus = 2500; // Reward for completing tutorial

	// ==========================================
	// RACE TYPE BASE REWARDS (RISK-SCALED)
	// ==========================================
	// BALANCE DECISION: Rewards scale with risk/skill requirement
	// Longer races and higher-skill modes pay more per minute

	// Sprint Races (2-3 laps, ~3 minutes) - Low risk, quick cash
	constexpr int64 SprintBase1st = 3500;
	constexpr int64 SprintBase2nd = 2100;
	constexpr int64 SprintBase3rd = 1400;
	constexpr int64 SprintBaseDNF = 350;

	// Circuit Races (3-5 laps, ~5 minutes) - Standard risk
	constexpr int64 CircuitBase1st = 5000;
	constexpr int64 CircuitBase2nd = 3000;
	constexpr int64 CircuitBase3rd = 2000;
	constexpr int64 CircuitBaseDNF = 500;

	// Drag Races (quick, high stakes) - Binary outcome, moderate risk
	constexpr int64 DragBase1st = 2500;
	constexpr int64 DragBase2nd = 1000;
	constexpr int64 DragBaseDNF = 100;

	// Drift Events (score-based) - Skill-intensive
	constexpr int64 DriftBase1st = 4000;
	constexpr int64 DriftBase2nd = 2400;
	constexpr int64 DriftBase3rd = 1600;
	constexpr int64 DriftBaseDNF = 400;

	// Touge/Canyon Races (1v1, high skill) - INCREASED: High risk/skill
	constexpr int64 TougeBaseWin = 6000;   // Raised from 4500 - deserves more for 1v1 skill
	constexpr int64 TougeBaseLose = 500;

	// Highway Battle (Wangan style) - NEW: High speed = high reward
	constexpr int64 HighwayBattleWin = 7500;
	constexpr int64 HighwayBattleLose = 750;

	// Time Attack (solo, beat ghost/time)
	constexpr int64 TimeAttackGoldReward = 3000;
	constexpr int64 TimeAttackSilverReward = 1500;
	constexpr int64 TimeAttackBronzeReward = 750;

	// ==========================================
	// RACE CLASS MULTIPLIERS (RISK-SCALED)
	// ==========================================
	// BALANCE DECISION: Higher class = more investment = higher rewards
	// This ensures players feel rewarded for building faster cars
	constexpr float ClassDMultiplier = 0.6f;    // Starter tier - learning the ropes
	constexpr float ClassCMultiplier = 0.8f;    // Entry level - first real competition
	constexpr float ClassBMultiplier = 1.0f;    // Standard - the baseline
	constexpr float ClassAMultiplier = 1.35f;   // Competitive - raised from 1.3
	constexpr float ClassSMultiplier = 1.8f;    // High stakes - raised from 1.7
	constexpr float ClassXMultiplier = 2.75f;   // Elite/Hypercar - raised from 2.5

	// ==========================================
	// DIFFICULTY/RISK MULTIPLIERS
	// ==========================================
	// BALANCE DECISION: Players who take risks deserve proportional rewards

	// Race Settings Multipliers
	constexpr float RankedRaceMultiplier = 1.5f;     // Ranked races pay 50% more
	constexpr float NightRaceMultiplier = 1.15f;     // Night races are harder
	constexpr float WetConditionsMultiplier = 1.25f; // Rain adds difficulty
	constexpr float TrafficEnabledMultiplier = 1.2f; // Traffic adds chaos
	constexpr float CopsEnabledMultiplier = 1.3f;    // Police pursuit risk

	// Opponent Count Scaling (more opponents = more risk = more reward)
	constexpr float OpponentBaseMultiplier = 1.0f;
	constexpr float OpponentScalePerRacer = 0.08f;   // +8% per opponent beyond 4

	// ==========================================
	// BONUS MULTIPLIERS
	// ==========================================
	constexpr float CleanRaceBonus = 1.15f;      // No collisions
	constexpr float PerfectStartBonus = 1.05f;   // Perfect launch
	constexpr float BestLapBonus = 1.10f;        // Set fastest lap
	constexpr float ComebackBonus = 1.25f;       // Win from last - raised from 1.2
	constexpr float FlawlessVictoryBonus = 1.30f; // 1st + clean + best lap - raised from 1.25
	constexpr float UnderDogBonus = 1.20f;       // Win with lower PI - raised from 1.15
	constexpr float RivalryBonus = 1.15f;        // Beat a rival - raised from 1.10
	constexpr float FirstRaceOnTrackBonus = 1.10f; // NEW: First time on this track

	// ==========================================
	// WAGER SYSTEM (RISK/REWARD CORE)
	// ==========================================
	// BALANCE DECISION: Wagering is high-risk/high-reward, not exploitable
	constexpr int64 MinWager = 500;
	constexpr int64 MaxWagerMultiplier = 5;      // Up to 5x base race reward
	constexpr float WagerWinMultiplier = 2.0f;   // Double your money
	constexpr float WagerHouseEdge = 0.0f;       // CHANGED: No house edge - player friendly

	// Wager Tier Multipliers (higher wager = higher risk multiplier on reward)
	constexpr float WagerTier1Multiplier = 1.0f;  // 1x-2x base reward wager
	constexpr float WagerTier2Multiplier = 1.1f;  // 2x-3x base reward wager
	constexpr float WagerTier3Multiplier = 1.2f;  // 3x-4x base reward wager
	constexpr float WagerTier4Multiplier = 1.3f;  // 4x-5x base reward wager

	// ==========================================
	// PINK SLIP RACES (ULTIMATE RISK)
	// ==========================================
	// BALANCE DECISION: Pink slips are endgame content, REP-gated
	// Winner takes loser's car - high stakes, can lose everything
	constexpr float PinkSlipMinimumValue = 15000.0f;  // Minimum car value
	constexpr float PinkSlipValueMatchTolerance = 0.3f; // 30% value difference max
	constexpr int32 PinkSlipMinREP = 500;             // NEW: Requires Known tier
	constexpr int32 PinkSlipCashBonusPercent = 10;    // Winner gets 10% of car value as bonus cash

	// ==========================================
	// REP (REPUTATION) REWARDS
	// ==========================================
	// BALANCE DECISION: REP gates content, earned through skill/wins
	// REP Thresholds: Unknown(0), Rookie(100), Known(500), Respected(1500), Feared(5000), Legend(15000)

	// Base REP by Race Type
	constexpr int32 REPSprintWin = 50;
	constexpr int32 REPCircuitWin = 75;
	constexpr int32 REPDragWin = 40;
	constexpr int32 REPDriftWin = 60;
	constexpr int32 REPTougeWin = 120;          // High skill = high REP
	constexpr int32 REPHighwayBattleWin = 100;
	constexpr int32 REPPinkSlipWin = 200;       // Ultimate flex
	constexpr int32 REPTimeTrialGold = 30;

	// REP Loss (losing reputation)
	constexpr int32 REPRaceLossBase = 10;
	constexpr int32 REPPinkSlipLoss = 50;       // Big hit for losing pink slip
	constexpr int32 REPBustHeat1 = 25;
	constexpr int32 REPBustHeat2 = 50;
	constexpr int32 REPBustHeat3 = 100;
	constexpr int32 REPBustHeat4 = 200;
	constexpr int32 REPBustHeat5 = 300;         // Manhunt bust is devastating

	// REP Bonus Multipliers
	constexpr float REPDominantWinMultiplier = 1.25f;  // 10+ seconds ahead
	constexpr float REPComebackMultiplier = 1.5f;      // Win from behind
	constexpr float REPCleanRaceMultiplier = 1.1f;     // No collisions
	constexpr float REPOpponentCountMultiplier = 0.1f; // +10% per opponent

	// ==========================================
	// VEHICLE PRICING TIERS
	// ==========================================
	// BALANCE DECISION: Prices reflect real-world JDM/tuner market
	// Progression should feel like building a real car collection

	// Tier 1: Starter (Free choices) - REP Tier 0
	// Honda Civic Type R EK9: $0 (Free starter)
	// Ford Focus SVT: $0 (Free starter)
	// VW Corrado VR6: $0 (Free starter)

	// Tier 2: Entry Level ($15,000 - $35,000) - REP Tier 1 (Rookie)
	// ~8-15 races to afford
	// Nissan 350Z: $22,000
	// VW Golf R32: $28,000
	// Audi S4 B5: $25,000
	// Mercedes C32 AMG: $35,000
	// Camaro SS: $32,000
	// Firebird Trans Am WS6: $35,000

	// Tier 3: Mid-Range ($38,000 - $55,000) - REP Tier 2 (Known)
	// ~20-30 races to afford
	// Nissan Silvia S15: $45,000
	// Honda S2000: $42,000
	// Acura Integra Type R: $52,000
	// BMW M3 E46: $55,000
	// Lotus Elise S1: $48,000

	// Tier 4: Performance ($55,000 - $95,000) - REP Tier 3 (Respected)
	// ~35-50 races to afford
	// Subaru Impreza STI: $58,000
	// Mitsubishi Evo VI: $65,000
	// Corvette Z06: $68,000
	// Mazda RX-7 FD: $85,000
	// Dodge Viper GTS: $92,000

	// Tier 5: High-End ($95,000 - $180,000) - REP Tier 4 (Feared)
	// ~60-90 races to afford
	// Ferrari 360 Modena: $115,000
	// Porsche 911 GT3: $125,000
	// Nissan Skyline R34 GT-R: $130,000
	// Ford Mustang Cobra R: $120,000
	// Toyota Supra MK4 TT: $150,000
	// Honda NSX Type R: $175,000

	// Tier 6: Exotic/Legendary ($250,000+) - REP Tier 5 (Legend)
	// ~120+ races to afford
	// Lamborghini Diablo SV: $350,000
	// McLaren F1: $25,000,000 (ultimate goal - legend status)

	// ==========================================
	// TUNING PART PRICES BY TIER (200+ PARTS)
	// ==========================================
	// BALANCE DECISION: Parts should feel like meaningful investments
	// A full build on a car costs ~40-60% of its base value
	// This ensures players bond with their builds

	// AIR INTAKE SYSTEM
	constexpr int64 AirIntakeStock = 0;
	constexpr int64 AirIntakeStreet = 250;       // +5-8 HP
	constexpr int64 AirIntakeSport = 600;        // +10-15 HP
	constexpr int64 AirIntakeRace = 1200;        // +15-25 HP
	constexpr int64 AirIntakePro = 2500;         // +25-40 HP
	constexpr int64 AirIntakeElite = 4500;       // +40-60 HP, requires REP Tier 3

	// EXHAUST SYSTEM
	constexpr int64 ExhaustStock = 0;
	constexpr int64 ExhaustStreet = 400;         // Cat-back, +5-10 HP
	constexpr int64 ExhaustSport = 900;          // High-flow, +10-20 HP
	constexpr int64 ExhaustRace = 1800;          // Headers + high-flow, +20-35 HP
	constexpr int64 ExhaustPro = 3500;           // Full race system, +35-50 HP
	constexpr int64 ExhaustTitanium = 6000;      // Titanium full system, +50-70 HP

	// ECU / TUNING
	constexpr int64 ECUStock = 0;
	constexpr int64 ECUStreet = 350;             // +3-5% power
	constexpr int64 ECUSport = 800;              // +5-8% power
	constexpr int64 ECURace = 1500;              // +8-12% power
	constexpr int64 ECUPro = 3000;               // +12-18% power, custom tune
	constexpr int64 ECUStandalone = 5000;        // Full standalone, unlimited potential
	constexpr int64 ECUAntiLag = 2000;           // Anti-lag kit (requires turbo)

	// FORCED INDUCTION - TURBO
	constexpr int64 TurboSmall = 2500;           // +50-80 HP, low lag
	constexpr int64 TurboMedium = 4500;          // +80-150 HP
	constexpr int64 TurboLarge = 7500;           // +150-250 HP
	constexpr int64 TurboMassive = 12000;        // +250-400 HP
	constexpr int64 TwinTurboKit = 18000;        // Specialized kit
	constexpr int64 TurboUpgrades = 3000;        // Wastegate, BOV, intercooler upgrades

	// FORCED INDUCTION - SUPERCHARGER
	constexpr int64 SuperchargerRoots = 4000;    // +80-120 HP, instant power
	constexpr int64 SuperchargerTwinScrew = 6000; // +100-180 HP
	constexpr int64 SuperchargerCentrifugal = 5000; // +80-150 HP
	constexpr int64 SuperchargerPulley = 800;    // Smaller pulley for more boost

	// ENGINE INTERNALS
	constexpr int64 CamshaftStreet = 500;
	constexpr int64 CamshaftSport = 1200;
	constexpr int64 CamshaftRace = 2500;

	constexpr int64 InternalsStreet = 1500;      // Pistons, rods
	constexpr int64 InternalsSport = 3500;
	constexpr int64 InternalsRace = 6000;
	constexpr int64 InternalsForged = 10000;     // Forged rotating assembly
	constexpr int64 InternalsBillet = 18000;     // Billet everything, REP Tier 4

	// ENGINE SWAP (complete replacement)
	constexpr int64 EngineSwapBase = 15000;      // Base swap labor
	constexpr int64 EngineSwapPremium = 35000;   // Premium engine
	constexpr int64 EngineSwapLegendary = 75000; // Legendary swap (2JZ, RB26, etc.)

	// TRANSMISSION
	constexpr int64 TransStreet = 1000;
	constexpr int64 TransSport = 2500;           // Short throw, upgraded synchros
	constexpr int64 TransRace = 5000;            // Sequential or dog box
	constexpr int64 TransDCT = 8000;             // DCT conversion where applicable
	constexpr int64 TransClutch = 800;           // Clutch upgrade
	constexpr int64 TransClutchRace = 2000;      // Multi-plate racing clutch
	constexpr int64 TransClutchTwin = 3500;      // Twin-disc clutch
	constexpr int64 TransFinalDrive = 1500;      // Final drive change
	constexpr int64 TransLSD = 2000;             // Limited slip differential
	constexpr int64 TransLSD2Way = 3500;         // Adjustable 2-way LSD

	// SUSPENSION
	constexpr int64 SuspStreet = 600;            // Lowering springs
	constexpr int64 SuspSport = 1500;            // Coilovers (adjustable height)
	constexpr int64 SuspRace = 3500;             // Full race coilovers (adjustable damping)
	constexpr int64 SuspPro = 6000;              // Multi-adjustable (height, compression, rebound)
	constexpr int64 SuspAir = 4500;              // Air suspension (stance builds)

	constexpr int64 SwayBarFront = 300;
	constexpr int64 SwayBarRear = 300;
	constexpr int64 SwayBarKit = 500;            // Front + rear combo
	constexpr int64 SwayBarAdjustable = 900;     // Adjustable end links

	constexpr int64 AlignmentKit = 200;          // Adjustable arms
	constexpr int64 AlignmentCamberKit = 400;    // Camber plates/arms
	constexpr int64 CageRollBar = 1500;          // Roll bar
	constexpr int64 CageRollCage = 4000;         // Full roll cage
	constexpr int64 ChassisReinforcement = 2500; // Strut bars, braces

	// BRAKES
	constexpr int64 BrakePadsStreet = 200;
	constexpr int64 BrakePadsSport = 400;
	constexpr int64 BrakePadsRace = 800;
	constexpr int64 BrakePadsCarbon = 1500;      // Carbon-ceramic pads

	constexpr int64 BrakeRotorsSlotted = 350;
	constexpr int64 BrakeRotorsDrilled = 400;
	constexpr int64 BrakeRotors2Piece = 800;
	constexpr int64 BrakeRotorsCarbon = 5000;    // Carbon-ceramic rotors

	constexpr int64 BrakeCaliperUpgrade = 1200;
	constexpr int64 BrakeBigBrakeKit = 3500;     // Full BBK
	constexpr int64 BrakeBBKPro = 6000;          // Pro BBK (Brembo GT, etc.)

	constexpr int64 BrakeLines = 150;            // Stainless lines
	constexpr int64 BrakeFluid = 50;             // Racing brake fluid
	constexpr int64 BrakeHydroHandbrake = 800;   // Hydraulic handbrake (drift)

	// WHEELS & TIRES
	constexpr int64 WheelsStreet = 800;          // 17-18" alloys
	constexpr int64 WheelsSport = 1500;          // Lightweight alloys
	constexpr int64 WheelsRace = 3000;           // Forged lightweight
	constexpr int64 WheelsUltra = 5000;          // Premium forged (TE37, CE28, etc.)
	constexpr int64 WheelsDrag = 2000;           // Drag skinnies + slicks

	constexpr int64 TiresAllSeason = 300;
	constexpr int64 TiresSport = 500;
	constexpr int64 TiresPerformance = 800;
	constexpr int64 TiresSemiSlick = 1200;
	constexpr int64 TiresSlick = 2000;           // Track only
	constexpr int64 TiresDrag = 1500;
	constexpr int64 TiresDrift = 600;            // Hard compound drift tires

	// AERODYNAMICS
	constexpr int64 AeroFrontLip = 400;
	constexpr int64 AeroSplitter = 800;
	constexpr int64 AeroSplitterRace = 1500;
	constexpr int64 AeroSplitterCarbon = 2500;

	constexpr int64 AeroRearSpoiler = 500;
	constexpr int64 AeroRearWing = 1200;
	constexpr int64 AeroRearWingRace = 2500;
	constexpr int64 AeroRearWingGT = 4000;       // GT-style big wing

	constexpr int64 AeroDiffuser = 1000;
	constexpr int64 AeroDiffuserRace = 2000;
	constexpr int64 AeroCanards = 600;           // Front canards
	constexpr int64 AeroWideBody = 5000;         // Wide body kit
	constexpr int64 AeroWideBodyPremium = 12000; // Premium widebody (Rocket Bunny, LB, etc.)

	// NITROUS OXIDE SYSTEMS
	constexpr int64 NOS50Shot = 1500;            // +50 HP
	constexpr int64 NOS75Shot = 2000;            // +75 HP
	constexpr int64 NOS100Shot = 2500;           // +100 HP
	constexpr int64 NOS150Shot = 3500;           // +150 HP
	constexpr int64 NOS200Shot = 5000;           // +200 HP
	constexpr int64 NOS300Shot = 8000;           // +300 HP, requires built engine
	constexpr int64 NOSDirectPort = 10000;       // Direct port system
	constexpr int64 NOSPurgeKit = 500;           // Purge kit (cosmetic + function)
	constexpr int64 NOSRefill = 50;              // Per lb

	// WEIGHT REDUCTION
	constexpr int64 WeightStage1 = 500;          // Remove spare, mats (-20 lbs)
	constexpr int64 WeightStage2 = 1500;         // Lightweight seats, panels (-50 lbs)
	constexpr int64 WeightStage3 = 3500;         // Full strip, carbon panels (-100 lbs)
	constexpr int64 WeightStage4 = 7000;         // Full race prep (-150+ lbs)
	constexpr int64 WeightCarbonHood = 1200;     // Carbon fiber hood (-30 lbs)
	constexpr int64 WeightCarbonTrunk = 900;     // Carbon fiber trunk (-20 lbs)
	constexpr int64 WeightCarbonRoof = 2500;     // Carbon fiber roof (-25 lbs)
	constexpr int64 WeightLexanWindows = 800;    // Lexan windows (-40 lbs)

	// ==========================================
	// COSMETIC PRICING (NON-PERFORMANCE)
	// ==========================================
	// BALANCE DECISION: Cosmetics are affordable - personalization matters

	// PAINT
	constexpr int64 PaintSolid = 500;
	constexpr int64 PaintMetallic = 1000;
	constexpr int64 PaintPearlescent = 1500;
	constexpr int64 PaintMatte = 2000;
	constexpr int64 PaintChrome = 3000;
	constexpr int64 PaintColorShift = 4000;      // Color-shifting paint
	constexpr int64 PaintCustom = 5000;

	// VINYL / WRAP
	constexpr int64 VinylBasic = 200;
	constexpr int64 VinylPremium = 500;
	constexpr int64 WrapFull = 3000;
	constexpr int64 WrapPremium = 6000;
	constexpr int64 WrapItasha = 4000;           // Anime wrap style

	// BODY KITS
	constexpr int64 BodyKitFrontBumper = 800;
	constexpr int64 BodyKitRearBumper = 600;
	constexpr int64 BodyKitSideSkirts = 500;
	constexpr int64 BodyKitFull = 2500;
	constexpr int64 BodyKitPremium = 5000;
	constexpr int64 BodyKitBrand = 8000;         // Licensed brand kits

	// INTERIOR
	constexpr int64 InteriorSeats = 1200;
	constexpr int64 InteriorSeatsBucket = 2500;  // Full bucket seats
	constexpr int64 InteriorSteeringWheel = 400;
	constexpr int64 InteriorSteeringWheelPremium = 1000;
	constexpr int64 InteriorShiftKnob = 150;
	constexpr int64 InteriorGauges = 600;
	constexpr int64 InteriorGaugesPod = 400;     // Gauge pod
	constexpr int64 InteriorHarness = 300;       // Racing harness

	// LIGHTING
	constexpr int64 LightsHeadlights = 600;
	constexpr int64 LightsHeadlightsPro = 1500;  // Projector/LED conversion
	constexpr int64 LightsTaillights = 400;
	constexpr int64 LightsTaillightsCustom = 800;
	constexpr int64 LightsNeon = 800;            // Underglow
	constexpr int64 LightsNeonPremium = 1500;    // RGB underglow

	// ==========================================
	// REPAIR COSTS
	// ==========================================
	// BALANCE DECISION: Repairs are minor money sinks, not punishing
	// Based on percentage of vehicle value
	constexpr float RepairMinorDamagePercent = 0.003f;  // 0.3% (reduced from 0.5%)
	constexpr float RepairModerateDamagePercent = 0.015f; // 1.5% (reduced from 2%)
	constexpr float RepairMajorDamagePercent = 0.04f;   // 4% (reduced from 5%)
	constexpr float RepairTotalDamagePercent = 0.12f;   // 12% (reduced from 15%)

	// ==========================================
	// SELL BACK VALUES
	// ==========================================
	// BALANCE DECISION: Generous sell-back to allow build experimentation
	constexpr float PartSellBackPercent = 0.6f;       // 60% of purchase price (up from 50%)
	constexpr float VehicleSellBackPercent = 0.75f;   // 75% of market value (up from 70%)
	constexpr float VehicleDepreciationPerRace = 0.0005f; // 0.05% per race (reduced)

	// ==========================================
	// MARKETPLACE (PLAYER-TO-PLAYER TRADING)
	// ==========================================
	// BALANCE DECISION: Player economy should be vibrant but not exploitable

	constexpr float MarketplaceTaxPercent = 0.05f;    // 5% listing fee
	constexpr float MarketplaceSaleTaxPercent = 0.0f; // No sale tax - player friendly
	constexpr int32 MarketplaceMinREP = 100;          // Requires Rookie tier to trade
	constexpr int32 MarketplaceMaxListings = 10;      // Max concurrent listings
	constexpr float MarketplaceMinPricePercent = 0.5f;  // Min 50% of base value
	constexpr float MarketplaceMaxPricePercent = 2.0f;  // Max 200% of base value
	constexpr int32 MarketplaceListingDurationDays = 7; // Listings last 7 days

	// Trade Requirements
	constexpr int32 TradeMinLevel = 5;            // Must be level 5 to trade
	constexpr float TradeValueWarningThreshold = 0.5f; // Warn if value ratio < 50%

	// ==========================================
	// DAILY/WEEKLY BONUSES
	// ==========================================
	// BALANCE DECISION: Reward consistent play without punishing absence
	constexpr int64 DailyLoginBonus = 750;       // Raised from 500
	constexpr int64 DailyLoginStreak3 = 2000;    // Raised from 1500
	constexpr int64 DailyLoginStreak7 = 6000;    // Raised from 5000
	constexpr int64 DailyLoginStreak14 = 15000;  // Raised from 12000
	constexpr int64 DailyLoginStreak30 = 35000;  // Raised from 30000

	constexpr int64 DailyFirstRaceBonus = 500;   // NEW: Bonus for first race of day
	constexpr int64 DailyFirstWinBonus = 1500;   // NEW: Bonus for first win of day

	constexpr int64 WeeklyChallenge1 = 4000;     // Raised from 3000
	constexpr int64 WeeklyChallenge2 = 7000;     // Raised from 5000
	constexpr int64 WeeklyChallenge3 = 12000;    // Raised from 8000

	// ==========================================
	// CREW / REPUTATION REWARDS
	// ==========================================
	constexpr int64 CrewRaceBonus = 750;          // Racing with crew members (up from 500)
	constexpr int64 CrewWinBonus = 1500;          // Win with crew (up from 1000)
	constexpr int64 CrewLevelUpReward = 7500;     // Crew level up (up from 5000)
	constexpr int64 CrewWeeklyContribution = 3000; // NEW: Weekly crew contribution bonus

	// ==========================================
	// ACHIEVEMENT REWARDS
	// ==========================================
	constexpr int64 AchievementMinor = 1500;      // Easy achievements (up from 1000)
	constexpr int64 AchievementMedium = 6000;     // Medium difficulty (up from 5000)
	constexpr int64 AchievementMajor = 18000;     // Hard achievements (up from 15000)
	constexpr int64 AchievementLegendary = 60000; // Legendary achievements (up from 50000)

	// ==========================================
	// PROGRESSION MILESTONES
	// ==========================================
	// Cash rewards for hitting specific milestones

	constexpr int64 Milestone10Races = 6000;     // Up from 5000
	constexpr int64 Milestone25Races = 12000;    // Up from 10000
	constexpr int64 Milestone50Races = 30000;    // Up from 25000
	constexpr int64 Milestone100Races = 60000;   // Up from 50000
	constexpr int64 Milestone250Races = 125000;  // Up from 100000
	constexpr int64 Milestone500Races = 250000;  // NEW

	constexpr int64 MilestoneFirstWin = 3000;    // Up from 2500
	constexpr int64 Milestone10Wins = 12000;     // Up from 10000
	constexpr int64 Milestone50Wins = 40000;     // Up from 35000
	constexpr int64 Milestone100Wins = 85000;    // Up from 75000
	constexpr int64 Milestone200Wins = 175000;   // NEW

	// ==========================================
	// TOURNAMENT PRIZES
	// ==========================================
	constexpr int64 TournamentStreet1st = 30000;  // Up from 25000
	constexpr int64 TournamentStreet2nd = 18000;  // Up from 15000
	constexpr int64 TournamentStreet3rd = 10000;  // Up from 8000

	constexpr int64 TournamentPro1st = 85000;     // Up from 75000
	constexpr int64 TournamentPro2nd = 50000;     // Up from 45000
	constexpr int64 TournamentPro3rd = 30000;     // Up from 25000

	constexpr int64 TournamentChampionship1st = 250000;  // Up from 200000
	constexpr int64 TournamentChampionship2nd = 125000;  // Up from 100000
	constexpr int64 TournamentChampionship3rd = 60000;   // Up from 50000

	// ==========================================
	// XP VALUES (for level progression)
	// ==========================================
	constexpr int64 XPRaceFinish = 100;
	constexpr int64 XPRaceWin = 250;
	constexpr int64 XPRacePodium = 150;
	constexpr int64 XPOvertake = 25;
	constexpr int64 XPNearMiss = 10;
	constexpr int64 XPDriftPer1000 = 50;
	constexpr int64 XPCleanLap = 75;
	constexpr int64 XPBestLap = 100;
	constexpr int64 XPPerfectStart = 50;
	constexpr int64 XPRankedBonus = 75;          // NEW: Bonus XP for ranked races

	// Level XP requirements (exponential curve)
	inline int64 GetXPForLevel(int32 Level)
	{
		if (Level <= 1) return 0;
		// Base 1000 XP, scaling 1.12x per level (reduced from 1.15 for smoother curve)
		return static_cast<int64>(1000.0 * FMath::Pow(1.12, Level - 1));
	}

	// ==========================================
	// HELPER FUNCTIONS
	// ==========================================

	/**
	 * Get race reward by position and race type with risk scaling
	 */
	inline int64 GetRaceReward(int32 Position, int32 TotalRacers, const FName& RaceType, EMGPerformanceClass VehicleClass)
	{
		int64 BaseReward = 0;

		// Determine base by race type
		if (RaceType == TEXT("Sprint"))
		{
			switch (Position)
			{
				case 1: BaseReward = SprintBase1st; break;
				case 2: BaseReward = SprintBase2nd; break;
				case 3: BaseReward = SprintBase3rd; break;
				default: BaseReward = SprintBaseDNF + (TotalRacers - Position) * 100; break;
			}
		}
		else if (RaceType == TEXT("Circuit"))
		{
			switch (Position)
			{
				case 1: BaseReward = CircuitBase1st; break;
				case 2: BaseReward = CircuitBase2nd; break;
				case 3: BaseReward = CircuitBase3rd; break;
				default: BaseReward = CircuitBaseDNF + (TotalRacers - Position) * 150; break;
			}
		}
		else if (RaceType == TEXT("Drag"))
		{
			BaseReward = Position == 1 ? DragBase1st : (Position == 2 ? DragBase2nd : DragBaseDNF);
		}
		else if (RaceType == TEXT("Drift"))
		{
			switch (Position)
			{
				case 1: BaseReward = DriftBase1st; break;
				case 2: BaseReward = DriftBase2nd; break;
				case 3: BaseReward = DriftBase3rd; break;
				default: BaseReward = DriftBaseDNF + (TotalRacers - Position) * 100; break;
			}
		}
		else if (RaceType == TEXT("Touge"))
		{
			BaseReward = Position == 1 ? TougeBaseWin : TougeBaseLose;
		}
		else if (RaceType == TEXT("HighwayBattle"))
		{
			BaseReward = Position == 1 ? HighwayBattleWin : HighwayBattleLose;
		}
		else
		{
			// Default to Sprint values
			BaseReward = Position <= 3 ? SprintBase1st - (Position - 1) * 700 : SprintBaseDNF;
		}

		// Apply class multiplier (higher class = higher investment = higher rewards)
		float ClassMult = ClassBMultiplier;
		switch (VehicleClass)
		{
			case EMGPerformanceClass::D: ClassMult = ClassDMultiplier; break;
			case EMGPerformanceClass::C: ClassMult = ClassCMultiplier; break;
			case EMGPerformanceClass::B: ClassMult = ClassBMultiplier; break;
			case EMGPerformanceClass::A: ClassMult = ClassAMultiplier; break;
			case EMGPerformanceClass::S: ClassMult = ClassSMultiplier; break;
			case EMGPerformanceClass::X: ClassMult = ClassXMultiplier; break;
		}

		// Apply opponent count bonus (more opponents = more risk = more reward)
		float OpponentMult = OpponentBaseMultiplier;
		if (TotalRacers > 4)
		{
			OpponentMult += (TotalRacers - 4) * OpponentScalePerRacer;
		}

		return static_cast<int64>(BaseReward * ClassMult * OpponentMult);
	}

	/**
	 * Get race reward with all modifiers applied
	 */
	inline int64 GetModifiedRaceReward(
		int32 Position,
		int32 TotalRacers,
		const FName& RaceType,
		EMGPerformanceClass VehicleClass,
		bool bIsRanked,
		bool bIsNight,
		bool bIsWet,
		bool bHasTraffic,
		bool bHasCops,
		bool bCleanRace,
		bool bBestLap,
		bool bComeback,
		bool bUnderdog)
	{
		int64 BaseReward = GetRaceReward(Position, TotalRacers, RaceType, VehicleClass);
		float TotalMultiplier = 1.0f;

		// Risk multipliers (stack additively)
		if (bIsRanked) TotalMultiplier += (RankedRaceMultiplier - 1.0f);
		if (bIsNight) TotalMultiplier += (NightRaceMultiplier - 1.0f);
		if (bIsWet) TotalMultiplier += (WetConditionsMultiplier - 1.0f);
		if (bHasTraffic) TotalMultiplier += (TrafficEnabledMultiplier - 1.0f);
		if (bHasCops) TotalMultiplier += (CopsEnabledMultiplier - 1.0f);

		// Skill bonuses (only if won/podium)
		if (Position <= 3)
		{
			if (bCleanRace) TotalMultiplier += (CleanRaceBonus - 1.0f);
			if (bBestLap) TotalMultiplier += (BestLapBonus - 1.0f);
			if (Position == 1)
			{
				if (bComeback) TotalMultiplier += (ComebackBonus - 1.0f);
				if (bUnderdog) TotalMultiplier += (UnderDogBonus - 1.0f);
			}
		}

		return static_cast<int64>(BaseReward * TotalMultiplier);
	}

	/**
	 * Calculate vehicle sell price with depreciation
	 */
	inline int64 GetVehicleSellPrice(int64 MarketValue, int32 RaceCount, float Condition)
	{
		float DepreciationFactor = 1.0f - (RaceCount * VehicleDepreciationPerRace);
		DepreciationFactor = FMath::Clamp(DepreciationFactor, 0.5f, 1.0f);

		float ConditionFactor = Condition / 100.0f;

		return static_cast<int64>(MarketValue * VehicleSellBackPercent * DepreciationFactor * ConditionFactor);
	}

	/**
	 * Calculate repair cost based on vehicle value and damage level
	 */
	inline int64 GetRepairCost(int64 VehicleValue, float DamagePercent)
	{
		float RepairPercent = 0.0f;

		if (DamagePercent <= 0.1f)
			RepairPercent = RepairMinorDamagePercent;
		else if (DamagePercent <= 0.25f)
			RepairPercent = RepairModerateDamagePercent;
		else if (DamagePercent <= 0.5f)
			RepairPercent = RepairMajorDamagePercent;
		else
			RepairPercent = RepairTotalDamagePercent;

		return static_cast<int64>(VehicleValue * RepairPercent * DamagePercent);
	}

	/**
	 * Get daily login bonus based on streak
	 */
	inline int64 GetDailyLoginBonus(int32 StreakDays)
	{
		if (StreakDays >= 30) return DailyLoginStreak30;
		if (StreakDays >= 14) return DailyLoginStreak14;
		if (StreakDays >= 7) return DailyLoginStreak7;
		if (StreakDays >= 3) return DailyLoginStreak3;
		return DailyLoginBonus;
	}

	/**
	 * Get part price by tier (general formula)
	 */
	inline int64 GetPartPrice(EMGPartTier Tier, int64 BasePrice)
	{
		switch (Tier)
		{
			case EMGPartTier::Stock: return 0;
			case EMGPartTier::Street: return BasePrice;
			case EMGPartTier::Sport: return static_cast<int64>(BasePrice * 2.5f);
			case EMGPartTier::Race: return static_cast<int64>(BasePrice * 5.0f);
			case EMGPartTier::Pro: return static_cast<int64>(BasePrice * 10.0f);
			case EMGPartTier::Legendary: return static_cast<int64>(BasePrice * 25.0f);
			default: return BasePrice;
		}
	}

	/**
	 * Calculate estimated time to afford a vehicle (in races)
	 */
	inline int32 EstimateRacesToAfford(int64 VehiclePrice, EMGPerformanceClass CurrentClass)
	{
		// Average reward per race at current class (assuming ~3rd place average)
		int64 AverageReward = GetRaceReward(3, 8, FName(TEXT("Circuit")), CurrentClass);

		if (AverageReward <= 0) return 9999;

		return static_cast<int32>(FMath::CeilToInt(static_cast<float>(VehiclePrice) / AverageReward));
	}

	/**
	 * Calculate REP reward for a race
	 */
	inline int32 GetREPReward(const FName& RaceType, bool bWon, float WinMargin, bool bCleanRace, bool bComeback, int32 OpponentCount)
	{
		if (!bWon) return -REPRaceLossBase;

		int32 BaseREP = REPSprintWin; // Default

		if (RaceType == TEXT("Sprint")) BaseREP = REPSprintWin;
		else if (RaceType == TEXT("Circuit")) BaseREP = REPCircuitWin;
		else if (RaceType == TEXT("Drag")) BaseREP = REPDragWin;
		else if (RaceType == TEXT("Drift")) BaseREP = REPDriftWin;
		else if (RaceType == TEXT("Touge")) BaseREP = REPTougeWin;
		else if (RaceType == TEXT("HighwayBattle")) BaseREP = REPHighwayBattleWin;
		else if (RaceType == TEXT("PinkSlip")) BaseREP = REPPinkSlipWin;

		float Multiplier = 1.0f;

		// Dominant win bonus
		if (WinMargin >= 10.0f) Multiplier *= REPDominantWinMultiplier;

		// Comeback bonus
		if (bComeback) Multiplier *= REPComebackMultiplier;

		// Clean race bonus
		if (bCleanRace) Multiplier *= REPCleanRaceMultiplier;

		// Opponent count bonus
		Multiplier += (OpponentCount - 1) * REPOpponentCountMultiplier;

		return static_cast<int32>(BaseREP * Multiplier);
	}

	/**
	 * Check if player meets marketplace requirements
	 */
	inline bool CanUseMarketplace(int32 PlayerREP, int32 PlayerLevel)
	{
		return PlayerREP >= MarketplaceMinREP && PlayerLevel >= TradeMinLevel;
	}

	/**
	 * Validate marketplace listing price
	 */
	inline bool IsValidMarketplacePrice(int64 ListingPrice, int64 BaseValue)
	{
		float MinPrice = BaseValue * MarketplaceMinPricePercent;
		float MaxPrice = BaseValue * MarketplaceMaxPricePercent;
		return ListingPrice >= MinPrice && ListingPrice <= MaxPrice;
	}
}
