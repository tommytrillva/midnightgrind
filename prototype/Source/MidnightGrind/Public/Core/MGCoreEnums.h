// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCoreEnums.h
 * @brief Shared enum definitions used across multiple subsystems
 *
 * This file contains enum definitions that are used by multiple subsystems.
 * Instead of duplicating enums, all subsystems should include this header.
 */

#pragma once

#include "CoreMinimal.h"
#include "MGCoreEnums.generated.h"

// ============================================================================
// DRIVETRAIN TYPES
// ============================================================================

/**
 * Vehicle drivetrain configuration
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	/** Front-Wheel Drive: Engine in front, power to front wheels. Understeer tendency. */
	FWD			UMETA(DisplayName = "Front-Wheel Drive"),
	/** Rear-Wheel Drive: Engine in front, power to rear wheels. Classic sports car layout. */
	RWD			UMETA(DisplayName = "Rear-Wheel Drive"),
	/** All-Wheel Drive: Power to all wheels. Best traction. */
	AWD			UMETA(DisplayName = "All-Wheel Drive"),
	/** Mid-Engine RWD: Engine behind driver, rear drive. Balanced weight. */
	MR			UMETA(DisplayName = "Mid-Engine RWD"),
	/** Rear-Engine RWD: Engine at rear, rear drive. Unique handling. */
	RR			UMETA(DisplayName = "Rear-Engine RWD"),
	/** Full-Time 4WD: Permanent 4-wheel drive with center differential. */
	F4WD		UMETA(DisplayName = "Full-Time 4WD")
};

// ============================================================================
// VEHICLE ERA
// ============================================================================

/**
 * Vehicle era/generation
 */
UENUM(BlueprintType)
enum class EMGVehicleEra : uint8
{
	Classic		UMETA(DisplayName = "Classic (Pre-1980)"),
	Retro		UMETA(DisplayName = "Retro (1980-1999)"),
	Modern		UMETA(DisplayName = "Modern (2000-2015)"),
	Current		UMETA(DisplayName = "Current (2015+)"),
	Future		UMETA(DisplayName = "Future Concept")
};

// ============================================================================
// TIME OF DAY
// ============================================================================

/**
 * Time of day states for visual/gameplay changes
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn"),
	Morning		UMETA(DisplayName = "Morning"),
	Noon		UMETA(DisplayName = "Noon"),
	Afternoon	UMETA(DisplayName = "Afternoon"),
	Dusk		UMETA(DisplayName = "Dusk"),
	Night		UMETA(DisplayName = "Night"),
	Midnight	UMETA(DisplayName = "Midnight")
};

// ============================================================================
// WEATHER TYPES
// ============================================================================

/**
 * Weather conditions
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Cloudy		UMETA(DisplayName = "Cloudy"),
	Overcast	UMETA(DisplayName = "Overcast"),
	LightRain	UMETA(DisplayName = "Light Rain"),
	HeavyRain	UMETA(DisplayName = "Heavy Rain"),
	Storm		UMETA(DisplayName = "Storm"),
	Fog			UMETA(DisplayName = "Fog"),
	Snow		UMETA(DisplayName = "Snow")
};

// ============================================================================
// MUTE REASONS
// ============================================================================

/**
 * Reasons for muting a player
 */
UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
	None		UMETA(DisplayName = "Not Muted"),
	Manual		UMETA(DisplayName = "Manual Mute"),
	Reported	UMETA(DisplayName = "Reported"),
	Toxic		UMETA(DisplayName = "Toxic Behavior"),
	Spam		UMETA(DisplayName = "Spam"),
	System		UMETA(DisplayName = "System Mute")
};
