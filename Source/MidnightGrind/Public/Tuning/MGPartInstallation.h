// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPartInstallation.generated.h"

class UMGPartDefinition;
class UMGEconomySubsystem;

/**
 * @brief Installation difficulty levels for vehicle parts
 *
 * Determines the complexity of installing a part, affecting:
 * - Base installation time
 * - DIY success rate
 * - Required tools and equipment
 * - Shop labor costs
 */
UENUM(BlueprintType)
enum class EMGInstallDifficulty : uint8
{
	/**
	 * Simple bolt-on parts requiring basic hand tools
	 * Examples: Air filters, shift knobs, interior trim
	 * Base time: 15 minutes | DIY risk: Minimal
	 */
	Simple UMETA(DisplayName = "Simple (15 min)"),

	/**
	 * Parts requiring moderate mechanical knowledge
	 * Examples: Exhaust systems, brake pads, coilovers
	 * Base time: 60 minutes | DIY risk: Low
	 */
	Moderate UMETA(DisplayName = "Moderate (1 hour)"),

	/**
	 * Complex installations requiring significant experience
	 * Examples: Turbo kits, transmission swaps, big brake kits
	 * Base time: 240 minutes | DIY risk: Moderate
	 */
	Complex UMETA(DisplayName = "Complex (4 hours)"),

	/**
	 * Expert-level installations requiring advanced knowledge
	 * Examples: Engine builds, drivetrain swaps, roll cages
	 * Base time: 480 minutes | DIY risk: High
	 */
	Expert UMETA(DisplayName = "Expert (8 hours)"),

	/**
	 * Professional-only installations - cannot be done DIY
	 * Examples: Custom fabrication, engine machining, paint/body
	 * Requires specialized shop equipment
	 */
	ShopOnly UMETA(DisplayName = "Shop Only")
};

/**
 * @brief Installation method chosen by the player
 */
UENUM(BlueprintType)
enum class EMGInstallMethod : uint8
{
	/** Player performs the installation themselves - free labor but risk of failure */
	DIY UMETA(DisplayName = "Do It Yourself"),

	/** Professional shop performs the installation - costs money but guaranteed quality */
	Shop UMETA(DisplayName = "Professional Shop")
};

/**
 * @brief Result of an installation attempt
 */
UENUM(BlueprintType)
enum class EMGInstallResult : uint8
{
	/** Installation completed successfully with full part effectiveness */
	Success UMETA(DisplayName = "Success"),

	/** Installation completed but with reduced effectiveness (DIY mishap) */
	Botched UMETA(DisplayName = "Botched"),

	/** Installation failed completely - part may be damaged */
	Failed UMETA(DisplayName = "Failed"),

	/** Installation cannot proceed - missing requirements */
	CannotInstall UMETA(DisplayName = "Cannot Install")
};

