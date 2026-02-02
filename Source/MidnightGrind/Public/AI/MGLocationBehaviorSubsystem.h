// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGLocationBehaviorSubsystem.h
 * @brief Location-Specific AI Behavior System
 *
 * @section overview Overview
 * Different racing locations demand different driving styles and tactics.
 * This subsystem applies location-specific behavioral modifiers to AI opponents,
 * creating varied racing experiences across street circuits, professional tracks,
 * mountain roads, and highways.
 *
 * Key Features:
 * - **Location Archetypes:** Street, Track, Touge, Highway, Industrial, Docks
 * - **Behavior Modifiers:** Risk tolerance, aggression, precision requirements
 * - **Environmental Adaptation:** Traffic awareness, weather sensitivity
 * - **Dynamic Application:** Modifiers applied at race start, removed after
 *
 * @section concepts Key Concepts
 *
 * @subsection archetypes Location Archetypes
 * Six distinct racing environments:
 * - **Street Racing:** Tight corners, traffic, obstacles, aggressive overtaking
 * - **Track Racing:** Wide circuits, optimal lines, clean racing, advanced racecraft
 * - **Touge (Mountain):** Narrow, dangerous, precision required, drift-focused
 * - **Highway Racing:** High speed, long straights, drafting mastery, traffic weaving
 * - **Industrial:** Mixed environment, tight + open sections, moderate traffic
 * - **Docks:** Wet surfaces, containers, technical corners, unique hazards
 *
 * @subsection modifiers Behavior Modifiers
 * Each location applies multipliers to:
 * - Risk tolerance (higher = more aggressive)
 * - Overtaking aggression
 * - Line accuracy requirements
 * - Look-ahead distance
 * - Contact tolerance
 * - Speed multipliers (corner vs straight)
 *
 * @section usage Usage Examples
 *
 * @subsection apply_location Applying Location Behavior
 * @code
 * UMGLocationBehaviorSubsystem* LocationSystem = 
 *     GetWorld()->GetSubsystem<UMGLocationBehaviorSubsystem>();
 *
 * // Apply location behavior to AI profile
 * UMGAIDriverProfile* Profile = LoadDriverProfile();
 * LocationSystem->ApplyLocationBehavior(
 *     Profile,
 *     EMGRaceLocationType::Street
 * );
 *
 * // Spawn AI with location-adapted profile
 * SpawnAIOpponent(Profile, SpawnTransform);
 * @endcode
 *
 * @subsection query_modifiers Querying Location Modifiers
 * @code
 * // Get modifiers for specific location
 * FMGLocationBehaviorModifiers Modifiers = 
 *     LocationSystem->GetLocationModifiers(EMGRaceLocationType::Touge);
 *
 * // Check if location requires special skills
 * bool bNeedsPrecision = Modifiers.RequiresPrecisionDriving;
 * bool bHasTraffic = Modifiers.HasTraffic;
 * @endcode
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AI/MG_AI_DriverProfile.h"
#include "MGLocationBehaviorSubsystem.generated.h"

/**
 * Race location type enumeration
 * Defines the type of racing environment
 */
UENUM(BlueprintType)
enum class EMGRaceLocationType : uint8
{
	/** Urban street racing - tight corners, traffic, aggressive */
	Street           UMETA(DisplayName = "Street Racing"),
	
	/** Professional racing circuit - wide tracks, optimal lines */
	Track            UMETA(DisplayName = "Track Racing"),
	
	/** Mountain/Touge roads - narrow, precision required */
	Touge            UMETA(DisplayName = "Touge/Mountain"),
	
	/** Highway racing - high speed, long straights */
	Highway          UMETA(DisplayName = "Highway Racing"),
	
	/** Industrial zones - mixed environment */
	Industrial       UMETA(DisplayName = "Industrial"),
	
	/** Dockyard areas - wet surfaces, containers */
	Docks            UMETA(DisplayName = "Docks")
};

/**
 * Location-specific behavior modifiers
 * Applied to AI profiles for location-appropriate behavior
 */
USTRUCT(BlueprintType)
struct FMGLocationBehaviorModifiers
{
	GENERATED_BODY()

	// ==========================================
	// AGGRESSION & RISK
	// ==========================================

