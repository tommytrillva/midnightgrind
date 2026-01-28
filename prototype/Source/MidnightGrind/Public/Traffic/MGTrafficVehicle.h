// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGTrafficVehicle.h
 * @brief Traffic Vehicle Pawn - AI-Controlled Civilian Vehicle Actor
 * =============================================================================
 *
 * @section Overview
 * This file defines the AMGTrafficVehicle pawn class, which represents a single
 * AI-controlled civilian vehicle in the game world. These vehicles are spawned
 * and managed by UMGTrafficSubsystem to populate the streets of Midnight City.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * 1. PAWN CLASS
 *    AMGTrafficVehicle inherits from APawn (not ACharacter):
 *    - Pawns are actors that can be "possessed" by controllers
 *    - Traffic vehicles use simple AI, not player control
 *    - Lighter weight than full vehicle physics simulation
 *
 * 2. TRAFFIC SUBSYSTEM RELATIONSHIP
 *    The subsystem is the "manager", this pawn is the "worker":
 *    - Subsystem decides WHAT the vehicle should do
 *    - Pawn handles HOW to do it (movement, visuals)
 *    - VehicleID links the pawn to subsystem's state data
 *
 * 3. MOVEMENT MODEL
 *    Traffic vehicles use simplified movement (not full physics):
 *    - CurrentSpeed: Actual speed in MPH
 *    - TargetSpeed: Speed the AI wants to reach
 *    - AccelerationRate/DecelerationRate: How fast speed changes
 *    - SteeringSpeed: How fast the vehicle can turn (degrees/sec)
 *
 * 4. COLLISION HANDLING
 *    When a traffic vehicle is hit:
 *    - bHasCollided flag is set to true
 *    - OnCollisionHit callback notifies the subsystem
 *    - Subsystem triggers panic behavior, scoring, etc.
 *
 * 5. VISUAL STATE
 *    Vehicles have visual indicators for realism:
 *    - Headlights: On/off based on time of day
 *    - Brake Lights: On when decelerating
 *    - Turn Signals: Left/right for lane changes
 *
 * 6. VEHICLE TYPES
 *    Different EMGTrafficVehicleType values use different meshes:
 *    - SetVehicleMesh() called by subsystem during initialization
 *    - GetVehicleDimensions() returns bounds for collision
 *
 * @section Usage Common Usage Patterns
 *
 * @code
 * // Traffic vehicles are typically spawned by the subsystem, not directly.
 * // However, you can interact with them:
 *
 * // Check if an actor is a traffic vehicle
 * AMGTrafficVehicle* TrafficVehicle = Cast<AMGTrafficVehicle>(HitActor);
 * if (TrafficVehicle)
 * {
 *     // Get the vehicle's ID to query the subsystem for more data
 *     int32 VehicleID = TrafficVehicle->GetVehicleID();
 *
 *     // Check if this is a sports car (might be faster)
 *     if (TrafficVehicle->GetVehicleType() == EMGTrafficVehicleType::SportsCar)
 *     {
 *         // Sports cars are faster
 *     }
 *
 *     // Get current speed for collision damage calculation
 *     float SpeedMPH = TrafficVehicle->GetCurrentSpeedMPH();
 *
 *     // Check if already in a collision (avoid double-counting)
 *     if (!TrafficVehicle->HasCollided())
 *     {
 *         // First collision with this vehicle
 *     }
 * }
 *
 * // The subsystem can control individual vehicles:
 * // (Usually done through the subsystem API, not directly)
 * TrafficVehicle->SetTargetSpeed(45.0f);  // Slow down
 * TrafficVehicle->SetBehavior(EMGTrafficBehavior::Panicked);
 * TrafficVehicle->SetTurnSignal(true, false);  // Left turn signal
 * @endcode
 *
 * @section Architecture Architecture Notes
 *
 * COMPONENT HIERARCHY:
 * - CollisionBox: Root component, handles physics collision
 * - VehicleMesh: Static mesh for visual representation
 *
 * TICK BEHAVIOR:
 * - Tick() updates movement and behavior each frame
 * - Movement is kinematic (not physics-driven)
 * - Subsystem provides high-level commands
 *
 * POOLING:
 * - Vehicles may be pooled for performance
 * - InitializeVehicle() resets state for reuse
 * - EndPlay() cleans up for pooling
 *
 * @see UMGTrafficSubsystem - The manager that controls these vehicles
 * @see FMGTrafficVehicleState - State data stored in the subsystem
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MGTrafficSubsystem.h"
#include "MGTrafficVehicle.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

/**
 * Traffic vehicle pawn
 *
 * Simple AI-controlled vehicle that populates the streets.
 * Managed by MGTrafficSubsystem for spawning, despawning, and behavior.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API AMGTrafficVehicle : public APawn
{
	GENERATED_BODY()

public:
	AMGTrafficVehicle();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void InitializeVehicle(int32 InVehicleID, EMGTrafficVehicleType InType, EMGTrafficBehavior InBehavior);

	// Movement
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetTargetSpeed(float SpeedMPH);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetTargetLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetBehavior(EMGTrafficBehavior NewBehavior);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void StopImmediately();

	// Queries
	UFUNCTION(BlueprintPure, Category = "Traffic")
	int32 GetVehicleID() const { return VehicleID; }

	UFUNCTION(BlueprintPure, Category = "Traffic")
	EMGTrafficVehicleType GetVehicleType() const { return VehicleType; }

	UFUNCTION(BlueprintPure, Category = "Traffic")
	EMGTrafficBehavior GetBehavior() const { return CurrentBehavior; }

	UFUNCTION(BlueprintPure, Category = "Traffic")
	float GetCurrentSpeedMPH() const { return CurrentSpeed; }

	UFUNCTION(BlueprintPure, Category = "Traffic")
	bool HasCollided() const { return bHasCollided; }

	// Visual
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetVehicleMesh(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetHeadlightsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetBrakeLightsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetTurnSignal(bool bLeft, bool bRight);

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VehicleMesh;

	// Vehicle state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 VehicleID = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMGTrafficVehicleType VehicleType = EMGTrafficVehicleType::Sedan;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMGTrafficBehavior CurrentBehavior = EMGTrafficBehavior::Normal;

	// Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CurrentSpeed = 0.0f; // MPH

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float TargetSpeed = 35.0f; // MPH

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bHasTargetLocation = false;

	// Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float AccelerationRate = 8.0f; // MPH per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float DecelerationRate = 15.0f; // MPH per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float BrakeRate = 30.0f; // MPH per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SteeringSpeed = 90.0f; // Degrees per second

	// Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	bool bHasCollided = false;

	// Lights
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
	bool bHeadlightsOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
	bool bBrakeLightsOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
	bool bLeftTurnSignal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
	bool bRightTurnSignal = false;

	// Internal
	void UpdateMovement(float DeltaTime);
	void UpdateBehavior(float DeltaTime);
	FVector GetVehicleDimensions() const;

	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