/**
 * @brief Installation requirements for a part
 *
 * Defines what is needed to install a specific part, including
 * time estimates, difficulty, and special requirements.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGInstallationRequirements
{
	GENERATED_BODY()

	/**
	 * @brief Difficulty level of the installation
	 * Affects DIY success rate and shop labor costs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	EMGInstallDifficulty Difficulty = EMGInstallDifficulty::Moderate;

	/**
	 * @brief Base installation time in minutes
	 * Actual time may vary based on mechanic skill and method
	 * Default values by difficulty:
	 * - Simple: 15 min
	 * - Moderate: 60 min
	 * - Complex: 240 min (4 hours)
	 * - Expert: 480 min (8 hours)
	 * - ShopOnly: Varies
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation", meta = (ClampMin = "5", ClampMax = "2880"))
	int32 InstallTimeMinutes = 60;

	/**
	 * @brief Whether the part requires the vehicle to be on a lift
	 * Examples: Exhaust systems, suspension, transmission work
	 * If true and player doesn't have lift access, must use shop
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresLift = false;

	/**
	 * @brief Whether the part requires special tools beyond basic hand tools
	 * Examples: Torque wrenches, spring compressors, alignment equipment
	 * Affects DIY success rate if player doesn't own the tools
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresSpecialTools = false;

	/**
	 * @brief List of specific tools required for installation
	 * Used for UI display and potential tool ownership system
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	TArray<FName> RequiredToolIDs;

	/**
	 * @brief Whether installation requires engine removal
	 * Significantly increases time and complexity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresEngineRemoval = false;

	/**
	 * @brief Whether installation requires transmission removal
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresTransmissionRemoval = false;

	/**
	 * @brief Whether the part needs professional tuning after install
	 * Examples: ECU tunes, turbo kits, fuel system upgrades
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresDynoTuning = false;

	/**
	 * @brief Additional shop labor cost multiplier for this specific part
	 * Default 1.0 = standard rate, higher = more expensive
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float LaborCostMultiplier = 1.0f;

	/** Get default install time for a difficulty level */
	static int32 GetDefaultInstallTime(EMGInstallDifficulty InDifficulty)
	{
		switch (InDifficulty)
		{
		case EMGInstallDifficulty::Simple:
			return 15;
		case EMGInstallDifficulty::Moderate:
			return 60;
		case EMGInstallDifficulty::Complex:
			return 240;
		case EMGInstallDifficulty::Expert:
			return 480;
		case EMGInstallDifficulty::ShopOnly:
			return 480;
		default:
			return 60;
		}
	}
};

/**
 * @brief Result data from a part installation attempt
 *
 * Contains all information about the outcome of an installation,
 * including any penalties from botched installs.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGInstallationResult
{
	GENERATED_BODY()

	/** The outcome of the installation attempt */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	EMGInstallResult Result = EMGInstallResult::CannotInstall;

	/** Whether the installation was successful (Success or Botched) */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	bool bInstallationComplete = false;

	/**
	 * Part effectiveness multiplier (0.0 - 1.0)
	 * Perfect install = 1.0, botched = 0.80-0.95
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	float EffectivenessMultiplier = 1.0f;

	/**
	 * Wear rate multiplier for the installed part
	 * Perfect install = 1.0, botched = 1.1-1.5 (wears faster)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	float WearRateMultiplier = 1.0f;

	/**
	 * Whether the part was damaged during installation
	 * Only possible on Failed installs
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	bool bPartDamaged = false;

	/**
	 * Damage percentage if part was damaged (0.0 - 1.0)
	 * Reduces part's remaining lifespan/value
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	float PartDamagePercent = 0.0f;

	/** Total labor cost paid (0 for DIY) */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	int64 LaborCostPaid = 0;

	/** Actual time taken for installation in minutes */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	int32 ActualInstallTimeMinutes = 0;

	/** XP gained from the installation (DIY only) */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	int32 MechanicXPGained = 0;

	/** Human-readable message about the result */
	UPROPERTY(BlueprintReadOnly, Category = "Installation")
	FText ResultMessage;

	/** Create a successful result */
	static FMGInstallationResult Success(int32 InstallTime, int64 LaborCost = 0, int32 XPGained = 0)
	{
		FMGInstallationResult OutResult;
		OutResult.Result = EMGInstallResult::Success;
		OutResult.bInstallationComplete = true;
		OutResult.EffectivenessMultiplier = 1.0f;
		OutResult.WearRateMultiplier = 1.0f;
		OutResult.ActualInstallTimeMinutes = InstallTime;
		OutResult.LaborCostPaid = LaborCost;
		OutResult.MechanicXPGained = XPGained;
		OutResult.ResultMessage = NSLOCTEXT("MGInstall", "Success", "Installation completed successfully!");
		return OutResult;
	}

	/** Create a botched result */
	static FMGInstallationResult Botched(int32 InstallTime, float Effectiveness, float WearRate, int32 XPGained)
	{
		FMGInstallationResult OutResult;
		OutResult.Result = EMGInstallResult::Botched;
		OutResult.bInstallationComplete = true;
		OutResult.EffectivenessMultiplier = Effectiveness;
		OutResult.WearRateMultiplier = WearRate;
		OutResult.ActualInstallTimeMinutes = InstallTime;
		OutResult.MechanicXPGained = XPGained;
		OutResult.ResultMessage = NSLOCTEXT("MGInstall", "Botched", "Installation complete, but not perfect. Part effectiveness reduced.");
		return OutResult;
	}

	/** Create a failed result */
	static FMGInstallationResult Failed(bool bDamaged, float DamagePercent)
	{
		FMGInstallationResult OutResult;
		OutResult.Result = EMGInstallResult::Failed;
		OutResult.bInstallationComplete = false;
		OutResult.bPartDamaged = bDamaged;
		OutResult.PartDamagePercent = DamagePercent;
		OutResult.ResultMessage = bDamaged
			? NSLOCTEXT("MGInstall", "FailedDamaged", "Installation failed! Part was damaged in the process.")
			: NSLOCTEXT("MGInstall", "Failed", "Installation failed. No damage to parts.");
		return OutResult;
	}
};

