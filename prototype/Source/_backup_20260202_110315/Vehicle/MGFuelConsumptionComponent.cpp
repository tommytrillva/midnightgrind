// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGFuelConsumptionComponent.cpp
 * @brief Implementation of realistic fuel consumption simulation
 */

#include "Vehicle/MGFuelConsumptionComponent.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Fuel/MG_FUEL_Subsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UMGFuelConsumptionComponent::UMGFuelConsumptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Initialize history buffers
	ThrottleHistory.SetNum(ThrottleHistorySamples);
	ConsumptionHistory.SetNum(ConsumptionHistorySamples);
}

void UMGFuelConsumptionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache movement component reference
	if (AActor* Owner = GetOwner())
	{
		MovementComponent = Owner->FindComponentByClass<UMGVehicleMovementComponent>();
	}

	// Store initial fuel weight for delta calculation
	InitialFuelWeightKg = TankConfig.GetFuelWeightKg();
	PreviousFuelPercentage = TankConfig.GetFuelPercentage();

	// Reset warnings for new session
	bLowWarningTriggered = false;
	bCriticalWarningTriggered = false;
	bEmptyEventTriggered = false;

	UE_LOG(LogTemp, Log, TEXT("FuelConsumption: Initialized with %.2f gallons (%.1f kg)"),
		TankConfig.CurrentFuelGallons, InitialFuelWeightKg);
}

void UMGFuelConsumptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Skip if no movement component or engine isn't running
	if (!MovementComponent)
	{
		return;
	}

	// Skip if already empty
	if (IsEmpty())
	{
		if (!bEmptyEventTriggered)
		{
			bEmptyEventTriggered = true;
			OnFuelEmpty.Broadcast();
		}
		// Update starvation to full severity when empty
		StarvationState.bIsStarving = true;
		StarvationState.StarvationSeverity = 1.0f;
		return;
	}

	// Update driving style analysis
	UpdateDrivingStyleMetrics(DeltaTime);

	// Calculate and apply fuel consumption
	const float FrameConsumption = CalculateFrameConsumption(DeltaTime);
	if (FrameConsumption > 0.0f)
	{
		ConsumeFuel(FrameConsumption);
	}

	// Update starvation simulation
	UpdateFuelStarvation(DeltaTime);

	// Update telemetry
	UpdateTelemetry(DeltaTime, FrameConsumption);

	// Update weight effects on vehicle
	UpdateWeightEffects();

	// Check for fuel warnings
	CheckFuelWarnings();
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGFuelConsumptionComponent::SetTankConfiguration(const FMGFuelTankConfiguration& Configuration)
{
	TankConfig = Configuration;
	InitialFuelWeightKg = TankConfig.GetFuelWeightKg();
	PreviousFuelPercentage = TankConfig.GetFuelPercentage();

	// Reset warning states
	bLowWarningTriggered = TankConfig.IsAtReserve();
	bCriticalWarningTriggered = TankConfig.IsCritical();
	bEmptyEventTriggered = IsEmpty();
}

void UMGFuelConsumptionComponent::SetBaseConsumptionRate(float GallonsPerHour)
{
	IdleConsumptionGPH = FMath::Clamp(GallonsPerHour, 0.1f, 2.0f);
}

// ==========================================
// FUEL OPERATIONS
// ==========================================

float UMGFuelConsumptionComponent::ConsumeFuel(float GallonsToConsume)
{
	if (GallonsToConsume <= 0.0f || IsEmpty())
	{
		return 0.0f;
	}

	const float PreviousFuel = TankConfig.CurrentFuelGallons;
	TankConfig.CurrentFuelGallons = FMath::Max(0.0f, TankConfig.CurrentFuelGallons - GallonsToConsume);

	const float ActualConsumed = PreviousFuel - TankConfig.CurrentFuelGallons;

	// Track session consumption
	Telemetry.SessionFuelConsumed += ActualConsumed;

	// Broadcast significant level changes (every 5%)
	const float CurrentPercentage = TankConfig.GetFuelPercentage();
	if (FMath::Abs(CurrentPercentage - PreviousFuelPercentage) >= 0.05f)
	{
		PreviousFuelPercentage = CurrentPercentage;
		OnFuelLevelChanged.Broadcast(TankConfig.CurrentFuelGallons, CurrentPercentage);
	}

	return ActualConsumed;
}

