// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MGVehicleFactory.h"
#include "MGDevCommands.generated.h"

class AMGVehiclePawn;

/**
 * Developer Commands Subsystem
 * Provides console commands and cheats for testing gameplay
 *
 * Console Commands (prefix with "MG."):
 * - MG.SpawnVehicle <preset> - Spawn a vehicle for player
 * - MG.SpawnAI <count> - Spawn AI opponents
 * - MG.StartRace - Start the race immediately
 * - MG.FinishRace - Force finish the race
 * - MG.SetLap <lap> - Set current lap number
 * - MG.AddCredits <amount> - Add credits
 * - MG.GodMode - Toggle invincibility
 * - MG.UnlimitedNitrous - Toggle unlimited nitrous
 * - MG.TimeScale <scale> - Set time scale
 * - MG.ShowDebug - Toggle debug display
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDevCommands : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// VEHICLE COMMANDS
	// ==========================================

	/** Spawn a vehicle at player start */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void SpawnVehicle(EMGVehiclePreset Preset = EMGVehiclePreset::JDM_Mid);

	/** Spawn AI opponent vehicles */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void SpawnAI(int32 Count = 5);

	/** Despawn all AI vehicles */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void DespawnAllAI();

	/** Teleport player to spawn point */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void TeleportToStart();

	/** Reset player vehicle */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void ResetVehicle();

	// ==========================================
	// RACE COMMANDS
	// ==========================================

	/** Start race countdown */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void StartRace();

	/** Force finish race */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void FinishRace();

	/** Restart current race */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void RestartRace();

	/** Set current lap */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SetLap(int32 LapNumber);

	/** Set player position */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SetPosition(int32 Position);

	/** Skip to results */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SkipToResults();

	// ==========================================
	// ECONOMY COMMANDS
	// ==========================================

	/** Add credits */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void AddCredits(int32 Amount = 100000);

	/** Add XP */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void AddXP(int32 Amount = 10000);

	/** Set player level */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void SetLevel(int32 Level);

	/** Unlock all vehicles */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void UnlockAllVehicles();

	// ==========================================
	// CHEAT COMMANDS
	// ==========================================

	/** Toggle god mode (no damage) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void GodMode();

	/** Toggle unlimited nitrous */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void UnlimitedNitrous();

	/** Toggle super speed */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void SuperSpeed();

	/** Set game time scale */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void TimeScale(float Scale = 1.0f);

	/** Freeze all AI */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void FreezeAI();

	// ==========================================
	// DEBUG COMMANDS
	// ==========================================

	/** Toggle debug HUD */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowDebug();

	/** Toggle checkpoint visualization */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowCheckpoints();

	/** Toggle racing line */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowRacingLine();

	/** Print current race state */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void PrintRaceState();

	/** Print vehicle stats */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void PrintVehicleStats();

	// ==========================================
	// AI DEBUG COMMANDS
	// ==========================================

	/** Toggle AI debug visualization (mood, state, targets) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void ShowAIDebug();

	/** Print all AI controller states */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void PrintAIStates();

	/** Set difficulty for all AI (0.0 = easy, 1.0 = hard) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void SetAIDifficulty(float Difficulty = 0.5f);

	/** Reset all AI moods to neutral */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void ResetAIMoods();

	// ==========================================
	// VEHICLE DEBUG COMMANDS
	// ==========================================

	/** Print current vehicle damage state */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void PrintDamageState();

	/** Print vehicle physics state (suspension, weight transfer, grip) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void PrintPhysicsState();

	/** Toggle tire debug visualization */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void ShowTireDebug();

	/** Full repair player vehicle */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void RepairVehicle();

	// ==========================================
	// ECONOMY DEBUG COMMANDS
	// ==========================================

	/** Print player economy state */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void PrintEconomyState();

	/** Simulate a purchase (dry run) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void SimulatePurchase(int32 Amount = 10000);

	/** Print recent transactions */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void PrintTransactions(int32 Count = 10);

	// ==========================================
	// QUICK TEST
	// ==========================================

	/** Quick start a test race with default settings */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Quick")
	void QuickRace(int32 AICount = 5, int32 Laps = 3);

	/** Quick start a time trial */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Quick")
	void QuickTimeTrial(int32 Laps = 3);

protected:
	// ==========================================
	// STATE
	// ==========================================

	bool bGodMode = false;
	bool bUnlimitedNitrous = false;
	bool bSuperSpeed = false;
	bool bAIFrozen = false;
	bool bShowDebug = false;
	bool bShowCheckpoints = false;
	bool bShowRacingLine = false;
	bool bShowAIDebug = false;
	bool bShowTireDebug = false;

	// ==========================================
	// HELPERS
	// ==========================================

	AMGVehiclePawn* GetPlayerVehicle() const;
	void LogCommand(const FString& Command);
};
