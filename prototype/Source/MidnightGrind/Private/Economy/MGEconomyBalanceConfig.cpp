// Copyright Midnight Grind. All Rights Reserved.
// Stage 50: Economy Balance Configuration - Tuned for 27-vehicle fleet

#include "Economy/MGEconomySubsystem.h"
#include "Balancing/MGBalancingSubsystem.h"

namespace MGEconomyBalance
{
	// ==========================================
	// CORE BALANCE PHILOSOPHY
	// ==========================================
	// Target: 40 hours to endgame (own a hypercar)
	// Starter -> Mid-Tier: ~8 hours (15-20 races)
	// Mid-Tier -> High-End: ~15 hours (40-50 races)
	// High-End -> Exotic: ~17 hours (50-60 races)
	// Average race duration: 3-5 minutes
	// Average session: 45 minutes (8-12 races)

	// ==========================================
	// STARTING ECONOMY
	// ==========================================
	constexpr int64 StartingCash = 5000;           // Enough for basic mods on starter car
	constexpr int64 StarterVehicleValue = 0;       // Civic Type R, Focus SVT, Corrado VR6 are free

	// ==========================================
	// RACE TYPE BASE REWARDS
	// ==========================================
	// Balanced for ~$2,500 average per race at mid-game

	// Sprint Races (2-3 laps, ~3 minutes)
	constexpr int64 SprintBase1st = 3500;
	constexpr int64 SprintBase2nd = 2100;
	constexpr int64 SprintBase3rd = 1400;
	constexpr int64 SprintBaseDNF = 350;

	// Circuit Races (3-5 laps, ~5 minutes)
	constexpr int64 CircuitBase1st = 5000;
	constexpr int64 CircuitBase2nd = 3000;
	constexpr int64 CircuitBase3rd = 2000;
	constexpr int64 CircuitBaseDNF = 500;

	// Drag Races (quick, high stakes)
	constexpr int64 DragBase1st = 2500;
	constexpr int64 DragBase2nd = 1000;
	constexpr int64 DragBaseDNF = 100;

	// Drift Events (score-based)
	constexpr int64 DriftBase1st = 4000;
	constexpr int64 DriftBase2nd = 2400;
	constexpr int64 DriftBase3rd = 1600;
	constexpr int64 DriftBaseDNF = 400;

	// Touge/Canyon Races (1v1, high skill)
	constexpr int64 TougeBaseWin = 4500;
	constexpr int64 TougeBaseLose = 500;

	// Time Attack (solo, beat ghost/time)
	constexpr int64 TimeAttackGoldReward = 3000;
	constexpr int64 TimeAttackSilverReward = 1500;
	constexpr int64 TimeAttackBronzeReward = 750;

	// ==========================================
	// RACE CLASS MULTIPLIERS
	// ==========================================
	// Higher class races pay more
	constexpr float ClassDMultiplier = 0.6f;    // Starter tier
	constexpr float ClassCMultiplier = 0.8f;    // Entry level
	constexpr float ClassBMultiplier = 1.0f;    // Standard
	constexpr float ClassAMultiplier = 1.3f;    // Competitive
	constexpr float ClassSMultiplier = 1.7f;    // High stakes
	constexpr float ClassXMultiplier = 2.5f;    // Elite/Hypercar

	// ==========================================
	// BONUS MULTIPLIERS
	// ==========================================
	constexpr float CleanRaceBonus = 1.15f;      // No collisions
	constexpr float PerfectStartBonus = 1.05f;   // Perfect launch
	constexpr float BestLapBonus = 1.10f;        // Set fastest lap
	constexpr float ComebackBonus = 1.20f;       // Win from last
	constexpr float FlawlessVictoryBonus = 1.25f; // 1st + clean + best lap
	constexpr float UnderDogBonus = 1.15f;       // Win with lower PI
	constexpr float RivalryBonus = 1.10f;        // Beat a rival

	// ==========================================
	// WAGER SYSTEM
	// ==========================================
	constexpr int64 MinWager = 500;
	constexpr int64 MaxWagerMultiplier = 5;      // Up to 5x base race reward
	constexpr float WagerWinMultiplier = 2.0f;   // Double your money
	constexpr float WagerHouseEdge = 0.05f;      // 5% house edge

	// ==========================================
	// PINK SLIP RACES
	// ==========================================
	// Winner takes loser's car - high stakes, can lose everything
	constexpr float PinkSlipMinimumValue = 15000.0f;  // Minimum car value
	constexpr float PinkSlipValueMatchTolerance = 0.3f; // 30% value difference max

	// ==========================================
	// VEHICLE PRICING TIERS
	// ==========================================
	// Based on real-world values and game progression