float UMGFuelConsumptionComponent::AddFuel(float GallonsToAdd)
{
	if (GallonsToAdd <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousFuel = TankConfig.CurrentFuelGallons;
	TankConfig.CurrentFuelGallons = FMath::Min(TankConfig.CapacityGallons, TankConfig.CurrentFuelGallons + GallonsToAdd);

	const float ActualAdded = TankConfig.CurrentFuelGallons - PreviousFuel;

	// Reset warning states if fuel is above thresholds
	if (!TankConfig.IsAtReserve())
	{
		bLowWarningTriggered = false;
	}
	if (!TankConfig.IsCritical())
	{
		bCriticalWarningTriggered = false;
	}
	bEmptyEventTriggered = false;

	// Broadcast fuel level change
	OnFuelLevelChanged.Broadcast(TankConfig.CurrentFuelGallons, TankConfig.GetFuelPercentage());

	return ActualAdded;
}

float UMGFuelConsumptionComponent::FillTank()
{
	const float GallonsNeeded = TankConfig.CapacityGallons - TankConfig.CurrentFuelGallons;
	return AddFuel(GallonsNeeded);
}

bool UMGFuelConsumptionComponent::PurchaseFuel(float GallonsToAdd, bool bFullTank)
{
	// Get economy subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	UMGEconomySubsystem* EconomySubsystem = GameInstance->GetSubsystem<UMGEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return false;
	}

	// Calculate amount to add
	float ActualGallonsToAdd = bFullTank ?
		(TankConfig.CapacityGallons - TankConfig.CurrentFuelGallons) :
		FMath::Min(GallonsToAdd, TankConfig.CapacityGallons - TankConfig.CurrentFuelGallons);

	if (ActualGallonsToAdd <= 0.0f)
	{
		return true; // Tank is already full
	}

	// Calculate cost
	const int64 Cost = GetRefuelCost(ActualGallonsToAdd);

	// Check if player can afford
	if (!EconomySubsystem->CanAfford(Cost))
	{
		UE_LOG(LogTemp, Warning, TEXT("FuelConsumption: Cannot afford refuel - Cost: %lld, Balance: %lld"),
			Cost, EconomySubsystem->GetCredits());
		return false;
	}

	// Process purchase
	FText Description = FText::Format(
		FText::FromString(TEXT("Purchased {0} gallons of fuel")),
		FText::AsNumber(FMath::RoundToInt(ActualGallonsToAdd))
	);

	if (!EconomySubsystem->SpendCredits(Cost, EMGTransactionType::FuelPurchase, Description))
	{
		return false;
	}

	// Add fuel
	const float GallonsAdded = AddFuel(ActualGallonsToAdd);

	// Broadcast refuel complete
	OnRefuelComplete.Broadcast(GallonsAdded, Cost);

	UE_LOG(LogTemp, Log, TEXT("FuelConsumption: Purchased %.2f gallons for %lld credits"),
		GallonsAdded, Cost);

	return true;
}

int64 UMGFuelConsumptionComponent::GetRefuelCost(float GallonsToAdd) const
{
	// Base cost calculation
	float PriceMultiplier = 1.0f;

	// Could check fuel type here for premium/racing fuel multiplier
	// For now, use base price

	return static_cast<int64>(FMath::CeilToInt(GallonsToAdd * FuelPricePerGallon * PriceMultiplier));
}

int64 UMGFuelConsumptionComponent::GetFillTankCost() const
{
	const float GallonsNeeded = TankConfig.CapacityGallons - TankConfig.CurrentFuelGallons;
	return GetRefuelCost(GallonsNeeded);
}

void UMGFuelConsumptionComponent::ResetSessionTracking()
{
	Telemetry.SessionFuelConsumed = 0.0f;
	Telemetry.SessionDistanceMiles = 0.0f;
	Telemetry.AverageGPH = 0.0f;
	Telemetry.AverageMPG = 0.0f;

	StarvationState.TotalStarvationTime = 0.0f;
	StarvationState.StarvationEventCount = 0;

	DrivingStyle = FMGDrivingStyleMetrics();

	HardAccelTimer = 0.0f;
	HardAccelCount = 0;

	// Reset history buffers
	for (int32 i = 0; i < ThrottleHistorySamples; ++i)
	{
		ThrottleHistory[i] = 0.0f;
	}
	for (int32 i = 0; i < ConsumptionHistorySamples; ++i)
	{
		ConsumptionHistory[i] = 0.0f;
	}
	ThrottleHistoryIndex = 0;
	ConsumptionHistoryIndex = 0;
}

