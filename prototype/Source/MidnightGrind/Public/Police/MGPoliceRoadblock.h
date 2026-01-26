// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGPoliceRoadblock.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockHit, AActor*, HitActor, float, ImpactSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoadblockDestroyed);

/**
 * Police roadblock obstacle
 * Spawned during high heat levels to block the player's path
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API AMGPoliceRoadblock : public AActor
{
	GENERATED_BODY()

public:
	AMGPoliceRoadblock();

	virtual void BeginPlay() override;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void Initialize(int32 InBlockadeID, FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void SetNumVehicles(int32 Count);

	// State
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	int32 GetBlockadeID() const { return BlockadeID; }

	UFUNCTION(BlueprintPure, Category = "Roadblock")
	bool IsBreached() const { return bIsBreached; }

	UFUNCTION(BlueprintPure, Category = "Roadblock")
	float GetHealth() const { return CurrentHealth; }

	// Damage
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoadblockHit OnRoadblockHit;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoadblockDestroyed OnRoadblockDestroyed;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BarricadeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<UStaticMeshComponent>> PoliceVehicleMeshes;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxHealth = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Width = 1200.0f; // Width of roadblock in cm

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	int32 MaxVehicles = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float BreachSpeedThreshold = 50.0f; // MPH required to break through

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 BlockadeID = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentHealth = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsBreached = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 NumVehicles = 2;

	// Internal
	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void BreachRoadblock();
};
