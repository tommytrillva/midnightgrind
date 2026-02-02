// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGTrackBoundaryActor.h
 * @brief Track boundary definition and off-track penalty system.
 *
 * This file defines the AMGTrackBoundaryActor class, which creates invisible
 * (or visible) boundaries along track edges. Boundaries detect when vehicles
 * leave the valid racing surface and apply appropriate responses like speed
 * penalties, collision forces, or vehicle resets.
 *
 * @section boundary_concepts Key Concepts
 *
 * TRACK BOUNDARY: The edge of the valid racing surface. In real racing, leaving
 * the track (exceeding track limits) can result in penalties. Boundaries define
 * where the track ends and the off-track area begins.
 *
 * SPLINE-BASED BOUNDARY: Rather than using many individual collision volumes,
 * boundaries are defined by a USplineComponent that follows the track edge.
 * This allows smooth, continuous boundary detection along complex track shapes.
 *
 * BOUNDARY TYPES: Different boundary types have different effects:
 * - Soft: Slows the vehicle (like gravel traps in real racing)
 * - Hard: Bounces the vehicle back (like barriers/walls)
 * - Invisible: Teleports vehicle back to track (arcade-style)
 * - KillZone: Resets vehicle after a delay (cliffs, water hazards)
 *
 * RECOVERY POSITION: When a vehicle goes off-track, the boundary system
 * calculates a safe position to return the vehicle to the racing surface.
 *
 * @section boundary_physics Physics Interactions
 *
 * Boundaries affect vehicles in several ways:
 * - SPEED PENALTY: Soft boundaries reduce vehicle speed while in contact
 * - BOUNCE FORCE: Hard boundaries apply impulse forces to push vehicles back
 * - GRIP REDUCTION: Off-track areas typically have reduced tire grip
 * - TIME PENALTY: Extended off-track time may incur race time penalties
 *
 * @section boundary_architecture Architecture
 *
 * The boundary system operates as follows:
 * 1. Designer places AMGTrackBoundaryActor along track edges
 * 2. Spline points define the boundary path
 * 3. Collision volumes are generated along the spline
 * 4. On overlap, the system identifies the vehicle and boundary type
 * 5. Appropriate effects are applied (speed penalty, bounce, reset)
 * 6. Events broadcast to other systems (HUD, race management)
 *
 * @section boundary_usage Usage Examples
 *
 * @code
 * // Setting up a soft boundary (gravel trap style)
 * AMGTrackBoundaryActor* GravelTrap = SpawnActor<AMGTrackBoundaryActor>();
 * GravelTrap->BoundaryType = EMGBoundaryType::Soft;
 * GravelTrap->SpeedPenaltyMultiplier = 0.6f; // Reduce speed to 60%
 * GravelTrap->bIsLeftBoundary = true; // Normal points into track
 *
 * // Setting up a hard boundary (wall)
 * AMGTrackBoundaryActor* Wall = SpawnActor<AMGTrackBoundaryActor>();
 * Wall->BoundaryType = EMGBoundaryType::Hard;
 * Wall->BounceForce = 800.0f;
 * Wall->bShowInGame = true; // Make wall visible
 *
 * // Querying boundary from vehicle code
 * AMGTrackBoundaryActor* NearestBoundary = FindNearestBoundary(VehicleLocation);
 * float DistanceToBoundary = NearestBoundary->GetDistanceToBoundary(VehicleLocation);
 *
 * // Responding to boundary events
 * Boundary->OnBoundaryHit.AddDynamic(this, &AMyClass::HandleBoundaryHit);
 *
 * void AMyClass::HandleBoundaryHit(AActor* Vehicle, const FMGBoundaryHitResult& Hit, float Force)
 * {
 *     // Play crash sound, show damage, apply penalty
 *     if (Force > CrashThreshold)
 *     {
 *         PlayCrashEffects(Hit.HitLocation, Force);
 *     }
 * }
 *
 * // Getting recovery position after going off-track
 * FMGBoundaryHitResult RecoveryInfo = Boundary->GetRecoveryInfo(VehicleLocation, VehicleVelocity);
 * Vehicle->SetActorTransform(FTransform(RecoveryInfo.RecoveryRotation, RecoveryInfo.RecoveryPosition));
 * @endcode
 *
 * @section boundary_visuals Visual Representation
 *
 * Boundaries can optionally be made visible using spline meshes:
 * - Set bShowInGame = true to render the boundary
 * - Configure BoundaryMesh and BoundaryMaterial for appearance
 * - BoundaryColor controls the tint (red for dangerous, yellow for caution)
 *
 * @section boundary_related Related Systems
 * - UMGTrackSubsystem: May track off-track time for penalties
 * - Vehicle Physics: Receives speed multipliers from soft boundaries
 * - Respawn System: Uses recovery positions from boundary hits
 * - Race HUD: Displays "off track" warnings
 *
 * @see EMGBoundaryType
 * @see FMGBoundaryHitResult
 * @see FOnBoundaryHit
 * @see FOnBoundaryEnter
 * @see FOnBoundaryExit
 */

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
	virtual void Tick(float MGDeltaTime) override;

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
	void UpdateBoundaryActors(float MGDeltaTime);

	/** Apply boundary effect to actor */
	void ApplyBoundaryEffect(AActor* Actor, float MGDeltaTime);

	/** Calculate impact force from velocity */
	float CalculateImpactForce(FVector Velocity, FVector Normal) const;
};
