// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGProgressionTypes.h
 * @brief Extended progression types for 150-200h single-player experience
 * 
 * This file defines the data structures for the deep progression systems:
 * - Car Tier Progression (5 tiers: Beaters → Legends)
 * - Location/District Unlocks (6 districts)
 * - Housing Progression (4 tiers: Garage → Penthouse)
 * - Customization Depth Tracking
 * - Enhanced Prerequisite System
 * 
 * @see MGPlayerProgression.h for core progression subsystem
 * @see Progression-Systems-Design.md for full design documentation
 */

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MGProgressionTypes.generated.h"

// =============================================================================
// ENUMERATIONS
// =============================================================================

/**
 * @brief Car tier classification for progression
 * 
 * Defines the 5-tier car progression system:
 * - Street Beaters (0-20h): Learn basics
 * - Tuner Cars (20-60h): Specialize and experiment
 * - Super Cars (60-120h): Premium performance
 * - Hypercars (120-180h): Elite status symbols
 * - Legends (180-200h): Trophy collection
 */
UENUM(BlueprintType)
enum class EMGCarTier : uint8
{
	None			UMETA(DisplayName = "No Tier"),
	StreetBeaters	UMETA(DisplayName = "Street Beaters"),	///< Tier 1: Starting cars (0-20h)
	TunerCars		UMETA(DisplayName = "Tuner Cars"),		///< Tier 2: Import performance (20-60h)
	SuperCars		UMETA(DisplayName = "Super Cars"),		///< Tier 3: Exotic performance (60-120h)
	Hypercars		UMETA(DisplayName = "Hypercars"),		///< Tier 4: Elite machines (120-180h)
	Legends			UMETA(DisplayName = "Legends")			///< Tier 5: Trophy vehicles (180-200h)
};

/**
 * @brief Housing tier for lifestyle progression
 * 
 * Represents the 4-stage housing system with increasing
 * car display capacity and customization options.
 */
UENUM(BlueprintType)
enum class EMGHousingTier : uint8
{
	None		UMETA(DisplayName = "No Housing"),
	Garage		UMETA(DisplayName = "Garage"),			///< Tier 1: 1 car display (0-40h)
	Apartment	UMETA(DisplayName = "Apartment"),		///< Tier 2: 3 car display (40-100h)
	Loft		UMETA(DisplayName = "Loft"),			///< Tier 3: 8 car display (100-150h)
	Penthouse	UMETA(DisplayName = "Penthouse")		///< Tier 4: 15 car display (150-200h)
};

/**
 * @brief District categories for location unlocks
 * 
 * 6 major districts that unlock progressively through the campaign.
 */
UENUM(BlueprintType)
enum class EMGDistrict : uint8
{
	None			UMETA(DisplayName = "No District"),
	Industrial		UMETA(DisplayName = "Industrial"),		///< Starting area (0h)
	Downtown		UMETA(DisplayName = "Downtown"),		///< Neon streets (15h)
	HarborDocks		UMETA(DisplayName = "Harbor Docks"),	///< Underground scene (40h)
	Financial		UMETA(DisplayName = "Financial"),		///< Glass towers (80h)
	HillsEstates	UMETA(DisplayName = "Hills Estates"),	///< Winding roads (130h)
	Airport			UMETA(DisplayName = "Airport")			///< Pro circuit (170h)
};

/**
 * @brief Customization categories for tracking unlock depth
 */
UENUM(BlueprintType)
enum class EMGCustomizationType : uint8
{
	Paint,			///< Paint colors and finishes
	Decal,			///< Decals and vinyls
	BodyKit,		///< Body kits and aero parts
	Lighting,		///< Underglow, headlights, accents
	Performance,	///< Engine, suspension, tuning parts
	PhotoMode,		///< Camera tools and locations
	Housing			///< Apartment furniture and equipment
};

/**
 * @brief Prerequisite types for unlock requirements
 */
UENUM(BlueprintType)
enum class EMGPrerequisiteType : uint8
{
	Reputation,			///< Total reputation across all crews
	Money,				///< Player cash balance
	Level,				///< Player level
	RaceWins,			///< Total races won
	CarOwnership,		///< Owns specific car (by ID)
	CarTierOwnership,	///< Owns any car in a tier
	LocationUnlocked,	///< Specific location unlocked
	DistrictUnlocked,	///< District available
	ChallengeCompleted,	///< Specific challenge/achievement done
	HousingTier,		///< Housing tier reached
	PlayTime,			///< Hours played
	CarUsage,			///< Distance driven in specific car
	CustomizationCount	///< Number of customization items unlocked
};