// ==========================================
// POWER REDUCTION
// ==========================================

float UMGFuelConsumptionComponent::GetFuelStarvationPowerMultiplier() const
{
	if (IsEmpty())
	{
		return 0.0f;
	}
	return StarvationState.GetPowerReductionFactor();
}

// ==========================================
// INTERNAL UPDATE METHODS
// ==========================================

float UMGFuelConsumptionComponent::CalculateFrameConsumption(float DeltaTime)
{
	if (!MovementComponent)
	{
		return 0.0f;
	}

	const FMGEngineState& EngineState = MovementComponent->GetEngineState();
	const FMGVehicleData& VehicleConfig = MovementComponent->GetVehicleConfiguration();

	// Base consumption at idle
	float ConsumptionGPH = IdleConsumptionGPH;

	// ==========================================
	// THROTTLE FACTOR
	// ==========================================
	// Consumption scales with throttle position
	// Uses a quadratic curve for more realistic behavior
	const float ThrottlePosition = EngineState.ThrottlePosition;
	const float ThrottleFactor = 1.0f + FMath::Pow(ThrottlePosition, 1.5f) * (WOTConsumptionMultiplier - 1.0f);
	ConsumptionGPH *= ThrottleFactor;

	// ==========================================
	// RPM FACTOR
	// ==========================================
	// Higher RPM increases consumption
	const float RPMNormalized = EngineState.CurrentRPM / FMath::Max(1.0f, (float)VehicleConfig.Stats.Redline);
	const float RPMFactor = 1.0f + RPMNormalized * RPMConsumptionFactor;
	ConsumptionGPH *= RPMFactor;

	// ==========================================
	// BOOST/TURBO FACTOR
	// ==========================================
	// Forced induction significantly increases fuel consumption
	if (EngineState.CurrentBoostPSI > 0.0f)
	{
		const float BoostFactor = 1.0f + (EngineState.CurrentBoostPSI * BoostConsumptionPerPSI);
		ConsumptionGPH *= BoostFactor;
	}

	// ==========================================
	// NITROUS FACTOR
	// ==========================================
	// Nitrous requires significantly more fuel (wet system mixes fuel)
	if (EngineState.bNitrousActive)
	{
		ConsumptionGPH *= NitrousConsumptionMultiplier;
	}

	// ==========================================
	// DRIVING STYLE FACTOR
	// ==========================================
	// Aggressive driving increases consumption
	const float AggressionPenalty = DrivingStyle.AggressionScore * MaxAggressionPenalty;
	ConsumptionGPH *= (1.0f + AggressionPenalty);

	// ==========================================
	// ENGINE LOAD FACTOR
	// ==========================================
	// High engine load (climbing hills, heavy car) increases consumption
	const float LoadFactor = 1.0f + EngineState.EngineLoad * 0.2f;
	ConsumptionGPH *= LoadFactor;

	// ==========================================
	// OVERHEATING FACTOR
	// ==========================================
	// Overheating engine runs rich (more fuel) for cooling
	if (EngineState.bOverheating)
	{
		ConsumptionGPH *= 1.15f; // 15% more fuel when overheating
	}

	// ==========================================
	// ANTI-LAG FACTOR
	// ==========================================
	// Anti-lag systems waste fuel to keep turbo spooled
	if (EngineState.bAntiLagActive)
	{
		ConsumptionGPH *= 1.5f; // Anti-lag is fuel-hungry
	}

	// Store instantaneous consumption for telemetry
	Telemetry.InstantGPH = ConsumptionGPH;

	// Convert GPH to gallons consumed this frame
	// GPH / 3600 = gallons per second * deltaTime = gallons this frame
	const float GallonsThisFrame = (ConsumptionGPH / 3600.0f) * DeltaTime;

	return GallonsThisFrame;
}

