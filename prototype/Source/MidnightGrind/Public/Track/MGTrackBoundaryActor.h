// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGTrackBoundaryActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

/**
 * Boundary type
 */
UENUM(BlueprintType)
enum class EMGBoundaryType : uint8
{
	/** Soft boundary - slowdown penalty */
	Soft,
	/** Hard boundary - collision/bounce */
	Hard,
	/** Invisible - teleport back to track */
	Invisible,
	/** Kill zone - reset vehicle */
	KillZone
};

/**
 * Boundary hit result
 */
USTRUCT(BlueprintType)
struct FMGBoundaryHitResult
{
	GENERATED_BODY()

	/** Location of the hit */
	UPROPERTY(BlueprintReadOnly)
	FVector HitLocation = FVector::ZeroVector;

	/** Normal of the boundary at hit point */
	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal = FVector::UpVector;

	/** Type of boundary hit */
	UPROPERTY(BlueprintReadOnly)
	EMGBoundaryType BoundaryType = EMGBoundaryType::Soft;

	/** Speed penalty multiplier (1.0 = no penalty) */
	UPROPERTY(BlueprintReadOnly)
	float SpeedPenaltyMultiplier = 1.0f;

	/** Distance along the boundary spline */
	UPROPERTY(BlueprintReadOnly)
	float DistanceAlongBoundary = 0.0f;

	/** Closest point on track to return to */
	UPROPERTY(BlueprintReadOnly)
	FVector RecoveryPosition = FVector::ZeroVector;

	/** Recovery rotation */
	UPROPERTY(BlueprintReadOnly)
	FRotator RecoveryRotation = FRotator::ZeroRotator;
};

/**
 * Delegate for boundary events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBoundaryHit, AActor*, Vehicle, const FMGBoundaryHitResult&, HitResult, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoundaryEnter, AActor*, Vehicle, EMGBoundaryType, BoundaryType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoundaryExit, AActor*, Vehicle, float, TimeInBoundary);

/**
 * Track Boundary Actor
 * Defines track boundaries using splines
 *
 * Features:
 * - Spline-based boundary definition
 * - Multiple boundary types (soft, hard, invisible, kill)
 * - Collision response handling
 * - Recovery position calculation
 * - Off-track penalty system
 */
UCLASS()
class MIDNIGHTGRIND_API AMGTrackBoundaryActor : public AActor
{
	GENERATED_BODY()

public:
	AMGTrackBoundaryActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a vehicle hits the boundary */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoundaryHit OnBoundaryHit;

	/** Called when a vehicle enters boundary zone */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoundaryEnter OnBoundaryEnter;

	/** Called when a vehicle exits boundary zone */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoundaryExit OnBoundaryExit;

	// ==========================================
	// QUERY
	// ==========================================

	/** Get closest point on boundary to world position */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	FVector GetClosestPointOnBoundary(FVector WorldPosition) const;

	/** Get distance from world position to boundary */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	float GetDistanceToBoundary(FVector WorldPosition) const;

	/** Get normal at closest point */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	FVector GetBoundaryNormalAtPoint(FVector WorldPosition) const;

	/** Check if position is inside boundary (off-track) */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	bool IsPositionOffTrack(FVector WorldPosition) const;

	/** Get recovery position for off-track vehicle */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	FMGBoundaryHitResult GetRecoveryInfo(FVector WorldPosition, FVector Velocity) const;

	/** Get boundary type */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	EMGBoundaryType GetBoundaryType() const { return BoundaryType; }

	/** Get total boundary length */
	UFUNCTION(BlueprintPure, Category = "Boundary")
	float GetBoundaryLength() const;

	// ==========================================
	// MODIFICATION
	// ==========================================

	/** Set boundary type */
	UFUNCTION(BlueprintCallable, Category = "Boundary")
	void SetBoundaryType(EMGBoundaryType NewType);

	/** Set speed penalty */
	UFUNCTION(BlueprintCallable, Category = "Boundary")
	void SetSpeedPenalty(float Multiplier);

	/** Enable/disable boundary */
	UFUNCTION(BlueprintCallable, Category = "Boundary")
	void SetBoundaryEnabled(bool bEnabled);

	/** Set visual appearance */
	UFUNCTION(BlueprintCallable, Category = "Boundary")
	void SetBoundaryVisible(bool bVisible);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Spline defining the boundary */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* BoundarySpline;

	/** Type of boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	EMGBoundaryType BoundaryType = EMGBoundaryType::Soft;

	/** Is this the left or right boundary (for normal direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	bool bIsLeftBoundary = true;

	/** Width of the boundary collision zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	float BoundaryWidth = 50.0f;

	/** Height of the boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	float BoundaryHeight = 200.0f;

	/** Speed penalty multiplier when in boundary (soft only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpeedPenaltyMultiplier = 0.7f;

	/** Bounce force for hard boundaries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	float BounceForce = 500.0f;

	/** Time before kill zone triggers reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	float KillZoneDelay = 2.0f;

	/** Is boundary closed loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
	bool bIsClosedLoop = false;

	/** Show boundary in game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bShowInGame = false;

	/** Boundary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor BoundaryColor = FLinearColor::Red;

	/** Mesh to use for visual representation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UStaticMesh* BoundaryMesh;

	/** Material for boundary mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UMaterialInterface* BoundaryMaterial;

	// ==========================================
	// STATE
	// ==========================================

	/** Whether boundary is active */
	bool bBoundaryEnabled = true;

	/** Actors currently in boundary zone */
	UPROPERTY()
	TMap<AActor*, float> ActorsInBoundary; // Actor -> Time in boundary

	/** Spline mesh components for visualization */
	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshComponents;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Build visual representation */
	void BuildVisuals();

	/** Clear visual representation */
	void ClearVisuals();

	/** Handle overlap events */
	UFUNCTION()
	void OnBoundaryOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoundaryOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Update actors in boundary */
	void UpdateBoundaryActors(float DeltaTime);

	/** Apply boundary effect to actor */
	void ApplyBoundaryEffect(AActor* Actor, float DeltaTime);

	/** Calculate impact force from velocity */
	float CalculateImpactForce(FVector Velocity, FVector Normal) const;
};