	// Tier 1: Starter (Free choices)
	// Honda Civic Type R EK9: $0 (Free starter)
	// Ford Focus SVT: $0 (Free starter)
	// VW Corrado VR6: $0 (Free starter)

	// Tier 2: Entry Level ($12,000 - $35,000) - 5-8 hours playtime
	// Nissan 350Z: $22,000
	// VW Golf R32: $28,000
	// Audi S4 B5: $25,000
	// Mercedes C32 AMG: $35,000
	// Camaro SS: $35,000
	// Firebird Trans Am WS6: $38,000

	// Tier 3: Mid-Range ($35,000 - $55,000) - 15-20 hours
	// Nissan Silvia S15: $45,000
	// Honda S2000: $42,000
	// Acura Integra Type R: $55,000
	// BMW M3 E46: $55,000
	// Lotus Elise S1: $45,000

	// Tier 4: Performance ($55,000 - $95,000) - 20-30 hours
	// Subaru Impreza STI: $55,000
	// Mitsubishi Evo VI: $65,000
	// Corvette Z06: $65,000
	// Mazda RX-7 FD: $85,000
	// Dodge Viper GTS: $95,000

	// Tier 5: High-End ($95,000 - $180,000) - 30-40 hours
	// Ferrari 360 Modena: $110,000
	// Porsche 911 GT3: $120,000
	// Nissan Skyline R34: $120,000
	// Ford Mustang Cobra R: $125,000
	// Toyota Supra MK4: $150,000
	// Honda NSX Type R: $180,000

	// Tier 6: Exotic/Legendary ($250,000+) - 40+ hours
	// Lamborghini Diablo SV: $350,000
	// McLaren F1: $25,000,000 (ultimate goal)

	// ==========================================
	// TUNING PART PRICES BY TIER
	// ==========================================

	// AIR INTAKE SYSTEM
	constexpr int64 AirIntakeStock = 0;
	constexpr int64 AirIntakeStreet = 250;       // +5-8 HP
	constexpr int64 AirIntakeSport = 600;        // +10-15 HP
	constexpr int64 AirIntakeRace = 1200;        // +15-25 HP
	constexpr int64 AirIntakePro = 2500;         // +25-40 HP

	// EXHAUST SYSTEM
	constexpr int64 ExhaustStock = 0;
	constexpr int64 ExhaustStreet = 400;         // Cat-back, +5-10 HP
	constexpr int64 ExhaustSport = 900;          // High-flow, +10-20 HP
	constexpr int64 ExhaustRace = 1800;          // Headers + high-flow, +20-35 HP
	constexpr int64 ExhaustPro = 3500;           // Full race system, +35-50 HP

	// ECU / TUNING
	constexpr int64 ECUStock = 0;
	constexpr int64 ECUStreet = 350;             // +3-5% power
	constexpr int64 ECUSport = 800;              // +5-8% power
	constexpr int64 ECURace = 1500;              // +8-12% power
	constexpr int64 ECUPro = 3000;               // +12-18% power, custom tune
	constexpr int64 ECUStandalone = 5000;        // Full standalone, unlimited

	// FORCED INDUCTION - TURBO
	constexpr int64 TurboSmall = 2500;           // +50-80 HP
	constexpr int64 TurboMedium = 4500;          // +80-150 HP
	constexpr int64 TurboLarge = 7500;           // +150-250 HP
	constexpr int64 TurboMassive = 12000;        // +250-400 HP
	constexpr int64 TwinTurboKit = 18000;        // Specialized kit

	// FORCED INDUCTION - SUPERCHARGER
	constexpr int64 SuperchargerRoots = 4000;    // +80-120 HP
	constexpr int64 SuperchargerTwinScrew = 6000; // +100-180 HP
	constexpr int64 SuperchargerCentrifugal = 5000; // +80-150 HP

	// ENGINE INTERNALS
	constexpr int64 CamshaftStreet = 500;
	constexpr int64 CamshaftSport = 1200;
	constexpr int64 CamshaftRace = 2500;

	constexpr int64 InternalsStreet = 1500;      // Pistons, rods
	constexpr int64 InternalsSport = 3500;
	constexpr int64 InternalsRace = 6000;
	constexpr int64 InternalsForged = 10000;     // Forged rotating assembly

	// ENGINE SWAP (complete replacement)
	constexpr int64 EngineSwapBase = 15000;      // Base swap labor
	constexpr int64 EngineSwapPremium = 35000;   // Premium engine