void UMGFuelConsumptionComponent::UpdateDrivingStyleMetrics(float DeltaTime)
{
	if (!MovementComponent)
	{
		return;
	}

	const FMGEngineState& EngineState = MovementComponent->GetEngineState();
	const float CurrentThrottle = EngineState.ThrottlePosition;

	// Update throttle history (circular buffer)
	ThrottleHistory[ThrottleHistoryIndex] = CurrentThrottle;
	ThrottleHistoryIndex = (ThrottleHistoryIndex + 1) % ThrottleHistorySamples;

	// Calculate average throttle and variance
	float ThrottleSum = 0.0f;
	float ThrottleSqSum = 0.0f;
	float WOTSamples = 0.0f;

	for (int32 i = 0; i < ThrottleHistorySamples; ++i)
	{
		const float Sample = ThrottleHistory[i];
		ThrottleSum += Sample;
		ThrottleSqSum += Sample * Sample;
		if (Sample >= 0.95f) // WOT threshold
		{
			WOTSamples += 1.0f;
		}
	}

	DrivingStyle.AverageThrottle = ThrottleSum / ThrottleHistorySamples;
	const float MeanSq = ThrottleSqSum / ThrottleHistorySamples;
	DrivingStyle.ThrottleVariance = MeanSq - (DrivingStyle.AverageThrottle * DrivingStyle.AverageThrottle);
	DrivingStyle.WOTPercentage = WOTSamples / ThrottleHistorySamples;

	// Track hard accelerations (sudden throttle application)
	static float PreviousThrottle = 0.0f;
	const float ThrottleDelta = CurrentThrottle - PreviousThrottle;
	if (ThrottleDelta > 0.5f && CurrentThrottle >= 0.8f)
	{
		HardAccelCount++;
	}
	PreviousThrottle = CurrentThrottle;

	// Update hard acceleration rate (per minute)
	HardAccelTimer += DeltaTime;
	if (HardAccelTimer >= 60.0f)
	{
		DrivingStyle.HardAccelerationsPerMinute = static_cast<float>(HardAccelCount);
		HardAccelCount = 0;
		HardAccelTimer = 0.0f;
	}
	else if (HardAccelTimer > 10.0f) // Start calculating after 10 seconds
	{
		DrivingStyle.HardAccelerationsPerMinute = (HardAccelCount / HardAccelTimer) * 60.0f;
	}

	// Calculate aggression score
	// Weighted combination of metrics
	const float ThrottleAggressionWeight = 0.3f;
	const float VarianceAggressionWeight = 0.25f;
	const float WOTAggressionWeight = 0.25f;
	const float HardAccelAggressionWeight = 0.2f;

	// Normalize hard accelerations (0-10/min = 0-1 aggression)
	const float NormalizedHardAccel = FMath::Clamp(DrivingStyle.HardAccelerationsPerMinute / 10.0f, 0.0f, 1.0f);

	DrivingStyle.AggressionScore =
		(DrivingStyle.AverageThrottle * ThrottleAggressionWeight) +
		(FMath::Sqrt(DrivingStyle.ThrottleVariance) * 2.0f * VarianceAggressionWeight) + // Variance 0-0.25 maps to 0-1
		(DrivingStyle.WOTPercentage * WOTAggressionWeight) +
		(NormalizedHardAccel * HardAccelAggressionWeight);

	DrivingStyle.AggressionScore = FMath::Clamp(DrivingStyle.AggressionScore, 0.0f, 1.0f);

	// Calculate consumption multiplier from style
	DrivingStyle.StyleConsumptionMultiplier = 1.0f + (DrivingStyle.AggressionScore * MaxAggressionPenalty);
}