/**
 * @brief Player's mechanic skill progression data
 *
 * Tracks the player's DIY installation skill level, which improves
 * over time through successful installations. Higher skill reduces
 * the chance of botched installs and unlocks ability to attempt
 * more difficult installations.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGMechanicSkill
{
	GENERATED_BODY()

	/**
	 * @brief Current mechanic skill level (1-100)
	 * Affects DIY success rate and time efficiency
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill", meta = (ClampMin = "1", ClampMax = "100"))
	int32 SkillLevel = 1;

	/**
	 * @brief Current XP toward next skill level
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill")
	int32 CurrentXP = 0;

	/**
	 * @brief Total installations completed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill")
	int32 TotalInstallations = 0;

	/**
	 * @brief Successful installations (no botches)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill")
	int32 SuccessfulInstallations = 0;

	/**
	 * @brief Botched installations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill")
	int32 BotchedInstallations = 0;

	/**
	 * @brief Failed installations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Skill")
	int32 FailedInstallations = 0;

	/** Get XP required for next level */
	int32 GetXPForNextLevel() const
	{
		// Exponential curve: 100 * level^1.5
		return static_cast<int32>(100.0f * FMath::Pow(static_cast<float>(SkillLevel), 1.5f));
	}

	/** Get progress to next level (0.0 - 1.0) */
	float GetLevelProgress() const
	{
		const int32 Required = GetXPForNextLevel();
		return Required > 0 ? FMath::Clamp(static_cast<float>(CurrentXP) / static_cast<float>(Required), 0.0f, 1.0f) : 0.0f;
	}

	/** Get success rate for a given difficulty (0.0 - 1.0) */
	float GetSuccessRate(EMGInstallDifficulty Difficulty) const
	{
		// Base success rates by difficulty
		float BaseRate = 0.0f;
		switch (Difficulty)
		{
		case EMGInstallDifficulty::Simple:
			BaseRate = 0.95f;
			break;
		case EMGInstallDifficulty::Moderate:
			BaseRate = 0.80f;
			break;
		case EMGInstallDifficulty::Complex:
			BaseRate = 0.60f;
			break;
		case EMGInstallDifficulty::Expert:
			BaseRate = 0.40f;
			break;
		case EMGInstallDifficulty::ShopOnly:
			return 0.0f; // Cannot DIY
		}

		// Skill bonus: up to +30% at max skill
		const float SkillBonus = (SkillLevel - 1) / 99.0f * 0.30f;

		return FMath::Clamp(BaseRate + SkillBonus, 0.0f, 0.98f);
	}

	/** Get time reduction multiplier based on skill (0.7 - 1.0) */
	float GetTimeMultiplier() const
	{
		// At max skill, installations take 30% less time
		return 1.0f - ((SkillLevel - 1) / 99.0f * 0.30f);
	}

	/** Check if player can attempt a difficulty level */
	bool CanAttemptDifficulty(EMGInstallDifficulty Difficulty) const
	{
		switch (Difficulty)
		{
		case EMGInstallDifficulty::Simple:
			return true;
		case EMGInstallDifficulty::Moderate:
			return SkillLevel >= 5;
		case EMGInstallDifficulty::Complex:
			return SkillLevel >= 20;
		case EMGInstallDifficulty::Expert:
			return SkillLevel >= 50;
		case EMGInstallDifficulty::ShopOnly:
			return false;
		}
		return false;
	}

	/** Get minimum skill level required for a difficulty */
	static int32 GetMinSkillForDifficulty(EMGInstallDifficulty Difficulty)
	{
		switch (Difficulty)
		{
		case EMGInstallDifficulty::Simple:
			return 1;
		case EMGInstallDifficulty::Moderate:
			return 5;
		case EMGInstallDifficulty::Complex:
			return 20;
		case EMGInstallDifficulty::Expert:
			return 50;
		case EMGInstallDifficulty::ShopOnly:
			return 999; // Effectively impossible
		}
		return 1;
	}
};