// =============================================================================
// CORE STRUCTURES
// =============================================================================

/**
 * @brief Single prerequisite condition for unlocking content
 * 
 * Represents one requirement that must be met (e.g., "Rep >= 5000").
 * Multiple prerequisites are combined with AND logic.
 */
USTRUCT(BlueprintType)
struct FMGPrerequisite
{
	GENERATED_BODY()

	/** Type of requirement to check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	EMGPrerequisiteType Type = EMGPrerequisiteType::Reputation;

	/** Target identifier (car ID, location ID, etc.) - optional depending on Type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	FName TargetID = NAME_None;

	/** Required value/amount to satisfy this prerequisite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	int32 RequiredValue = 0;

	/** Enum value for tier checks (CarTier, HousingTier, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	int32 RequiredEnumValue = 0;

	FMGPrerequisite() = default;

	FMGPrerequisite(EMGPrerequisiteType InType, int32 InValue)
		: Type(InType), RequiredValue(InValue) {}

	FMGPrerequisite(EMGPrerequisiteType InType, FName InTarget, int32 InValue)
		: Type(InType), TargetID(InTarget), RequiredValue(InValue) {}
};

/**
 * @brief Group of prerequisites with OR logic
 * 
 * All prerequisites in a group are combined with OR.
 * Multiple groups are combined with AND.
 * Example: (Rep >= 5000 OR Money >= 100000) AND (Own Super Car)
 */
USTRUCT(BlueprintType)
struct FMGPrerequisiteGroup
{
	GENERATED_BODY()

	/** Prerequisites in this group (OR logic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	TArray<FMGPrerequisite> Prerequisites;
};

/**
 * @brief Complete unlock requirement with AND/OR logic
 * 
 * Structure:
 * - AllPrerequisites: All must be met (AND logic)
 * - PrerequisiteGroups: At least one prerequisite in EACH group must be met
 * 
 * Final logic: AllPrerequisites AND Group1(OR) AND Group2(OR) AND ...
 */
USTRUCT(BlueprintType)
struct FMGUnlockRequirement
{
	GENERATED_BODY()

	/** Prerequisites that ALL must be met (AND logic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FMGPrerequisite> AllPrerequisites;

	/** Prerequisite groups where at least ONE in each group must be met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FMGPrerequisiteGroup> PrerequisiteGroups;

	/** If true, this unlock is hidden until requirements are met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bHiddenUntilMet = false;
};

// =============================================================================
// PROGRESSION DATA STRUCTURES
// =============================================================================

/**
 * @brief Car ownership and usage tracking
 * 
 * Tracks owned cars with usage statistics for per-car progression
 * (unlocks performance parts as you drive each vehicle).
 */
USTRUCT(BlueprintType)
struct FMGOwnedCar
{
	GENERATED_BODY()

	/** Unique ID of the car model */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	FName CarID;

	/** Tier this car belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	EMGCarTier Tier = EMGCarTier::StreetBeaters;

	/** Total distance driven in this car (km) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	float DistanceDrivenKm = 0.0f;

	/** Number of races won with this car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	int32 RacesWon = 0;

	/** Performance upgrade stage unlocked (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	int32 PerformanceStage = 1;

	/** Timestamp when car was acquired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Car")
	FDateTime AcquiredAt;

	FMGOwnedCar() { AcquiredAt = FDateTime::Now(); }
};

/**
 * @brief Location unlock data
 * 
 * Tracks which locations/sub-areas are accessible to the player.
 */
USTRUCT(BlueprintType)
struct FMGUnlockedLocation
{
	GENERATED_BODY()

	/** Unique location identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Location")
	FName LocationID;

	/** District this location belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Location")
	EMGDistrict District = EMGDistrict::Industrial;

	/** When this location was unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Location")
	FDateTime UnlockedAt;

	/** Number of races completed at this location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Location")
	int32 RacesCompleted = 0;

	FMGUnlockedLocation() { UnlockedAt = FDateTime::Now(); }
};

/**
 * @brief Housing progression data
 * 
 * Tracks current housing tier and unlocked cosmetic upgrades.
 */
USTRUCT(BlueprintType)
struct FMGHousingData
{
	GENERATED_BODY()

