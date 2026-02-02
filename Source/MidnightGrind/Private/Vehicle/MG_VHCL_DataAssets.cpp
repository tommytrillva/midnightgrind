// Copyright Midnight Grind. All Rights Reserved.
// Stage 49: Vehicle Asset Infrastructure - Authentic Racing Fleet

#include "Vehicle/MG_VHCL_Data.h"
#include "Engine/DataTable.h"

// Authentic vehicle configurations from the golden era of street racing
// Late 90s / Early 2000s - The Midnight Club era

namespace MGVehicleDefaults
{
	// ==========================================
	// JDM LEGENDS - JAPANESE DOMESTIC MARKET
	// ==========================================

	FMGVehicleModelConfig GetNissanSkylineR34Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("NISSAN_SKYLINE_R34"));
		Config.DisplayName = FText::FromString(TEXT("Nissan Skyline GT-R V-Spec II"));
		Config.Manufacturer = FText::FromString(TEXT("Nissan"));
		Config.Year = 1999;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::AWD;

		// RB26DETT specs
		Config.BaseHorsePower = 280.0f; // Underrated from factory
		Config.BaseTorque = 293.0f;
		Config.BaseWeight = 1560.0f;
		Config.DisplacementCC = 2568;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::TwinTurbo;

		Config.TopSpeedMPH = 165.0f;
		Config.ZeroToSixty = 4.8f;
		Config.BasePI = 680;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 48000;
		Config.MarketValue = 120000;

		Config.MaxHorsePower = 1500.0f; // RB26 legend
		Config.TuningDifficulty = 0.6f;

		return Config;
	}

	FMGVehicleModelConfig GetToyotaSupraMK4Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("TOYOTA_SUPRA_MK4"));
		Config.DisplayName = FText::FromString(TEXT("Toyota Supra RZ"));
		Config.Manufacturer = FText::FromString(TEXT("Toyota"));
		Config.Year = 1998;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 2JZ-GTE specs
		Config.BaseHorsePower = 320.0f;
		Config.BaseTorque = 315.0f;
		Config.BaseWeight = 1510.0f;
		Config.DisplacementCC = 2997;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::TwinTurbo;

		Config.TopSpeedMPH = 177.0f;
		Config.ZeroToSixty = 4.6f;
		Config.BasePI = 700;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 45000;
		Config.MarketValue = 150000;

		Config.MaxHorsePower = 2000.0f; // 2JZ builds insane power
		Config.TuningDifficulty = 0.55f;

		return Config;
	}

	FMGVehicleModelConfig GetMazdaRX7FDConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("MAZDA_RX7_FD"));
		Config.DisplayName = FText::FromString(TEXT("Mazda RX-7 Spirit R"));
		Config.Manufacturer = FText::FromString(TEXT("Mazda"));
		Config.Year = 2002;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 13B-REW rotary
		Config.BaseHorsePower = 280.0f;
		Config.BaseTorque = 231.0f;
		Config.BaseWeight = 1280.0f;
		Config.DisplacementCC = 1308;
		Config.Cylinders = 2; // Rotors
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::SequentialTurbo;

		Config.TopSpeedMPH = 168.0f;
		Config.ZeroToSixty = 4.9f;
		Config.BasePI = 660;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 38000;
		Config.MarketValue = 85000;

		Config.MaxHorsePower = 900.0f;
		Config.TuningDifficulty = 0.75f; // Rotary expertise needed

		return Config;
	}

	FMGVehicleModelConfig GetHondaNSXConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("HONDA_NSX_NA1"));
		Config.DisplayName = FText::FromString(TEXT("Honda NSX Type R"));
		Config.Manufacturer = FText::FromString(TEXT("Honda"));
		Config.Year = 1999;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// C30A VTEC
		Config.BaseHorsePower = 290.0f;
		Config.BaseTorque = 224.0f;
		Config.BaseWeight = 1230.0f;
		Config.DisplacementCC = 3179;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Mid;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 175.0f;
		Config.ZeroToSixty = 4.5f;
		Config.BasePI = 720;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 89000;
		Config.MarketValue = 180000;

		Config.MaxHorsePower = 550.0f;
		Config.TuningDifficulty = 0.7f;

		return Config;
	}

	FMGVehicleModelConfig GetMitsubishiEvoVIConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("MITSUBISHI_EVO_VI"));
		Config.DisplayName = FText::FromString(TEXT("Mitsubishi Lancer Evolution VI TME"));
		Config.Manufacturer = FText::FromString(TEXT("Mitsubishi"));
		Config.Year = 2000;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Sedan;
		Config.DrivetrainType = EMGDrivetrainType::AWD;

		// 4G63T
		Config.BaseHorsePower = 280.0f;
		Config.BaseTorque = 275.0f;
		Config.BaseWeight = 1360.0f;
		Config.DisplacementCC = 1997;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::Turbocharged;

		Config.TopSpeedMPH = 157.0f;
		Config.ZeroToSixty = 4.4f;
		Config.BasePI = 670;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 35000;
		Config.MarketValue = 65000;

		Config.MaxHorsePower = 1000.0f;
		Config.TuningDifficulty = 0.5f;

		return Config;
	}

	FMGVehicleModelConfig GetSubaruImprezaSTIConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("SUBARU_IMPREZA_STI"));
		Config.DisplayName = FText::FromString(TEXT("Subaru Impreza WRX STI"));
		Config.Manufacturer = FText::FromString(TEXT("Subaru"));
		Config.Year = 2000;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Sedan;
		Config.DrivetrainType = EMGDrivetrainType::AWD;

		// EJ207
		Config.BaseHorsePower = 280.0f;
		Config.BaseTorque = 260.0f;
		Config.BaseWeight = 1395.0f;
		Config.DisplacementCC = 1994;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::Turbocharged;

		Config.TopSpeedMPH = 158.0f;
		Config.ZeroToSixty = 4.7f;
		Config.BasePI = 665;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 32000;
		Config.MarketValue = 55000;

		Config.MaxHorsePower = 800.0f;
		Config.TuningDifficulty = 0.5f;

		return Config;
	}

	FMGVehicleModelConfig GetNissanSilviaS15Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("NISSAN_SILVIA_S15"));
		Config.DisplayName = FText::FromString(TEXT("Nissan Silvia Spec-R"));
		Config.Manufacturer = FText::FromString(TEXT("Nissan"));
		Config.Year = 1999;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// SR20DET
		Config.BaseHorsePower = 250.0f;
		Config.BaseTorque = 203.0f;
		Config.BaseWeight = 1240.0f;
		Config.DisplacementCC = 1998;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::Turbocharged;

		Config.TopSpeedMPH = 155.0f;
		Config.ZeroToSixty = 5.5f;
		Config.BasePI = 590;
		Config.BaseClass = EMGPerformanceClass::B;

		Config.BasePriceMSRP = 28000;
		Config.MarketValue = 45000;

		Config.MaxHorsePower = 700.0f;
		Config.TuningDifficulty = 0.45f;

		return Config;
	}

	FMGVehicleModelConfig GetHondaS2000Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("HONDA_S2000_AP1"));
		Config.DisplayName = FText::FromString(TEXT("Honda S2000"));
		Config.Manufacturer = FText::FromString(TEXT("Honda"));
		Config.Year = 1999;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Convertible;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// F20C - 9000 RPM screamer
		Config.BaseHorsePower = 240.0f;
		Config.BaseTorque = 153.0f;
		Config.BaseWeight = 1250.0f;
		Config.DisplacementCC = 1997;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 150.0f;
		Config.ZeroToSixty = 5.8f;
		Config.BasePI = 580;
		Config.BaseClass = EMGPerformanceClass::B;

		Config.BasePriceMSRP = 32000;
		Config.MarketValue = 42000;

		Config.MaxHorsePower = 450.0f;
		Config.TuningDifficulty = 0.6f;

		return Config;
	}

	// ==========================================
	// AMERICAN MUSCLE
	// ==========================================

	FMGVehicleModelConfig GetFordMustangCobraRConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("FORD_MUSTANG_COBRA_R"));
		Config.DisplayName = FText::FromString(TEXT("Ford Mustang SVT Cobra R"));
		Config.Manufacturer = FText::FromString(TEXT("Ford"));
		Config.Year = 2000;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Muscle;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 5.4L DOHC V8
		Config.BaseHorsePower = 385.0f;
		Config.BaseTorque = 385.0f;
		Config.BaseWeight = 1590.0f;
		Config.DisplacementCC = 5408;
		Config.Cylinders = 8;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 170.0f;
		Config.ZeroToSixty = 4.6f;
		Config.BasePI = 710;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 55000;
		Config.MarketValue = 125000;

		Config.MaxHorsePower = 800.0f;
		Config.TuningDifficulty = 0.45f;

		return Config;
	}

	FMGVehicleModelConfig GetChevroletCamaroSSConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("CHEVROLET_CAMARO_SS"));
		Config.DisplayName = FText::FromString(TEXT("Chevrolet Camaro SS"));
		Config.Manufacturer = FText::FromString(TEXT("Chevrolet"));
		Config.Year = 2002;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Muscle;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// LS1 V8
		Config.BaseHorsePower = 325.0f;
		Config.BaseTorque = 350.0f;
		Config.BaseWeight = 1545.0f;
		Config.DisplacementCC = 5700;
		Config.Cylinders = 8;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 161.0f;
		Config.ZeroToSixty = 5.0f;
		Config.BasePI = 650;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 28000;
		Config.MarketValue = 35000;

		Config.MaxHorsePower = 900.0f;
		Config.TuningDifficulty = 0.4f;

		return Config;
	}

	FMGVehicleModelConfig GetDodgeViperGTSConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("DODGE_VIPER_GTS"));
		Config.DisplayName = FText::FromString(TEXT("Dodge Viper GTS"));
		Config.Manufacturer = FText::FromString(TEXT("Dodge"));
		Config.Year = 1999;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Supercar;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 8.0L V10
		Config.BaseHorsePower = 450.0f;
		Config.BaseTorque = 490.0f;
		Config.BaseWeight = 1531.0f;
		Config.DisplacementCC = 7990;
		Config.Cylinders = 10;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 185.0f;
		Config.ZeroToSixty = 4.0f;
		Config.BasePI = 790;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 66000;
		Config.MarketValue = 95000;

		Config.MaxHorsePower = 1200.0f;
		Config.TuningDifficulty = 0.6f;

		return Config;
	}

	FMGVehicleModelConfig GetChevroletCorvetteZ06Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("CHEVROLET_CORVETTE_Z06"));
		Config.DisplayName = FText::FromString(TEXT("Chevrolet Corvette Z06"));
		Config.Manufacturer = FText::FromString(TEXT("Chevrolet"));
		Config.Year = 2002;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// LS6 V8
		Config.BaseHorsePower = 405.0f;
		Config.BaseTorque = 400.0f;
		Config.BaseWeight = 1418.0f;
		Config.DisplacementCC = 5665;
		Config.Cylinders = 8;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 171.0f;
		Config.ZeroToSixty = 3.9f;
		Config.BasePI = 760;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 50000;
		Config.MarketValue = 65000;

		Config.MaxHorsePower = 950.0f;
		Config.TuningDifficulty = 0.45f;

		return Config;
	}

	FMGVehicleModelConfig GetPontiacFirebirdTransAmConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("PONTIAC_FIREBIRD_WS6"));
		Config.DisplayName = FText::FromString(TEXT("Pontiac Firebird Trans Am WS6"));
		Config.Manufacturer = FText::FromString(TEXT("Pontiac"));
		Config.Year = 2002;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Muscle;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// LS1 V8 with Ram Air
		Config.BaseHorsePower = 325.0f;
		Config.BaseTorque = 350.0f;
		Config.BaseWeight = 1560.0f;
		Config.DisplacementCC = 5700;
		Config.Cylinders = 8;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 160.0f;
		Config.ZeroToSixty = 5.1f;
		Config.BasePI = 640;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 32000;
		Config.MarketValue = 38000;

		Config.MaxHorsePower = 900.0f;
		Config.TuningDifficulty = 0.4f;

		return Config;
	}

	// ==========================================
	// EUROPEAN PERFORMANCE
	// ==========================================

	FMGVehicleModelConfig GetBMWM3E46Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("BMW_M3_E46"));
		Config.DisplayName = FText::FromString(TEXT("BMW M3 E46"));
		Config.Manufacturer = FText::FromString(TEXT("BMW"));
		Config.Year = 2001;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// S54B32 inline-6
		Config.BaseHorsePower = 333.0f;
		Config.BaseTorque = 262.0f;
		Config.BaseWeight = 1570.0f;
		Config.DisplacementCC = 3246;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 155.0f; // Limited
		Config.ZeroToSixty = 4.8f;
		Config.BasePI = 670;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 50000;
		Config.MarketValue = 55000;

		Config.MaxHorsePower = 600.0f;
		Config.TuningDifficulty = 0.55f;

		return Config;
	}

	FMGVehicleModelConfig GetPorsche911GT3Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("PORSCHE_911_GT3_996"));
		Config.DisplayName = FText::FromString(TEXT("Porsche 911 GT3"));
		Config.Manufacturer = FText::FromString(TEXT("Porsche"));
		Config.Year = 1999;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 3.6L Flat-6
		Config.BaseHorsePower = 360.0f;
		Config.BaseTorque = 273.0f;
		Config.BaseWeight = 1350.0f;
		Config.DisplacementCC = 3600;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Rear;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 188.0f;
		Config.ZeroToSixty = 4.3f;
		Config.BasePI = 750;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 100000;
		Config.MarketValue = 120000;

		Config.MaxHorsePower = 550.0f;
		Config.TuningDifficulty = 0.65f;

		return Config;
	}

	FMGVehicleModelConfig GetMercedesAMGC32Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("MERCEDES_C32_AMG"));
		Config.DisplayName = FText::FromString(TEXT("Mercedes-Benz C32 AMG"));
		Config.Manufacturer = FText::FromString(TEXT("Mercedes-Benz"));
		Config.Year = 2002;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Sedan;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// Supercharged 3.2L V6
		Config.BaseHorsePower = 349.0f;
		Config.BaseTorque = 332.0f;
		Config.BaseWeight = 1635.0f;
		Config.DisplacementCC = 3199;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::Supercharged;

		Config.TopSpeedMPH = 155.0f; // Limited
		Config.ZeroToSixty = 5.0f;
		Config.BasePI = 650;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 52000;
		Config.MarketValue = 35000;

		Config.MaxHorsePower = 550.0f;
		Config.TuningDifficulty = 0.6f;

		return Config;
	}

	FMGVehicleModelConfig GetAudiS4B5Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("AUDI_S4_B5"));
		Config.DisplayName = FText::FromString(TEXT("Audi S4 quattro"));
		Config.Manufacturer = FText::FromString(TEXT("Audi"));
		Config.Year = 2000;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Sedan;
		Config.DrivetrainType = EMGDrivetrainType::AWD;

		// 2.7L Twin-Turbo V6
		Config.BaseHorsePower = 250.0f;
		Config.BaseTorque = 258.0f;
		Config.BaseWeight = 1695.0f;
		Config.DisplacementCC = 2671;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::TwinTurbo;

		Config.TopSpeedMPH = 155.0f; // Limited
		Config.ZeroToSixty = 5.6f;
		Config.BasePI = 600;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 42000;
		Config.MarketValue = 25000;

		Config.MaxHorsePower = 700.0f;
		Config.TuningDifficulty = 0.5f;

		return Config;
	}

	FMGVehicleModelConfig GetLotusEliseS1Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("LOTUS_ELISE_S1"));
		Config.DisplayName = FText::FromString(TEXT("Lotus Elise Sport 190"));
		Config.Manufacturer = FText::FromString(TEXT("Lotus"));
		Config.Year = 1999;
		Config.Country = TEXT("UK");
		Config.VehicleType = EMGVehicleType::Convertible;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// Rover K-Series
		Config.BaseHorsePower = 190.0f;
		Config.BaseTorque = 128.0f;
		Config.BaseWeight = 725.0f; // Incredibly light
		Config.DisplacementCC = 1796;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Mid;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 140.0f;
		Config.ZeroToSixty = 4.7f;
		Config.BasePI = 620;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 38000;
		Config.MarketValue = 45000;

		Config.MaxHorsePower = 350.0f;
		Config.TuningDifficulty = 0.65f;

		return Config;
	}

	FMGVehicleModelConfig GetVWGolfR32Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("VW_GOLF_R32_MK4"));
		Config.DisplayName = FText::FromString(TEXT("Volkswagen Golf R32"));
		Config.Manufacturer = FText::FromString(TEXT("Volkswagen"));
		Config.Year = 2002;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Hatchback;
		Config.DrivetrainType = EMGDrivetrainType::AWD;

		// 3.2L VR6
		Config.BaseHorsePower = 240.0f;
		Config.BaseTorque = 236.0f;
		Config.BaseWeight = 1477.0f;
		Config.DisplacementCC = 3189;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 153.0f;
		Config.ZeroToSixty = 6.4f;
		Config.BasePI = 560;
		Config.BaseClass = EMGPerformanceClass::B;

		Config.BasePriceMSRP = 32000;
		Config.MarketValue = 28000;

		Config.MaxHorsePower = 500.0f;
		Config.TuningDifficulty = 0.5f;

		return Config;
	}

	// ==========================================
	// PRO RACE / EXOTIC
	// ==========================================

	FMGVehicleModelConfig GetMcLarenF1Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("MCLAREN_F1"));
		Config.DisplayName = FText::FromString(TEXT("McLaren F1"));
		Config.Manufacturer = FText::FromString(TEXT("McLaren"));
		Config.Year = 1995;
		Config.Country = TEXT("UK");
		Config.VehicleType = EMGVehicleType::Hypercar;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// BMW S70/2 V12
		Config.BaseHorsePower = 627.0f;
		Config.BaseTorque = 479.0f;
		Config.BaseWeight = 1138.0f;
		Config.DisplacementCC = 6064;
		Config.Cylinders = 12;
		Config.EngineLayout = EMGEngineLayout::Mid;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 241.0f;
		Config.ZeroToSixty = 3.2f;
		Config.BasePI = 920;
		Config.BaseClass = EMGPerformanceClass::X;

		Config.BasePriceMSRP = 1000000;
		Config.MarketValue = 25000000;

		Config.MaxHorsePower = 750.0f;
		Config.TuningDifficulty = 0.9f;

		return Config;
	}

	FMGVehicleModelConfig GetFerrari360ModenaConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("FERRARI_360_MODENA"));
		Config.DisplayName = FText::FromString(TEXT("Ferrari 360 Modena"));
		Config.Manufacturer = FText::FromString(TEXT("Ferrari"));
		Config.Year = 1999;
		Config.Country = TEXT("Italy");
		Config.VehicleType = EMGVehicleType::Supercar;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 3.6L F131 V8
		Config.BaseHorsePower = 395.0f;
		Config.BaseTorque = 275.0f;
		Config.BaseWeight = 1390.0f;
		Config.DisplacementCC = 3586;
		Config.Cylinders = 8;
		Config.EngineLayout = EMGEngineLayout::Mid;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 183.0f;
		Config.ZeroToSixty = 4.3f;
		Config.BasePI = 780;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 150000;
		Config.MarketValue = 110000;

		Config.MaxHorsePower = 550.0f;
		Config.TuningDifficulty = 0.75f;

		return Config;
	}

	FMGVehicleModelConfig GetLamborghiniDiabloSVConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("LAMBORGHINI_DIABLO_SV"));
		Config.DisplayName = FText::FromString(TEXT("Lamborghini Diablo SV"));
		Config.Manufacturer = FText::FromString(TEXT("Lamborghini"));
		Config.Year = 1999;
		Config.Country = TEXT("Italy");
		Config.VehicleType = EMGVehicleType::Supercar;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// 5.7L V12
		Config.BaseHorsePower = 530.0f;
		Config.BaseTorque = 457.0f;
		Config.BaseWeight = 1530.0f;
		Config.DisplacementCC = 5707;
		Config.Cylinders = 12;
		Config.EngineLayout = EMGEngineLayout::Mid;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 204.0f;
		Config.ZeroToSixty = 3.9f;
		Config.BasePI = 850;
		Config.BaseClass = EMGPerformanceClass::S;

		Config.BasePriceMSRP = 250000;
		Config.MarketValue = 350000;

		Config.MaxHorsePower = 700.0f;
		Config.TuningDifficulty = 0.8f;

		return Config;
	}

	// ==========================================
	// STREET RACING SCENE - STARTER TIER
	// ==========================================

	FMGVehicleModelConfig GetHondaCivicTypeRConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("HONDA_CIVIC_TYPE_R_EK9"));
		Config.DisplayName = FText::FromString(TEXT("Honda Civic Type R"));
		Config.Manufacturer = FText::FromString(TEXT("Honda"));
		Config.Year = 1998;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Hatchback;
		Config.DrivetrainType = EMGDrivetrainType::FWD;

		// B16B VTEC
		Config.BaseHorsePower = 185.0f;
		Config.BaseTorque = 118.0f;
		Config.BaseWeight = 1050.0f;
		Config.DisplacementCC = 1595;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 140.0f;
		Config.ZeroToSixty = 6.5f;
		Config.BasePI = 480;
		Config.BaseClass = EMGPerformanceClass::C;

		Config.BasePriceMSRP = 0; // Starter car - free
		Config.MarketValue = 35000;

		Config.MaxHorsePower = 500.0f;
		Config.TuningDifficulty = 0.35f;

		return Config;
	}

	FMGVehicleModelConfig GetAcuraIntegraTTypeRConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("ACURA_INTEGRA_TYPE_R"));
		Config.DisplayName = FText::FromString(TEXT("Acura Integra Type R"));
		Config.Manufacturer = FText::FromString(TEXT("Acura"));
		Config.Year = 1998;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::FWD;

		// B18C5 VTEC
		Config.BaseHorsePower = 195.0f;
		Config.BaseTorque = 130.0f;
		Config.BaseWeight = 1125.0f;
		Config.DisplacementCC = 1797;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 145.0f;
		Config.ZeroToSixty = 6.2f;
		Config.BasePI = 510;
		Config.BaseClass = EMGPerformanceClass::B;

		Config.BasePriceMSRP = 25000;
		Config.MarketValue = 55000;

		Config.MaxHorsePower = 550.0f;
		Config.TuningDifficulty = 0.4f;

		return Config;
	}

	FMGVehicleModelConfig GetNissanZ33Config()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("NISSAN_350Z"));
		Config.DisplayName = FText::FromString(TEXT("Nissan 350Z"));
		Config.Manufacturer = FText::FromString(TEXT("Nissan"));
		Config.Year = 2003;
		Config.Country = TEXT("Japan");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::RWD;

		// VQ35DE V6
		Config.BaseHorsePower = 287.0f;
		Config.BaseTorque = 274.0f;
		Config.BaseWeight = 1469.0f;
		Config.DisplacementCC = 3498;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 155.0f;
		Config.ZeroToSixty = 5.4f;
		Config.BasePI = 600;
		Config.BaseClass = EMGPerformanceClass::A;

		Config.BasePriceMSRP = 28000;
		Config.MarketValue = 22000;

		Config.MaxHorsePower = 650.0f;
		Config.TuningDifficulty = 0.45f;

		return Config;
	}

	FMGVehicleModelConfig GetFordFocusSVTConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("FORD_FOCUS_SVT"));
		Config.DisplayName = FText::FromString(TEXT("Ford Focus SVT"));
		Config.Manufacturer = FText::FromString(TEXT("Ford"));
		Config.Year = 2002;
		Config.Country = TEXT("USA");
		Config.VehicleType = EMGVehicleType::Hatchback;
		Config.DrivetrainType = EMGDrivetrainType::FWD;

		// Zetec 2.0L
		Config.BaseHorsePower = 170.0f;
		Config.BaseTorque = 145.0f;
		Config.BaseWeight = 1240.0f;
		Config.DisplacementCC = 1989;
		Config.Cylinders = 4;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 135.0f;
		Config.ZeroToSixty = 7.2f;
		Config.BasePI = 440;
		Config.BaseClass = EMGPerformanceClass::C;

		Config.BasePriceMSRP = 18000;
		Config.MarketValue = 12000;

		Config.MaxHorsePower = 400.0f;
		Config.TuningDifficulty = 0.35f;

		return Config;
	}

	FMGVehicleModelConfig GetVWSciroccoConfig()
	{
		FMGVehicleModelConfig Config;
		Config.ModelID = FName(TEXT("VW_CORRADO_VR6"));
		Config.DisplayName = FText::FromString(TEXT("Volkswagen Corrado VR6"));
		Config.Manufacturer = FText::FromString(TEXT("Volkswagen"));
		Config.Year = 1995;
		Config.Country = TEXT("Germany");
		Config.VehicleType = EMGVehicleType::Coupe;
		Config.DrivetrainType = EMGDrivetrainType::FWD;

		// 2.9L VR6
		Config.BaseHorsePower = 190.0f;
		Config.BaseTorque = 185.0f;
		Config.BaseWeight = 1290.0f;
		Config.DisplacementCC = 2861;
		Config.Cylinders = 6;
		Config.EngineLayout = EMGEngineLayout::Front;
		Config.Aspiration = EMGAspirationType::NaturallyAspirated;

		Config.TopSpeedMPH = 145.0f;
		Config.ZeroToSixty = 6.4f;
		Config.BasePI = 480;
		Config.BaseClass = EMGPerformanceClass::C;

		Config.BasePriceMSRP = 22000;
		Config.MarketValue = 18000;

		Config.MaxHorsePower = 400.0f;
		Config.TuningDifficulty = 0.5f;

		return Config;
	}

	// ==========================================
	// VEHICLE REGISTRY
	// ==========================================

	TArray<FMGVehicleModelConfig> GetAllVehicles()
	{
		TArray<FMGVehicleModelConfig> Vehicles;

		// JDM
		Vehicles.Add(GetNissanSkylineR34Config());
		Vehicles.Add(GetToyotaSupraMK4Config());
		Vehicles.Add(GetMazdaRX7FDConfig());
		Vehicles.Add(GetHondaNSXConfig());
		Vehicles.Add(GetMitsubishiEvoVIConfig());
		Vehicles.Add(GetSubaruImprezaSTIConfig());
		Vehicles.Add(GetNissanSilviaS15Config());
		Vehicles.Add(GetHondaS2000Config());

		// American Muscle
		Vehicles.Add(GetFordMustangCobraRConfig());
		Vehicles.Add(GetChevroletCamaroSSConfig());
		Vehicles.Add(GetDodgeViperGTSConfig());
		Vehicles.Add(GetChevroletCorvetteZ06Config());
		Vehicles.Add(GetPontiacFirebirdTransAmConfig());

		// European
		Vehicles.Add(GetBMWM3E46Config());
		Vehicles.Add(GetPorsche911GT3Config());
		Vehicles.Add(GetMercedesAMGC32Config());
		Vehicles.Add(GetAudiS4B5Config());
		Vehicles.Add(GetLotusEliseS1Config());
		Vehicles.Add(GetVWGolfR32Config());

		// Pro Race / Exotic
		Vehicles.Add(GetMcLarenF1Config());
		Vehicles.Add(GetFerrari360ModenaConfig());
		Vehicles.Add(GetLamborghiniDiabloSVConfig());

		// Street Racing Starters
		Vehicles.Add(GetHondaCivicTypeRConfig());
		Vehicles.Add(GetAcuraIntegraTTypeRConfig());
		Vehicles.Add(GetNissanZ33Config());
		Vehicles.Add(GetFordFocusSVTConfig());
		Vehicles.Add(GetVWSciroccoConfig());

		return Vehicles;
	}

	TArray<FName> GetStarterVehicleChoices()
	{
		// Three starter car options representing different styles
		return {
			FName(TEXT("HONDA_CIVIC_TYPE_R_EK9")),   // JDM VTEC legend
			FName(TEXT("FORD_FOCUS_SVT")),          // American hot hatch
			FName(TEXT("VW_CORRADO_VR6"))           // European classic
		};
	}

	TArray<FMGVehicleModelConfig> GetJDMVehicles()
	{
		TArray<FMGVehicleModelConfig> Vehicles;
		Vehicles.Add(GetNissanSkylineR34Config());
		Vehicles.Add(GetToyotaSupraMK4Config());
		Vehicles.Add(GetMazdaRX7FDConfig());
		Vehicles.Add(GetHondaNSXConfig());
		Vehicles.Add(GetMitsubishiEvoVIConfig());
		Vehicles.Add(GetSubaruImprezaSTIConfig());
		Vehicles.Add(GetNissanSilviaS15Config());
		Vehicles.Add(GetHondaS2000Config());
		Vehicles.Add(GetHondaCivicTypeRConfig());
		Vehicles.Add(GetAcuraIntegraTTypeRConfig());
		Vehicles.Add(GetNissanZ33Config());
		return Vehicles;
	}

	TArray<FMGVehicleModelConfig> GetMuscleVehicles()
	{
		TArray<FMGVehicleModelConfig> Vehicles;
		Vehicles.Add(GetFordMustangCobraRConfig());
		Vehicles.Add(GetChevroletCamaroSSConfig());
		Vehicles.Add(GetDodgeViperGTSConfig());
		Vehicles.Add(GetChevroletCorvetteZ06Config());
		Vehicles.Add(GetPontiacFirebirdTransAmConfig());
		return Vehicles;
	}

	TArray<FMGVehicleModelConfig> GetEuropeanVehicles()
	{
		TArray<FMGVehicleModelConfig> Vehicles;
		Vehicles.Add(GetBMWM3E46Config());
		Vehicles.Add(GetPorsche911GT3Config());
		Vehicles.Add(GetMercedesAMGC32Config());
		Vehicles.Add(GetAudiS4B5Config());
		Vehicles.Add(GetLotusEliseS1Config());
		Vehicles.Add(GetVWGolfR32Config());
		Vehicles.Add(GetVWSciroccoConfig());
		return Vehicles;
	}

	TArray<FMGVehicleModelConfig> GetExoticVehicles()
	{
		TArray<FMGVehicleModelConfig> Vehicles;
		Vehicles.Add(GetMcLarenF1Config());
		Vehicles.Add(GetFerrari360ModenaConfig());
		Vehicles.Add(GetLamborghiniDiabloSVConfig());
		Vehicles.Add(GetHondaNSXConfig());
		Vehicles.Add(GetDodgeViperGTSConfig());
		return Vehicles;
	}
}