	/** Risk tolerance multiplier (0.5-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float RiskToleranceMultiplier = 1.0f;

	/** Overtake aggression multiplier (0.6-1.6) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.6", ClampMax = "1.6"))
	float OvertakeAggressionMultiplier = 1.0f;

	/** Defense aggression multiplier (0.6-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.6", ClampMax = "1.5"))
	float DefenseAggressionMultiplier = 1.0f;

	/** Contact tolerance multiplier (0.5-1.5, higher = accepts more contact) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float ContactToleranceMultiplier = 1.0f;

	// ==========================================
	// PRECISION & SKILL
	// ==========================================

	/** Line accuracy multiplier (0.7-1.5, higher = tighter lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "0.7", ClampMax = "1.5"))
	float LineAccuracyMultiplier = 1.0f;

	/** Braking precision multiplier (0.7-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "0.7", ClampMax = "1.5"))
	float BrakingPrecisionMultiplier = 1.0f;

	/** Cornering skill multiplier (0.7-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "0.7", ClampMax = "1.5"))
	float CorneringSkillMultiplier = 1.0f;

	/** Consistency multiplier (0.7-1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "0.7", ClampMax = "1.3"))
	float ConsistencyMultiplier = 1.0f;

	// ==========================================
	// PERCEPTION & AWARENESS
	// ==========================================

	/** Look-ahead distance multiplier (0.6-1.4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness", meta = (ClampMin = "0.6", ClampMax = "1.4"))
	float LookAheadDistanceMultiplier = 1.0f;

	/** Awareness multiplier (0.7-1.4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness", meta = (ClampMin = "0.7", ClampMax = "1.4"))
	float AwarenessMultiplier = 1.0f;

	/** Reaction time multiplier (0.7-1.3, higher = slower) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness", meta = (ClampMin = "0.7", ClampMax = "1.3"))
	float ReactionTimeMultiplier = 1.0f;

	// ==========================================
	// SPEED & PERFORMANCE
	// ==========================================

	/** Top speed multiplier (0.9-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.9", ClampMax = "1.2"))
	float TopSpeedMultiplier = 1.0f;

	/** Corner speed multiplier (0.8-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.8", ClampMax = "1.2"))
	float CornerSpeedMultiplier = 1.0f;

	/** Straight speed multiplier (0.9-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.9", ClampMax = "1.2"))
	float StraightSpeedMultiplier = 1.0f;

	// ==========================================
	// TACTICAL ADJUSTMENTS
	// ==========================================

	/** Defensive driving skill multiplier (0.6-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics", meta = (ClampMin = "0.6", ClampMax = "1.5"))
	float DefensiveSkillMultiplier = 1.0f;

	/** Drafting/slipstream skill multiplier (0.5-1.8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics", meta = (ClampMin = "0.5", ClampMax = "1.8"))
	float DraftingSkillMultiplier = 1.0f;

	/** NOS usage frequency multiplier (0.6-1.6) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics", meta = (ClampMin = "0.6", ClampMax = "1.6"))
	float NOSUsageFrequency = 1.0f;

	// ==========================================
	// ENVIRONMENT-SPECIFIC
	// ==========================================

	/** Traffic weaving skill (0-1, 0 = no traffic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrafficWeavingSkill = 0.0f;

	/** Drifting preference (0-1, 0 = grip racing, 1 = drift-focused) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DriftingPreference = 0.0f;

	/** Mistake consequence severity (1.0-3.0, higher = mistakes cost more) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float MistakeConsequenceSeverity = 1.0f;

	/** Wet weather adaptation (0-1, higher = better in wet) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WetWeatherAdaptation = 0.0f;

	// ==========================================
	// FLAGS
	// ==========================================

	/** Location has traffic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bHasTraffic = false;

	/** Requires precision driving (Touge, technical tracks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bRequiresPrecisionDriving = false;

	/** Supports drafting (long straights) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bSupportsDrafting = true;

	/** Contact-heavy environment (street racing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bContactHeavy = false;

	/** High-speed environment (highway, track) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bHighSpeed = false;
};

/**
 * Location profile data asset
 * Defines all parameters for a specific location type
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGLocationProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Location type identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Location")
	EMGRaceLocationType LocationType = EMGRaceLocationType::Street;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Location")
	FText LocationName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Location", meta = (MultiLine = true))
	FText Description;

	/** Behavior modifiers for this location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Location")
	FMGLocationBehaviorModifiers Modifiers;

	/** Icon for location type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Location")
	TObjectPtr<UTexture2D> Icon;
};

/**
 * Location Behavior Subsystem
 * Applies location-appropriate behavior to AI opponents
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLocationBehaviorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// INITIALIZATION
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// LOCATION QUERIES
	// ==========================================

	/**
	 * Get behavior modifiers for location type
	 * @param LocationType The location type
	 * @return Modifiers for this location
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	FMGLocationBehaviorModifiers GetLocationModifiers(EMGRaceLocationType LocationType) const;

	/**
	 * Get location profile data asset
	 * @param LocationType The location type
	 * @return Profile asset, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	UMGLocationProfile* GetLocationProfile(EMGRaceLocationType LocationType) const;

	/**
	 * Get current location type (if set)
	 * @return Current location type, or Street if not set
	 */
	UFUNCTION(BlueprintPure, Category = "Location Behavior")
	EMGRaceLocationType GetCurrentLocationType() const { return CurrentLocationType; }