/**
 * @brief Installed part instance with installation quality data
 *
 * Extends the base installed part concept with information about
 * how well the part was installed, affecting its performance.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGInstalledPartInstance
{
	GENERATED_BODY()

	/** The part definition ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	/** Unique instance identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FGuid InstanceID;

	/** How the part was installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	EMGInstallMethod InstallMethod = EMGInstallMethod::Shop;

	/** Result of the installation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	EMGInstallResult InstallResult = EMGInstallResult::Success;

	/**
	 * Part effectiveness multiplier based on install quality
	 * 1.0 = perfect, 0.80-0.95 = botched
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EffectivenessMultiplier = 1.0f;

	/**
	 * Wear rate multiplier based on install quality
	 * 1.0 = normal, >1.0 = wears faster (botched install)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float WearRateMultiplier = 1.0f;

	/** Current wear level (0.0 = new, 1.0 = worn out) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WearLevel = 0.0f;

	/** Date/time the part was installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FDateTime InstallDate;

	/** Who installed the part (player name or shop name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	FString InstalledBy;

	/** Labor cost paid for installation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	int64 LaborCostPaid = 0;

	FMGInstalledPartInstance()
	{
		InstanceID = FGuid::NewGuid();
		InstallDate = FDateTime::Now();
	}

	/** Check if this is a botched installation */
	bool IsBotched() const
	{
		return InstallResult == EMGInstallResult::Botched;
	}

	/** Get effective performance multiplier accounting for wear */
	float GetCurrentEffectiveness() const
	{
		// Effectiveness degrades with wear
		const float WearPenalty = WearLevel * 0.20f; // Up to 20% penalty at full wear
		return FMath::Max(0.5f, EffectivenessMultiplier - WearPenalty);
	}
};

/**
 * @brief Shop pricing and service configuration
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGShopConfig
{
	GENERATED_BODY()

	/** Base labor rate per hour in credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	int64 LaborRatePerHour = 75;

	/** Minimum labor charge regardless of time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	int64 MinimumLaborCharge = 50;

	/** Dyno tuning session cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	int64 DynoTuningCost = 250;

	/** Alignment service cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	int64 AlignmentCost = 100;

	/** Rush job multiplier (same day service) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	float RushJobMultiplier = 1.5f;

	/** Calculate labor cost for an installation */
	int64 CalculateLaborCost(const FMGInstallationRequirements& Requirements) const
	{
		const float Hours = Requirements.InstallTimeMinutes / 60.0f;
		int64 BaseCost = static_cast<int64>(Hours * LaborRatePerHour * Requirements.LaborCostMultiplier);

		// Add dyno tuning if required
		if (Requirements.bRequiresDynoTuning)
		{
			BaseCost += DynoTuningCost;
		}

		return FMath::Max(BaseCost, MinimumLaborCharge);
	}
};

//=============================================================================
// Wrapper Structs for TMap Value Types
//=============================================================================

/**
 * @brief Wrapper for TMap<FName, FMGInstalledPartInstance> to support UPROPERTY in TMap.
 */
USTRUCT(BlueprintType)
struct FMGSlotPartInstanceMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FMGInstalledPartInstance> Parts;
};

/** Delegate for installation completion */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPartInstallationComplete, FName, PartID, FGuid, VehicleID, const FMGInstallationResult&, Result);

