// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSpikeStrip.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleHitSpikes, AActor*, Vehicle);

/**
 * Police spike strip trap
 * Damages tires when vehicles drive over it
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API AMGSpikeStrip : public AActor
{
	GENERATED_BODY()

public:
	AMGSpikeStrip();

	virtual void BeginPlay() override;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Initialize(int32 InStripID, FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void SetLength(float Length);

	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Deploy();

	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Retract();

	// State
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	int32 GetStripID() const { return StripID; }

	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	bool IsDeployed() const { return bIsDeployed; }

	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	bool IsTriggered() const { return bHasBeenTriggered; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVehicleHitSpikes OnVehicleHitSpikes;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SpikeMesh;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float DefaultLength = 600.0f; // Length in cm

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float TireDamageAmount = 50.0f; // Damage per tire

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SpeedReductionPercent = 30.0f; // Speed reduction when hit

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	bool bSingleUse = true; // Destroyed after first hit

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 StripID = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDeployed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bHasBeenTriggered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentLength = 600.0f;

	// Internal
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyTireDamage(AActor* Vehicle);
};