	// ==========================================
	// LOCATION APPLICATION
	// ==========================================

	/**
	 * Apply location behavior to AI profile
	 * Modifies profile in-place for location-specific racing
	 * @param Profile Profile to modify
	 * @param LocationType Location type
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	void ApplyLocationBehavior(
		UMGAIDriverProfile* Profile,
		EMGRaceLocationType LocationType
	);

	/**
	 * Remove location modifiers from profile
	 * Resets profile to base values
	 * @param Profile Profile to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	void RemoveLocationBehavior(UMGAIDriverProfile* Profile);

	/**
	 * Set current race location type
	 * Affects all subsequently spawned AI
	 * @param LocationType The location type
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	void SetCurrentLocation(EMGRaceLocationType LocationType);

	// ==========================================
	// BATCH OPERATIONS
	// ==========================================

	/**
	 * Apply location behavior to multiple profiles
	 * @param Profiles Array of profiles to modify
	 * @param LocationType Location type
	 */
	UFUNCTION(BlueprintCallable, Category = "Location Behavior")
	void ApplyLocationBehaviorToProfiles(
		TArray<UMGAIDriverProfile*> Profiles,
		EMGRaceLocationType LocationType
	);

	// ==========================================
	// UTILITY
	// ==========================================

	/**
	 * Get location name as text
	 * @param LocationType The location type
	 * @return Display name
	 */
	UFUNCTION(BlueprintPure, Category = "Location Behavior")
	FText GetLocationName(EMGRaceLocationType LocationType) const;

	/**
	 * Get location description
	 * @param LocationType The location type
	 * @return Description text
	 */
	UFUNCTION(BlueprintPure, Category = "Location Behavior")
	FText GetLocationDescription(EMGRaceLocationType LocationType) const;

	/**
	 * Check if location has traffic
	 * @param LocationType The location type
	 * @return True if location has traffic
	 */
	UFUNCTION(BlueprintPure, Category = "Location Behavior")
	bool LocationHasTraffic(EMGRaceLocationType LocationType) const;

	/**
	 * Check if location requires precision driving
	 * @param LocationType The location type
	 * @return True if precision required
	 */
	UFUNCTION(BlueprintPure, Category = "Location Behavior")
	bool LocationRequiresPrecision(EMGRaceLocationType LocationType) const;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Location profile data assets (loaded from config) */
	UPROPERTY()
	TMap<EMGRaceLocationType, TObjectPtr<UMGLocationProfile>> LocationProfiles;

	/** Cache of default location modifiers */
	UPROPERTY()
	TMap<EMGRaceLocationType, FMGLocationBehaviorModifiers> DefaultModifiers;

	/** Current race location type */
	UPROPERTY()
	EMGRaceLocationType CurrentLocationType = EMGRaceLocationType::Street;

	/** Track original profile values for reverting */
	UPROPERTY()
	TMap<TObjectPtr<UMGAIDriverProfile>, FMGAISkillParams> OriginalSkillParams;

	UPROPERTY()
	TMap<TObjectPtr<UMGAIDriverProfile>, FMGAIAggressionParams> OriginalAggressionParams;

	UPROPERTY()
	TMap<TObjectPtr<UMGAIDriverProfile>, FMGAISpeedParams> OriginalSpeedParams;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Load location profile data assets */
	void LoadLocationProfiles();

	/** Create default modifier sets */
	void InitializeDefaultModifiers();

	/** Apply modifiers to skill parameters */
	void ApplySkillModifiers(
		FMGAISkillParams& Skill,
		const FMGLocationBehaviorModifiers& Modifiers
	) const;

	/** Apply modifiers to aggression parameters */
	void ApplyAggressionModifiers(
		FMGAIAggressionParams& Aggression,
		const FMGLocationBehaviorModifiers& Modifiers
	) const;

	/** Apply modifiers to speed parameters */
	void ApplySpeedModifiers(
		FMGAISpeedParams& Speed,
		const FMGLocationBehaviorModifiers& Modifiers
	) const;

	/** Apply modifiers to racecraft parameters */
	void ApplyRacecraftModifiers(
		FMGAIRacecraftParams& Racecraft,
		const FMGLocationBehaviorModifiers& Modifiers
	) const;

	/** Store original profile values */
	void StoreOriginalValues(UMGAIDriverProfile* Profile);

	/** Restore original profile values */
	void RestoreOriginalValues(UMGAIDriverProfile* Profile);
};