	/** Current housing tier owned by player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Housing")
	EMGHousingTier CurrentTier = EMGHousingTier::Garage;

	/** Unlocked furniture/decoration items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Housing")
	TArray<FName> UnlockedCosmetics;

	/** Number of cars currently displayed in housing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Housing")
	int32 DisplayedCarCount = 0;

	/** Maximum cars that can be displayed at current tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Housing")
	int32 MaxDisplayCapacity = 1;
};

/**
 * @brief Customization unlock tracking
 * 
 * Tracks which customization items are unlocked per category.
 */
USTRUCT(BlueprintType)
struct FMGCustomizationProgress
{
	GENERATED_BODY()

	/** Category of customization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Customization")
	EMGCustomizationType Category = EMGCustomizationType::Paint;

	/** IDs of unlocked items in this category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Customization")
	TArray<FName> UnlockedItems;
};

/**
 * @brief Milestone tracking for "wow moments"
 * 
 * Represents major progression milestones that create memorable moments.
 */
USTRUCT(BlueprintType)
struct FMGMilestone
{
	GENERATED_BODY()

	/** Unique milestone identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FName MilestoneID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText DisplayName;

	/** Description of achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText Description;

	/** Target hour when this milestone should be reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 TargetHour = 0;

	/** Whether this milestone has been completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Milestone")
	bool bCompleted = false;

	/** Timestamp when completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Milestone")
	FDateTime CompletedAt;

	/** Notification tier for UI (1=small, 2=medium, 3=large cinematic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 NotificationTier = 2;
};

// =============================================================================
// DATA TABLE ROW STRUCTURES
// =============================================================================

/**
 * @brief Car unlock definition (Data Table row)
 * 
 * Defines a car that can be unlocked, its tier, cost, and requirements.
 */
USTRUCT(BlueprintType)
struct FMGCarUnlockData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique car identifier (matches vehicle data) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	FName CarID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	FText DisplayName;

	/** Car tier classification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	EMGCarTier Tier = EMGCarTier::StreetBeaters;

	/** Purchase price (0 = earned through progression, not bought) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	int64 PurchaseCost = 0;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	FMGUnlockRequirement Requirements;

	/** Whether this is a starter car (available immediately) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	bool bStarterCar = false;

	/** Whether this car is hidden until unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	bool bHiddenUntilUnlocked = false;
};

/**
 * @brief Location unlock definition (Data Table row)
 * 
 * Defines a location/district that can be unlocked.
 */
USTRUCT(BlueprintType)
struct FMGLocationUnlockData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique location identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FName LocationID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FText DisplayName;

	/** District this location belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	EMGDistrict District = EMGDistrict::Industrial;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FMGUnlockRequirement Requirements;

	/** Number of race types available here */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	int32 RaceCount = 0;

	/** Whether this is a starting location (available immediately) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	bool bStartingLocation = false;
};

/**
 * @brief Housing tier definition (Data Table row)
 * 
 * Defines a housing tier upgrade with requirements and features.
 */
USTRUCT(BlueprintType)
struct FMGHousingUnlockData : public FTableRowBase
{
	GENERATED_BODY()

	/** Housing tier identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	EMGHousingTier Tier = EMGHousingTier::Garage;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FText DisplayName;

	/** Description of features */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FText Description;

	/** Purchase/upgrade cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	int64 PurchaseCost = 0;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FMGUnlockRequirement Requirements;

	/** Maximum cars that can be displayed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	int32 MaxCarDisplay = 1;

	/** Cosmetic slots available (furniture, decorations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	int32 CosmeticSlots = 5;
};

/**
 * @brief Customization item unlock definition (Data Table row)
 * 
 * Defines paint colors, decals, body kits, etc. that can be unlocked.
 */
USTRUCT(BlueprintType)
struct FMGCustomizationUnlockData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique item identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName ItemID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FText DisplayName;

	/** Customization category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	EMGCustomizationType Category = EMGCustomizationType::Paint;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FMGUnlockRequirement Requirements;

	/** Purchase price (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 PurchaseCost = 0;

	/** Tier within category (for progressive unlocks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 TierLevel = 1;
};

/**
 * @brief Milestone definition (Data Table row)
 * 
 * Defines major progression milestones with rewards.
 */
USTRUCT(BlueprintType)
struct FMGMilestoneData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique milestone identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FName MilestoneID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText DisplayName;

	/** Description of achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText Description;

	/** Target hour when this should be reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 TargetHour = 0;

	/** Requirements to complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FMGUnlockRequirement Requirements;

	/** Reward type (Car, Money, Unlock, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FString RewardType;

	/** Reward identifier (car ID, item ID, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FName RewardID;

	/** Money reward amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int64 RewardMoney = 0;

	/** Notification tier (1=small, 2=medium, 3=large) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 NotificationTier = 2;
};
