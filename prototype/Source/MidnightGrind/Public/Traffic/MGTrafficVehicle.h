// Copyright Midnight Grind. All Rights Reserved.

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