void UMGFuelConsumptionComponent::UpdateFuelStarvation(float DeltaTime)
{
	// Get current lateral G-force
	const float LateralG = GetLateralGForce();
	StarvationState.LateralGForce = LateralG;

	// Calculate starvation threshold based on current fuel level
	const float GThreshold = CalculateStarvationThreshold();
	StarvationState.StarvationThresholdG = GThreshold;

	// Check if starvation conditions are met
	const bool bShouldStarve = (LateralG > GThreshold) && TankConfig.GetFuelPercentage() < 0.5f;

	if (bShouldStarve)
	{
		if (!StarvationState.bIsStarving)
		{
			// Starvation just started
			StarvationState.bIsStarving = true;
			StarvationState.StarvationEventCount++;
			OnFuelStarvationStarted.Broadcast(LateralG, TankConfig.GetFuelPercentage());

			UE_LOG(LogTemp, Warning, TEXT("FuelConsumption: Fuel starvation started - Lateral G: %.2f, Threshold: %.2f, Fuel: %.1f%%"),
				LateralG, GThreshold, TankConfig.GetFuelPercentage() * 100.0f);
		}

		// Build up starvation severity
		const float ExcessG = LateralG - GThreshold;
		const float TargetSeverity = FMath::Clamp(ExcessG / 0.5f, 0.0f, 1.0f); // Full starvation at 0.5G over threshold
		StarvationState.StarvationSeverity = FMath::FInterpTo(
			StarvationState.StarvationSeverity,
			TargetSeverity,
			DeltaTime,
			StarvationBuildupRate
		);

		StarvationState.TotalStarvationTime += DeltaTime;
	}
	else
	{
		if (StarvationState.bIsStarving && StarvationState.StarvationSeverity < 0.05f)
		{
			// Starvation ended
			StarvationState.bIsStarving = false;
			OnFuelStarvationEnded.Broadcast();

			UE_LOG(LogTemp, Log, TEXT("FuelConsumption: Fuel starvation ended"));
		}

		// Recover from starvation
		StarvationState.StarvationSeverity = FMath::FInterpTo(
			StarvationState.StarvationSeverity,
			0.0f,
			DeltaTime,
			StarvationRecoveryRate
		);

		if (StarvationState.StarvationSeverity < 0.01f)
		{
			StarvationState.bIsStarving = false;
			StarvationState.StarvationSeverity = 0.0f;
		}
	}

	// Apply starvation effect to movement component
	if (MovementComponent)
	{
		const float PowerMultiplier = StarvationState.GetPowerReductionFactor();
		MovementComponent->SetFuelStarvationMultiplier(PowerMultiplier);
	}
}

void UMGFuelConsumptionComponent::UpdateTelemetry(float DeltaTime, float FrameConsumption)
{
	// Update consumption history
	ConsumptionHistory[ConsumptionHistoryIndex] = FrameConsumption;
	ConsumptionHistoryIndex = (ConsumptionHistoryIndex + 1) % ConsumptionHistorySamples;

	// Calculate average consumption (GPH)
	float TotalConsumption = 0.0f;
	for (int32 i = 0; i < ConsumptionHistorySamples; ++i)
	{
		TotalConsumption += ConsumptionHistory[i];
	}
	// Average gallons per frame * frames per second * 3600 = GPH
	const float AvgGallonsPerFrame = (ConsumptionHistorySamples > 0) ? TotalConsumption / ConsumptionHistorySamples : 0.0f;
	Telemetry.AverageGPH = (AvgGallonsPerFrame / FMath::Max(0.001f, DeltaTime)) * 3600.0f;

	// Update distance tracking
	if (MovementComponent)
	{
		const float SpeedMPH = MovementComponent->GetSpeedMPH();
		const float MilesTraveled = (SpeedMPH / 3600.0f) * DeltaTime;
		Telemetry.SessionDistanceMiles += MilesTraveled;

		// Calculate MPG
		if (FrameConsumption > KINDA_SMALL_NUMBER)
		{
			Telemetry.InstantMPG = MilesTraveled / FrameConsumption;
			// Clamp to reasonable range
			Telemetry.InstantMPG = FMath::Clamp(Telemetry.InstantMPG, 0.0f, 100.0f);
		}
		else if (SpeedMPH < 1.0f)
		{
			Telemetry.InstantMPG = 0.0f; // Idling/stopped
		}

		// Average MPG
		if (Telemetry.SessionFuelConsumed > KINDA_SMALL_NUMBER)
		{
			Telemetry.AverageMPG = Telemetry.SessionDistanceMiles / Telemetry.SessionFuelConsumed;
			Telemetry.AverageMPG = FMath::Clamp(Telemetry.AverageMPG, 0.0f, 100.0f);
		}
	}

	// Calculate estimated range
	if (Telemetry.AverageGPH > KINDA_SMALL_NUMBER && MovementComponent)
	{
		const float SpeedMPH = MovementComponent->GetSpeedMPH();
		if (SpeedMPH > 5.0f && Telemetry.AverageMPG > 0.0f)
		{
			// Range = fuel * MPG
			Telemetry.EstimatedRangeMiles = TankConfig.CurrentFuelGallons * Telemetry.AverageMPG;
		}
		else if (Telemetry.AverageGPH > 0.1f)
		{
			// At idle/low speed, estimate based on time remaining
			const float HoursRemaining = TankConfig.CurrentFuelGallons / Telemetry.AverageGPH;
			Telemetry.EstimatedRangeMiles = HoursRemaining * 30.0f; // Assume 30 mph average when moving
		}
	}
}

