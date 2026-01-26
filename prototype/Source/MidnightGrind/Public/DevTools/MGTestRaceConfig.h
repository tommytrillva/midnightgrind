// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Test Race Configuration - MVP Testing Utility

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "MGTestRaceConfig.generated.h"

/**
 * Test race preset
 */
USTRUCT(BlueprintType)
struct FMGTestRacePreset
{
	GENERATED_BODY()

	/** Preset name for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FText PresetName;

	/** Description of test scenario */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FText Description;

	/** Full race setup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FMGRaceSetupRequest Setup;

	/** Auto-start countdown seconds (0 = manual) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float AutoStartDelay = 3.0f;

	/** Enable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bEnableDebugVis = true;

	/** Skip countdown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bSkipCountdown = false;

	/** Invincible player (no damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPlayerInvincible = false;

	/** AI doesn't attack player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPassiveAI = false;
};

/**
 * Test Race Configuration Data Asset
 *
 * Contains pre-configured race setups for testing various scenarios.
 * Use in editor to quickly test different race types, AI behaviors, etc.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTestRaceConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Available test presets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	TArray<FMGTestRacePreset> TestPresets;

	/** Default preset index */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	int32 DefaultPresetIndex = 0;

	/** Get preset by index */
	UFUNCTION(BlueprintPure, Category = "Test")
	FMGTestRacePreset GetPreset(int32 Index) const;

	/** Get preset by name */
	UFUNCTION(BlueprintPure, Category = "Test")
	bool GetPresetByName(const FText& Name, FMGTestRacePreset& OutPreset) const;

	/** Get all preset names */
	UFUNCTION(BlueprintPure, Category = "Test")
	TArray<FText> GetPresetNames() const;

	UMGTestRaceConfig();

#if WITH_EDITOR
	/** Generate default presets */
	UFUNCTION(CallInEditor, Category = "Test")
	void GenerateDefaultPresets();
#endif
};

/**
 * Test Race Runner
 *
 * Blueprint-callable utility for running test races from editor
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGTestRaceRunner : public UObject
{
	GENERATED_BODY()

public:
	// ==========================================
	// QUICK TEST FUNCTIONS
	// ==========================================

	/** Run the simplest possible test race */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunMinimalTest(UObject* WorldContextObject);

	/** Run a full race with all systems */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunFullTest(UObject* WorldContextObject);

	/** Run a drift test */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunDriftTest(UObject* WorldContextObject);

	/** Run a drag test */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunDragTest(UObject* WorldContextObject);

	/** Run a stress test with max AI */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunStressTest(UObject* WorldContextObject);

	/** Run from a test preset */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunFromPreset(UObject* WorldContextObject, const FMGTestRacePreset& Preset);

	// ==========================================
	// PRESET FACTORIES
	// ==========================================

	/** Create minimal test preset (2 laps, 3 AI, easy) */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateMinimalTestPreset();

	/** Create full test preset (3 laps, 7 AI, medium) */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateFullTestPreset();

	/** Create drift test preset */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateDriftTestPreset();

	/** Create drag test preset */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateDragTestPreset();

	/** Create sprint test preset */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateSprintTestPreset();

	/** Create pink slip test preset */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreatePinkSlipTestPreset();

	/** Create stress test preset (max AI, hard) */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateStressTestPreset();

	/** Create performance benchmark preset */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateBenchmarkPreset();
};
