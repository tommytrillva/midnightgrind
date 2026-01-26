// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Test Race Configuration - MVP Testing Utility

#include "DevTools/MGTestRaceConfig.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGTestRace, Log, All);

// ==========================================
// TEST RACE CONFIG DATA ASSET
// ==========================================

UMGTestRaceConfig::UMGTestRaceConfig()
{
	// Generate default presets in constructor
#if WITH_EDITOR
	GenerateDefaultPresets();
#endif
}

FMGTestRacePreset UMGTestRaceConfig::GetPreset(int32 Index) const
{
	if (TestPresets.IsValidIndex(Index))
	{
		return TestPresets[Index];
	}

	// Return first preset or empty
	if (TestPresets.Num() > 0)
	{
		return TestPresets[0];
	}

	return FMGTestRacePreset();
}

bool UMGTestRaceConfig::GetPresetByName(const FText& Name, FMGTestRacePreset& OutPreset) const
{
	for (const FMGTestRacePreset& Preset : TestPresets)
	{
		if (Preset.PresetName.EqualTo(Name))
		{
			OutPreset = Preset;
			return true;
		}
	}
	return false;
}

TArray<FText> UMGTestRaceConfig::GetPresetNames() const
{
	TArray<FText> Names;
	for (const FMGTestRacePreset& Preset : TestPresets)
	{
		Names.Add(Preset.PresetName);
	}
	return Names;
}

#if WITH_EDITOR
void UMGTestRaceConfig::GenerateDefaultPresets()
{
	TestPresets.Empty();

	// Minimal test
	TestPresets.Add(UMGTestRaceRunner::CreateMinimalTestPreset());

	// Full test
	TestPresets.Add(UMGTestRaceRunner::CreateFullTestPreset());

	// Drift test
	TestPresets.Add(UMGTestRaceRunner::CreateDriftTestPreset());

	// Drag test
	TestPresets.Add(UMGTestRaceRunner::CreateDragTestPreset());

	// Sprint test
	TestPresets.Add(UMGTestRaceRunner::CreateSprintTestPreset());

	// Pink slip test
	TestPresets.Add(UMGTestRaceRunner::CreatePinkSlipTestPreset());

	// Stress test
	TestPresets.Add(UMGTestRaceRunner::CreateStressTestPreset());

	// Benchmark
	TestPresets.Add(UMGTestRaceRunner::CreateBenchmarkPreset());

	UE_LOG(LogMGTestRace, Log, TEXT("Generated %d default test presets"), TestPresets.Num());
}
#endif

// ==========================================
// TEST RACE RUNNER
// ==========================================

bool UMGTestRaceRunner::RunMinimalTest(UObject* WorldContextObject)
{
	return RunFromPreset(WorldContextObject, CreateMinimalTestPreset());
}

bool UMGTestRaceRunner::RunFullTest(UObject* WorldContextObject)
{
	return RunFromPreset(WorldContextObject, CreateFullTestPreset());
}

bool UMGTestRaceRunner::RunDriftTest(UObject* WorldContextObject)
{
	return RunFromPreset(WorldContextObject, CreateDriftTestPreset());
}

bool UMGTestRaceRunner::RunDragTest(UObject* WorldContextObject)
{
	return RunFromPreset(WorldContextObject, CreateDragTestPreset());
}

bool UMGTestRaceRunner::RunStressTest(UObject* WorldContextObject)
{
	return RunFromPreset(WorldContextObject, CreateStressTestPreset());
}

bool UMGTestRaceRunner::RunFromPreset(UObject* WorldContextObject, const FMGTestRacePreset& Preset)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogMGTestRace, Error, TEXT("No world context for test race"));
		return false;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		UE_LOG(LogMGTestRace, Error, TEXT("No game instance for test race"));
		return false;
	}

	UMGRaceFlowSubsystem* RaceFlow = GI->GetSubsystem<UMGRaceFlowSubsystem>();
	if (!RaceFlow)
	{
		UE_LOG(LogMGTestRace, Error, TEXT("Race flow subsystem not available"));
		return false;
	}

	if (!RaceFlow->CanStartRace())
	{
		UE_LOG(LogMGTestRace, Warning, TEXT("Cannot start race - another may be in progress"));
		return false;
	}

	UE_LOG(LogMGTestRace, Log, TEXT("Starting test race: %s"), *Preset.PresetName.ToString());
	UE_LOG(LogMGTestRace, Log, TEXT("  Track: %s"), *Preset.Setup.TrackID.ToString());
	UE_LOG(LogMGTestRace, Log, TEXT("  Type: %s"), *Preset.Setup.RaceType.ToString());
	UE_LOG(LogMGTestRace, Log, TEXT("  Laps: %d"), Preset.Setup.LapCount);
	UE_LOG(LogMGTestRace, Log, TEXT("  AI: %d @ %.0f%% difficulty"), Preset.Setup.AICount, Preset.Setup.AIDifficulty * 100.0f);

	return RaceFlow->StartRace(Preset.Setup);
}

// ==========================================
// PRESET FACTORIES
// ==========================================