	// TRANSMISSION
	constexpr int64 TransStreet = 1000;
	constexpr int64 TransSport = 2500;           // Short throw, upgraded synchros
	constexpr int64 TransRace = 5000;            // Sequential or dog box
	constexpr int64 TransClutch = 800;           // Clutch upgrade
	constexpr int64 TransClutchRace = 2000;      // Multi-plate racing clutch
	constexpr int64 TransFinalDrive = 1500;      // Final drive change
	constexpr int64 TransLSD = 2000;             // Limited slip differential

	// SUSPENSION
	constexpr int64 SuspStreet = 600;            // Lowering springs
	constexpr int64 SuspSport = 1500;            // Coilovers (adjustable)
	constexpr int64 SuspRace = 3500;             // Full race coilovers
	constexpr int64 SuspPro = 6000;              // Multi-adjustable

	constexpr int64 SwayBarFront = 300;
	constexpr int64 SwayBarRear = 300;
	constexpr int64 SwayBarKit = 500;            // Front + rear combo

	constexpr int64 AlignmentKit = 200;          // Adjustable arms
	constexpr int64 CageRollBar = 1500;          // Roll bar
	constexpr int64 CageRollCage = 4000;         // Full roll cage

	// BRAKES
	constexpr int64 BrakePadsStreet = 200;
	constexpr int64 BrakePadsSport = 400;
	constexpr int64 BrakePadsRace = 800;

	constexpr int64 BrakeRotorsSlotted = 350;
	constexpr int64 BrakeRotorsDrilled = 400;
	constexpr int64 BrakeRotors2Piece = 800;

	constexpr int64 BrakeCaliperUpgrade = 1200;
	constexpr int64 BrakeBigBrakeKit = 3500;     // Full BBK

	constexpr int64 BrakeLines = 150;            // Stainless lines

	// WHEELS & TIRES
	constexpr int64 WheelsStreet = 800;          // 17-18"
	constexpr int64 WheelsSport = 1500;          // Lightweight
	constexpr int64 WheelsRace = 3000;           // Forged lightweight
	constexpr int64 WheelsDrag = 2000;           // Drag skinnies + slicks

	constexpr int64 TiresAllSeason = 300;
	constexpr int64 TiresSport = 500;
	constexpr int64 TiresPerformance = 800;
	constexpr int64 TiresSemiSlick = 1200;
	constexpr int64 TiresSlick = 2000;           // Track only
	constexpr int64 TiresDrag = 1500;

	// AERODYNAMICS
	constexpr int64 AeroFrontLip = 400;
	constexpr int64 AeroSplitter = 800;
	constexpr int64 AeroSplitterRace = 1500;

	constexpr int64 AeroRearSpoiler = 500;
	constexpr int64 AeroRearWing = 1200;
	constexpr int64 AeroRearWingRace = 2500;

	constexpr int64 AeroDiffuser = 1000;
	constexpr int64 AeroWideBody = 5000;         // Wide body kit

	// NITROUS OXIDE SYSTEMS
	constexpr int64 NOS50Shot = 1500;            // +50 HP
	constexpr int64 NOS75Shot = 2000;            // +75 HP
	constexpr int64 NOS100Shot = 2500;           // +100 HP
	constexpr int64 NOS150Shot = 3500;           // +150 HP
	constexpr int64 NOS200Shot = 5000;           // +200 HP
	constexpr int64 NOSDirectPort = 8000;        // Direct port system
	constexpr int64 NOSRefill = 50;              // Per lb

	// WEIGHT REDUCTION
	constexpr int64 WeightStage1 = 500;          // Remove spare, mats (-20 lbs)
	constexpr int64 WeightStage2 = 1500;         // Lightweight seats, panels (-50 lbs)
	constexpr int64 WeightStage3 = 3500;         // Full strip, carbon panels (-100 lbs)
	constexpr int64 WeightStage4 = 7000;         // Full race prep (-150+ lbs)

	// ==========================================
	// COSMETIC PRICING
	// ==========================================

	// PAINT
	constexpr int64 PaintSolid = 500;
	constexpr int64 PaintMetallic = 1000;
	constexpr int64 PaintPearlescent = 1500;
	constexpr int64 PaintMatte = 2000;
	constexpr int64 PaintChrome = 3000;
	constexpr int64 PaintCustom = 5000;

	// VINYL / WRAP
	constexpr int64 VinylBasic = 200;
	constexpr int64 VinylPremium = 500;
	constexpr int64 WrapFull = 3000;
	constexpr int64 WrapPremium = 6000;

	// BODY KITS
	constexpr int64 BodyKitFrontBumper = 800;
	constexpr int64 BodyKitRearBumper = 600;
	constexpr int64 BodyKitSideSkirts = 500;
	constexpr int64 BodyKitFull = 2500;
	constexpr int64 BodyKitPremium = 5000;