void UMGFuelConsumptionComponent::UpdateWeightEffects()
{
	const float CurrentWeightKg = TankConfig.GetFuelWeightKg();
	const float WeightReduction = InitialFuelWeightKg - CurrentWeightKg;

	// Only broadcast if there's been a meaningful change (>0.5 kg)
	static float LastBroadcastWeight = 0.0f;
	if (FMath::Abs(CurrentWeightKg - LastBroadcastWeight) > 0.5f)
	{
		LastBroadcastWeight = CurrentWeightKg;
		OnFuelWeightChanged.Broadcast(CurrentWeightKg, WeightReduction);

		// Update movement component with new fuel weight
		if (MovementComponent)
		{
			MovementComponent->SetCurrentFuelWeightKg(CurrentWeightKg);
		}
	}
}

void UMGFuelConsumptionComponent::CheckFuelWarnings()
{
	// Check for low fuel warning
	if (TankConfig.IsAtReserve() && !bLowWarningTriggered)
	{
		bLowWarningTriggered = true;
		OnFuelLowWarning.Broadcast(TankConfig.CurrentFuelGallons);

		UE_LOG(LogTemp, Warning, TEXT("FuelConsumption: Low fuel warning - %.2f gallons remaining"),
			TankConfig.CurrentFuelGallons);
	}

	// Check for critical fuel warning
	if (TankConfig.IsCritical() && !bCriticalWarningTriggered)
	{
		bCriticalWarningTriggered = true;
		OnFuelCriticalWarning.Broadcast(TankConfig.CurrentFuelGallons);

		UE_LOG(LogTemp, Warning, TEXT("FuelConsumption: CRITICAL fuel warning - %.2f gallons remaining"),
			TankConfig.CurrentFuelGallons);
	}
}

float UMGFuelConsumptionComponent::CalculateStarvationThreshold() const
{
	// Starvation threshold decreases as fuel level decreases
	// At 50% fuel: No starvation possible (threshold = 999G)
	// At 25% fuel: Starvation at BaseGThreshold (e.g., 1.2G)
	// At 10% fuel: Starvation at lower G (0.8G)
	// At 5% fuel: Starvation at even lower G (0.5G)

	const float FuelPercent = TankConfig.GetFuelPercentage();

	if (FuelPercent >= 0.5f)
	{
		return 999.0f; // No starvation risk above 50%
	}

	if (FuelPercent >= 0.25f)
	{
		// Linear interpolation from no risk at 50% to base threshold at 25%
		const float T = (0.5f - FuelPercent) / 0.25f;
		return FMath::Lerp(999.0f, StarvationBaseGThreshold, T);
	}

	if (FuelPercent >= 0.10f)
	{
		// Linear interpolation from base threshold at 25% to 0.8G at 10%
		const float T = (0.25f - FuelPercent) / 0.15f;
		return FMath::Lerp(StarvationBaseGThreshold, StarvationBaseGThreshold * 0.67f, T);
	}

	// Below 10%: interpolate to very low threshold
	const float T = FMath::Clamp((0.10f - FuelPercent) / 0.10f, 0.0f, 1.0f);
	return FMath::Lerp(StarvationBaseGThreshold * 0.67f, StarvationBaseGThreshold * 0.4f, T);
}

float UMGFuelConsumptionComponent::GetLateralGForce() const
{
	if (!MovementComponent || !MovementComponent->GetOwner())
	{
		return 0.0f;
	}

	AActor* Owner = MovementComponent->GetOwner();
	const FVector Velocity = Owner->GetVelocity();
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Right = Owner->GetActorRightVector();

	// Calculate velocity change for acceleration
	float DeltaSeconds = 0.001f;
	if (UWorld* World = GetWorld())
	{
		DeltaSeconds = FMath::Max(0.001f, World->GetDeltaSeconds());
	}
	const FVector Acceleration = (Velocity - PreviousVelocity) / DeltaSeconds;
	const_cast<UMGFuelConsumptionComponent*>(this)->PreviousVelocity = Velocity;

	// Lateral acceleration is the component along the right vector
	const float LateralAccelCmPerSec2 = FMath::Abs(FVector::DotProduct(Acceleration, Right));

	// Convert to G (1G = 980.665 cm/s^2)
	const float LateralG = LateralAccelCmPerSec2 / 980.665f;

	return LateralG;
}