FMGTestRacePreset UMGTestRaceRunner::CreateMinimalTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Minimal Test"));
	Preset.Description = FText::FromString(TEXT("2 laps, 3 easy AI - Quick functionality check"));

	Preset.Setup.TrackID = FName("Track_Downtown");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_240SX");
	Preset.Setup.RaceType = FName("Circuit");
	Preset.Setup.LapCount = 2;
	Preset.Setup.AICount = 3;
	Preset.Setup.AIDifficulty = 0.25f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 2000;
	Preset.Setup.BaseRepReward = 50;

	Preset.AutoStartDelay = 2.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = true;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateFullTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Full Race Test"));
	Preset.Description = FText::FromString(TEXT("3 laps, 7 AI medium difficulty - Full race experience"));

	Preset.Setup.TrackID = FName("Track_Downtown");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_Supra");
	Preset.Setup.RaceType = FName("Circuit");
	Preset.Setup.LapCount = 3;
	Preset.Setup.AICount = 7;
	Preset.Setup.AIDifficulty = 0.5f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 5000;
	Preset.Setup.BaseRepReward = 100;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = false;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateDriftTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Drift Test"));
	Preset.Description = FText::FromString(TEXT("Drift competition - Test scoring and physics"));

	Preset.Setup.TrackID = FName("Track_Docks");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_240SX");
	Preset.Setup.RaceType = FName("Drift");
	Preset.Setup.LapCount = 2;
	Preset.Setup.AICount = 3;
	Preset.Setup.AIDifficulty = 0.4f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 4000;
	Preset.Setup.BaseRepReward = 100;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = false;
	Preset.bPassiveAI = true;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateDragTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Drag Test"));
	Preset.Description = FText::FromString(TEXT("Quarter mile drag race - Test launch and straight line"));

	Preset.Setup.TrackID = FName("Track_Airport");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_Mustang");
	Preset.Setup.RaceType = FName("Drag");
	Preset.Setup.LapCount = 1;
	Preset.Setup.AICount = 1;
	Preset.Setup.AIDifficulty = 0.5f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 2500;
	Preset.Setup.BaseRepReward = 50;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = false;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateSprintTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Sprint Test"));
	Preset.Description = FText::FromString(TEXT("Point-to-point sprint race"));

	Preset.Setup.TrackID = FName("Track_Highway");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_Skyline");
	Preset.Setup.RaceType = FName("Sprint");
	Preset.Setup.LapCount = 1;
	Preset.Setup.AICount = 5;
	Preset.Setup.AIDifficulty = 0.5f;
	Preset.Setup.TimeOfDay = 0.1f;  // Just after midnight
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 3500;
	Preset.Setup.BaseRepReward = 75;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = false;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreatePinkSlipTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Pink Slip Test"));
	Preset.Description = FText::FromString(TEXT("High-stakes pink slip race - Winner takes car"));

	Preset.Setup.TrackID = FName("Track_Industrial");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_RX7");
	Preset.Setup.RaceType = FName("Circuit");
	Preset.Setup.LapCount = 3;
	Preset.Setup.AICount = 1;
	Preset.Setup.AIDifficulty = 0.6f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.bIsPinkSlip = true;
	Preset.Setup.PinkSlipVehicleID = FName("Vehicle_GTR");
	Preset.Setup.BaseCashReward = 0;  // No cash, just the car
	Preset.Setup.BaseRepReward = 250;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = false;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateStressTestPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Stress Test"));
	Preset.Description = FText::FromString(TEXT("Maximum AI, high difficulty - Performance stress test"));

	Preset.Setup.TrackID = FName("Track_Downtown");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_GTR");
	Preset.Setup.RaceType = FName("Circuit");
	Preset.Setup.LapCount = 5;
	Preset.Setup.AICount = 11;  // Max AI
	Preset.Setup.AIDifficulty = 0.9f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.8f;  // Heavy rain
	Preset.Setup.BaseCashReward = 10000;
	Preset.Setup.BaseRepReward = 200;

	Preset.AutoStartDelay = 5.0f;
	Preset.bEnableDebugVis = true;
	Preset.bSkipCountdown = false;
	Preset.bPlayerInvincible = true;
	Preset.bPassiveAI = false;

	return Preset;
}

FMGTestRacePreset UMGTestRaceRunner::CreateBenchmarkPreset()
{
	FMGTestRacePreset Preset;
	Preset.PresetName = FText::FromString(TEXT("Performance Benchmark"));
	Preset.Description = FText::FromString(TEXT("Standardized benchmark for performance comparison"));

	Preset.Setup.TrackID = FName("Track_Downtown");
	Preset.Setup.PlayerVehicleID = FName("Vehicle_Supra");
	Preset.Setup.RaceType = FName("Circuit");
	Preset.Setup.LapCount = 3;
	Preset.Setup.AICount = 7;
	Preset.Setup.AIDifficulty = 0.5f;
	Preset.Setup.TimeOfDay = 0.0f;
	Preset.Setup.Weather = 0.0f;
	Preset.Setup.BaseCashReward = 5000;
	Preset.Setup.BaseRepReward = 100;

	Preset.AutoStartDelay = 3.0f;
	Preset.bEnableDebugVis = false;  // No debug overhead
	Preset.bSkipCountdown = true;    // Consistent start
	Preset.bPlayerInvincible = true; // No variation from damage
	Preset.bPassiveAI = false;

	return Preset;
}
