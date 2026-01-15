// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "MGTrackSpline.generated.h"

class USplineMeshComponent;

/**
 * Track surface type
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt		UMETA(DisplayName = "Asphalt"),
	Concrete	UMETA(DisplayName = "Concrete"),
	Gravel		UMETA(DisplayName = "Gravel"),
	Dirt		UMETA(DisplayName = "Dirt"),
	Grass		UMETA(DisplayName = "Grass"),
	Sand		UMETA(DisplayName = "Sand"),
	Snow		UMETA(DisplayName = "Snow"),
	Ice			UMETA(DisplayName = "Ice"),
	Wet			UMETA(DisplayName = "Wet Asphalt")
};

/**
 * Per-segment track data
 */
USTRUCT(BlueprintType)
struct FMGTrackSegment
{
	GENERATED_BODY()

	/** Spline distance where this segment starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float StartDistance = 0.0f;

	/** Surface type for this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	EMGTrackSurface Surface = EMGTrackSurface::Asphalt;

	/** Track width at this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float TrackWidth = 1000.0f;

	/** Is this segment a hazard zone? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	bool bHazardZone = false;

	/** Speed limit for this segment (for AI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float SuggestedSpeed = 0.0f; // 0 = no limit
};

/**
 * Track spline actor - defines the racing line and track bounds
 * Used for AI pathfinding, position calculation, and respawn points
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGTrackSpline : public AActor
{
	GENERATED_BODY()

public:
	AMGTrackSpline();

	//~ Begin AActor Interface
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	//~ End AActor Interface

	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Main racing line spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> RacingLineSpline;

	/** Inner track boundary spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> InnerBoundarySpline;

	/** Outer track boundary spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> OuterBoundarySpline;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Track name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FText TrackName;

	/** Track length (calculated from spline) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track")
	float TrackLength = 0.0f;

	/** Is this a closed loop track? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bClosedLoop = true;

	/** Default track width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float DefaultTrackWidth = 1000.0f;

	/** Track segments with variable properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TArray<FMGTrackSegment> TrackSegments;

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get total track length */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetTrackLength() const { return TrackLength; }

	/** Get position on racing line at distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	FVector GetPositionAtDistance(float Distance) const;

	/** Get rotation at distance (facing direction) */
	UFUNCTION(BlueprintPure, Category = "Track")
	FRotator GetRotationAtDistance(float Distance) const;

	/** Get transform at distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	FTransform GetTransformAtDistance(float Distance) const;

	/** Get closest point on track to a world position */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetClosestDistanceOnTrack(const FVector& WorldPosition) const;

	/** Get track width at a distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetTrackWidthAtDistance(float Distance) const;

	/** Get surface type at a distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	EMGTrackSurface GetSurfaceAtDistance(float Distance) const;

	/** Check if a position is on track */
	UFUNCTION(BlueprintPure, Category = "Track")
	bool IsPositionOnTrack(const FVector& WorldPosition) const;

	/** Get signed distance from racing line (positive = right, negative = left) */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetLateralDistanceFromRacingLine(const FVector& WorldPosition) const;

	// ==========================================
	// AI HELPERS
	// ==========================================

	/** Get suggested speed at distance */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	float GetSuggestedSpeedAtDistance(float Distance) const;

	/** Get racing line point ahead of current position */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	FVector GetLookAheadPoint(float CurrentDistance, float LookAheadDistance) const;

	/** Get curvature at distance (for speed calculation) */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	float GetCurvatureAtDistance(float Distance) const;

	// ==========================================
	// RESPAWN
	// ==========================================

	/** Get respawn transform at a distance */
	UFUNCTION(BlueprintPure, Category = "Track|Respawn")
	FTransform GetRespawnTransformAtDistance(float Distance) const;

	/** Get nearest respawn point to a position */
	UFUNCTION(BlueprintPure, Category = "Track|Respawn")
	FTransform GetNearestRespawnTransform(const FVector& WorldPosition) const;

protected:
	/** Calculate track length from spline */
	void CalculateTrackLength();

	/** Generate boundary splines from racing line */
	void GenerateBoundarySplines();

	/** Get segment data at distance */
	const FMGTrackSegment* GetSegmentAtDistance(float Distance) const;
};

/**
 * Track data asset - contains track configuration for loading
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Track display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	FText TrackName;

	/** Track description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	FText Description;

	/** Track thumbnail */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	/** Level to load */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	TSoftObjectPtr<UWorld> TrackLevel;

	/** Track length in meters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	float TrackLengthMeters = 0.0f;

	/** Number of turns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	int32 TurnCount = 0;

	/** Track difficulty (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Difficulty = 3;

	/** Location/region */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	FText Location;

	/** Time of day options available */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	bool bSupportsNight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	bool bSupportsDay = true;

	/** Weather options available */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	bool bSupportsDry = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	bool bSupportsWet = true;

	/** Lap record */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	float LapRecord = 0.0f;

	/** Record holder name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	FText RecordHolder;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	int32 RequiredReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	bool bRequiresPurchase = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	int64 PurchasePrice = 0;
};