	// INTERIOR
	constexpr int64 InteriorSeats = 1200;
	constexpr int64 InteriorSteeringWheel = 400;
	constexpr int64 InteriorShiftKnob = 150;
	constexpr int64 InteriorGauges = 600;

	// LIGHTING
	constexpr int64 LightsHeadlights = 600;
	constexpr int64 LightsTaillights = 400;
	constexpr int64 LightsNeon = 800;            // Underglow

	// ==========================================
	// REPAIR COSTS
	// ==========================================
	// Based on percentage of vehicle value
	constexpr float RepairMinorDamagePercent = 0.005f;  // 0.5%
	constexpr float RepairModerateDamagePercent = 0.02f; // 2%
	constexpr float RepairMajorDamagePercent = 0.05f;   // 5%
	constexpr float RepairTotalDamagePercent = 0.15f;   // 15%

	// ==========================================
	// SELL BACK VALUES
	// ==========================================
	constexpr float PartSellBackPercent = 0.5f;       // 50% of purchase price
	constexpr float VehicleSellBackPercent = 0.7f;    // 70% of market value
	constexpr float VehicleDepreciationPerRace = 0.001f; // 0.1% per race

	// ==========================================
	// DAILY/WEEKLY BONUSES
	// ==========================================
	constexpr int64 DailyLoginBonus = 500;
	constexpr int64 DailyLoginStreak3 = 1500;
	constexpr int64 DailyLoginStreak7 = 5000;
	constexpr int64 DailyLoginStreak14 = 12000;
	constexpr int64 DailyLoginStreak30 = 30000;

	constexpr int64 WeeklyChallenge1 = 3000;
	constexpr int64 WeeklyChallenge2 = 5000;
	constexpr int64 WeeklyChallenge3 = 8000;

	// ==========================================
	// CREW / REPUTATION REWARDS
	// ==========================================
	constexpr int64 CrewRaceBonus = 500;          // Racing with crew members
	constexpr int64 CrewWinBonus = 1000;          // Win with crew
	constexpr int64 CrewLevelUpReward = 5000;     // Crew level up

	// ==========================================
	// ACHIEVEMENT REWARDS
	// ==========================================
	constexpr int64 AchievementMinor = 1000;      // Easy achievements
	constexpr int64 AchievementMedium = 5000;     // Medium difficulty
	constexpr int64 AchievementMajor = 15000;     // Hard achievements
	constexpr int64 AchievementLegendary = 50000; // Legendary achievements

	// ==========================================
	// PROGRESSION MILESTONES
	// ==========================================
	// Cash rewards for hitting specific milestones

	constexpr int64 Milestone10Races = 5000;
	constexpr int64 Milestone25Races = 10000;
	constexpr int64 Milestone50Races = 25000;
	constexpr int64 Milestone100Races = 50000;
	constexpr int64 Milestone250Races = 100000;

	constexpr int64 MilestoneFirstWin = 2500;
	constexpr int64 Milestone10Wins = 10000;
	constexpr int64 Milestone50Wins = 35000;
	constexpr int64 Milestone100Wins = 75000;

	// ==========================================
	// TOURNAMENT PRIZES
	// ==========================================
	constexpr int64 TournamentStreet1st = 25000;
	constexpr int64 TournamentStreet2nd = 15000;
	constexpr int64 TournamentStreet3rd = 8000;

	constexpr int64 TournamentPro1st = 75000;
	constexpr int64 TournamentPro2nd = 45000;
	constexpr int64 TournamentPro3rd = 25000;

	constexpr int64 TournamentChampionship1st = 200000;
	constexpr int64 TournamentChampionship2nd = 100000;
	constexpr int64 TournamentChampionship3rd = 50000;

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

	// Level XP requirements (exponential curve)
	inline int64 GetXPForLevel(int32 Level)
	{
		if (Level <= 1) return 0;
		// Base 1000 XP, scaling 1.15x per level
		return static_cast<int64>(1000.0 * FMath::Pow(1.15, Level - 1));
	}

	// ==========================================
	// HELPER FUNCTIONS
	// ==========================================

	/**
	 * Get race reward by position and race type
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
		else
		{
			// Default to Sprint values
			BaseReward = Position <= 3 ? SprintBase1st - (Position - 1) * 700 : SprintBaseDNF;
		}

		// Apply class multiplier
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

		return static_cast<int64>(BaseReward * ClassMult);
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
		// Average reward per race at current class
		int64 AverageReward = GetRaceReward(3, 8, FName(TEXT("Circuit")), CurrentClass); // Average ~3rd place

		if (AverageReward <= 0) return 9999;

		return static_cast<int32>(FMath::CeilToInt(static_cast<float>(VehiclePrice) / AverageReward));
	}
}