/** Delegate for mechanic skill level up */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMechanicSkillLevelUp, int32, NewLevel, int32, OldLevel);

/** Delegate for installation started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnInstallationStarted, FName, PartID, FGuid, VehicleID, EMGInstallMethod, Method, int32, EstimatedMinutes);

/**
 * @brief Game Instance Subsystem for part installation mechanics
 *
 * Manages the installation of parts onto vehicles, including:
 * - DIY vs Professional shop installation
 * - Installation time and labor costs
 * - Botched install mechanics and consequences
 * - Mechanic skill progression
 *
 * DESIGN PHILOSOPHY:
 * - DIY saves money but carries risk - rewards player investment in the mechanic skill
 * - Shop installations are guaranteed quality but cost credits
 * - Higher difficulty parts have higher stakes for DIY attempts
 * - Skill progression makes DIY more viable over time
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPartInstallationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// INSTALLATION
	// ==========================================

	/**
	 * @brief Attempt to install a part on a vehicle
	 *
	 * @param VehicleID The vehicle to install the part on
	 * @param PartID The part to install
	 * @param Method DIY or Shop installation
	 * @param OutResult The result of the installation attempt
	 * @return True if installation started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation")
	bool InstallPart(FGuid VehicleID, FName PartID, EMGInstallMethod Method, FMGInstallationResult& OutResult);

	/**
	 * @brief Preview the cost and time for an installation
	 *
	 * @param PartID The part to check
	 * @param Method The installation method
	 * @param OutLaborCost The labor cost (0 for DIY)
	 * @param OutTimeMinutes The estimated installation time
	 * @param OutSuccessRate The success rate for DIY (1.0 for shop)
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation")
	void PreviewInstallation(FName PartID, EMGInstallMethod Method, int64& OutLaborCost, int32& OutTimeMinutes, float& OutSuccessRate) const;

	/**
	 * @brief Check if a part can be installed DIY
	 *
	 * @param PartID The part to check
	 * @param OutReason Human-readable reason if cannot install
	 * @return True if DIY installation is possible
	 */
	UFUNCTION(BlueprintPure, Category = "Installation")
	bool CanInstallDIY(FName PartID, FText& OutReason) const;

	/**
	 * @brief Get installation requirements for a part
	 *
	 * @param PartID The part to check
	 * @param OutRequirements The installation requirements
	 * @return True if part was found
	 */
	UFUNCTION(BlueprintPure, Category = "Installation")
	bool GetInstallationRequirements(FName PartID, FMGInstallationRequirements& OutRequirements) const;

	// ==========================================
	// MECHANIC SKILL
	// ==========================================

	/**
	 * @brief Get the player's current mechanic skill
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Skill")
	FMGMechanicSkill GetMechanicSkill() const { return MechanicSkill; }

	/**
	 * @brief Get the player's mechanic skill level
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Skill")
	int32 GetMechanicSkillLevel() const { return MechanicSkill.SkillLevel; }

	/**
	 * @brief Get DIY success rate for a difficulty level
	 *
	 * @param Difficulty The installation difficulty
	 * @return Success rate (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Skill")
	float GetDIYSuccessRate(EMGInstallDifficulty Difficulty) const;

	/**
	 * @brief Check if player meets skill requirement for a difficulty
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Skill")
	bool MeetsSkillRequirement(EMGInstallDifficulty Difficulty) const;

	/**
	 * @brief Add mechanic XP (typically from successful installations)
	 *
	 * @param XPAmount Amount of XP to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation|Skill")
	void AddMechanicXP(int32 XPAmount);

	// ==========================================
	// SHOP CONFIGURATION
	// ==========================================

	/**
	 * @brief Get the current shop configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Shop")
	FMGShopConfig GetShopConfig() const { return ShopConfig; }

	/**
	 * @brief Calculate total labor cost for an installation
	 *
	 * @param PartID The part to install
	 * @return Labor cost in credits
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Shop")
	int64 CalculateLaborCost(FName PartID) const;

	/**
	 * @brief Get the base labor rate per hour
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Shop")
	int64 GetLaborRatePerHour() const { return ShopConfig.LaborRatePerHour; }

	// ==========================================
	// INSTALLED PARTS QUERY
	// ==========================================

	/**
	 * @brief Get an installed part instance by vehicle and slot
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Parts")
	bool GetInstalledPartInstance(FGuid VehicleID, FName SlotID, FMGInstalledPartInstance& OutInstance) const;

	/**
	 * @brief Check if an installed part is botched
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Parts")
	bool IsPartBotched(FGuid VehicleID, FName SlotID) const;

	/**
	 * @brief Get effectiveness of an installed part (accounting for install quality and wear)
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Parts")
	float GetPartEffectiveness(FGuid VehicleID, FName SlotID) const;

	// ==========================================
	// PLAYER FACILITIES
	// ==========================================

	/**
	 * @brief Check if player has access to a vehicle lift
	 * Affects ability to DIY certain installations
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Facilities")
	bool HasLiftAccess() const { return bHasLiftAccess; }

	/**
	 * @brief Set player's lift access status
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation|Facilities")
	void SetLiftAccess(bool bHasAccess) { bHasLiftAccess = bHasAccess; }

	/**
	 * @brief Check if player owns a specific tool
	 */
	UFUNCTION(BlueprintPure, Category = "Installation|Facilities")
	bool OwnsTool(FName ToolID) const;

	/**
	 * @brief Grant a tool to the player
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation|Facilities")
	void GrantTool(FName ToolID);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a part installation completes */
	UPROPERTY(BlueprintAssignable, Category = "Installation|Events")
	FOnPartInstallationComplete OnPartInstallationComplete;

	/** Called when mechanic skill levels up */
	UPROPERTY(BlueprintAssignable, Category = "Installation|Events")
	FOnMechanicSkillLevelUp OnMechanicSkillLevelUp;

	/** Called when an installation starts */
	UPROPERTY(BlueprintAssignable, Category = "Installation|Events")
	FOnInstallationStarted OnInstallationStarted;

	// ==========================================
	// PART REGISTRATION
	// ==========================================

	/**
	 * @brief Register installation requirements for a part
	 * Called during part catalog initialization
	 */
	UFUNCTION(BlueprintCallable, Category = "Installation|Setup")
	void RegisterPartRequirements(FName PartID, const FMGInstallationRequirements& Requirements);

protected:
	/**
	 * @brief Perform the DIY installation roll
	 *
	 * @param Requirements The part's installation requirements
	 * @param OutEffectiveness Output effectiveness multiplier
	 * @param OutWearRate Output wear rate multiplier
	 * @return The installation result
	 */
	EMGInstallResult RollDIYInstallation(const FMGInstallationRequirements& Requirements, float& OutEffectiveness, float& OutWearRate);

	/**
	 * @brief Calculate XP gained from an installation
	 */
	int32 CalculateInstallXP(EMGInstallDifficulty Difficulty, EMGInstallResult Result) const;

	/**
	 * @brief Check for mechanic skill level up
	 */
	void CheckSkillLevelUp();

	/**
	 * @brief Save progression data
	 */
	void SaveProgressionData();

	/**
	 * @brief Load progression data
	 */
	void LoadProgressionData();

	// ==========================================
	// DATA
	// ==========================================

	/** Player's mechanic skill progression */
	UPROPERTY(SaveGame)
	FMGMechanicSkill MechanicSkill;

	/** Shop pricing configuration */
	UPROPERTY(EditAnywhere, Category = "Configuration")
	FMGShopConfig ShopConfig;

	/** Database of installation requirements by part ID */
	UPROPERTY()
	TMap<FName, FMGInstallationRequirements> PartRequirementsDatabase;

	/** Installed part instances by vehicle ID and slot */
	UPROPERTY(SaveGame)
	TMap<FGuid, FMGSlotPartInstanceMap> InstalledPartsMap;

	/** Whether player has access to a vehicle lift */
	UPROPERTY(SaveGame)
	bool bHasLiftAccess = false;

	/** Tools owned by the player */
	UPROPERTY(SaveGame)
	TSet<FName> OwnedTools;
};
