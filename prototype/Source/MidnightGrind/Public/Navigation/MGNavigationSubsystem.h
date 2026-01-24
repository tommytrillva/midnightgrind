// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGNavigationSubsystem.generated.h"

/**
 * Point of interest type
 */
UENUM(BlueprintType)
enum class EMGPOIType : uint8
{
	Race UMETA(DisplayName = "Race Event"),
	Garage UMETA(DisplayName = "Garage"),
	CarDealer UMETA(DisplayName = "Car Dealer"),
	PartsShop UMETA(DisplayName = "Parts Shop"),
	GasStation UMETA(DisplayName = "Gas Station"),
	SafeHouse UMETA(DisplayName = "Safe House"),
	Meetup UMETA(DisplayName = "Street Meetup"),
	RivalEncounter UMETA(DisplayName = "Rival"),
	SecretArea UMETA(DisplayName = "Secret"),
	Collectible UMETA(DisplayName = "Collectible"),
	PhotoSpot UMETA(DisplayName = "Photo Spot"),
	SpeedTrap UMETA(DisplayName = "Speed Trap"),
	PoliceStation UMETA(DisplayName = "Police Station"),
	Shortcut UMETA(DisplayName = "Shortcut"),
	JumpRamp UMETA(DisplayName = "Jump Ramp"),
	DriftZone UMETA(DisplayName = "Drift Zone"),
	SpeedZone UMETA(DisplayName = "Speed Zone"),
	Custom UMETA(DisplayName = "Custom Waypoint")
};

/**
 * Navigation instruction
 */
UENUM(BlueprintType)
enum class EMGNavInstruction : uint8
{
	Straight UMETA(DisplayName = "Continue Straight"),
	TurnLeft UMETA(DisplayName = "Turn Left"),
	TurnRight UMETA(DisplayName = "Turn Right"),
	SharpLeft UMETA(DisplayName = "Sharp Left"),
	SharpRight UMETA(DisplayName = "Sharp Right"),
	SlightLeft UMETA(DisplayName = "Slight Left"),
	SlightRight UMETA(DisplayName = "Slight Right"),
	UTurn UMETA(DisplayName = "U-Turn"),
	Merge UMETA(DisplayName = "Merge"),
	Exit UMETA(DisplayName = "Take Exit"),
	Roundabout UMETA(DisplayName = "Roundabout"),
	Arrived UMETA(DisplayName = "Destination")
};

/**
 * Road type
 */
UENUM(BlueprintType)
enum class EMGRoadType : uint8
{
	Highway UMETA(DisplayName = "Highway"),
	MainRoad UMETA(DisplayName = "Main Road"),
	SideStreet UMETA(DisplayName = "Side Street"),
	Alley UMETA(DisplayName = "Alley"),
	Parking UMETA(DisplayName = "Parking Lot"),
	OffRoad UMETA(DisplayName = "Off-Road"),
	Bridge UMETA(DisplayName = "Bridge"),
	Tunnel UMETA(DisplayName = "Tunnel"),
	Ramp UMETA(DisplayName = "Ramp")
};

/**
 * Point of interest data
 */
USTRUCT(BlueprintType)
struct FMGPointOfInterest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FName POIID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	EMGPOIType Type = EMGPOIType::Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	float InteractionRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	TSoftObjectPtr<UTexture2D> MapIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bShowOnMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bShowWhenClose = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	float VisibilityDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FName LinkedEventID;
};

/**
 * Navigation waypoint in a route
 */
USTRUCT(BlueprintType)
struct FMGNavWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	EMGNavInstruction Instruction = EMGNavInstruction::Straight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float DistanceFromStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float DistanceToNext = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FString StreetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	EMGRoadType RoadType = EMGRoadType::MainRoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float SpeedLimit = 35.0f; // mph

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bIsCheckpoint = false;
};

/**
 * Navigation route
 */
USTRUCT(BlueprintType)
struct FMGNavRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FGuid RouteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FVector Destination = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FName DestinationPOI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<FMGNavWaypoint> Waypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedTime = 0.0f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bAvoidHighways = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bAvoidPolice = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bShortestRoute = true;
};

/**
 * Map region/district
 */
USTRUCT(BlueprintType)
struct FMGMapRegion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FName RegionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FVector2D MapBoundsMin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FVector2D MapBoundsMax = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FLinearColor RegionColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 RequiredREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	float PoliceActivity = 0.5f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	float RacingActivity = 0.5f;
};

/**
 * Minimap marker
 */
USTRUCT(BlueprintType)
struct FMGMapMarker
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FGuid MarkerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	EMGPOIType MarkerType = EMGPOIType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FLinearColor Color = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bPulse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bShowDistance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bShowDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FText Label;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationSet, const FMGPointOfInterest&, Destination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestinationCleared);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationReached, const FMGPointOfInterest&, Destination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNavInstructionChanged, EMGNavInstruction, Instruction, float, Distance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRouteRecalculated, const FMGNavRoute&, NewRoute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPOIDiscovered, const FMGPointOfInterest&, POI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegionEntered, const FMGMapRegion&, Region);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointAdded, const FMGMapMarker&, Waypoint);

/**
 * Navigation and Map Subsystem
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGNavigationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDestinationSet OnDestinationSet;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDestinationCleared OnDestinationCleared;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDestinationReached OnDestinationReached;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNavInstructionChanged OnNavInstructionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRouteRecalculated OnRouteRecalculated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPOIDiscovered OnPOIDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRegionEntered OnRegionEntered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaypointAdded OnWaypointAdded;

	// Destination
	UFUNCTION(BlueprintCallable, Category = "Destination")
	bool SetDestination(FName POIID);

	UFUNCTION(BlueprintCallable, Category = "Destination")
	bool SetDestinationLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Destination")
	void ClearDestination();

	UFUNCTION(BlueprintPure, Category = "Destination")
	bool HasDestination() const { return bHasDestination; }

	UFUNCTION(BlueprintPure, Category = "Destination")
	FMGPointOfInterest GetCurrentDestination() const;

	UFUNCTION(BlueprintPure, Category = "Destination")
	float GetDistanceToDestination() const;

	UFUNCTION(BlueprintPure, Category = "Destination")
	float GetETAToDestination() const;

	// Route
	UFUNCTION(BlueprintPure, Category = "Route")
	const FMGNavRoute& GetCurrentRoute() const { return CurrentRoute; }

	UFUNCTION(BlueprintCallable, Category = "Route")
	void RecalculateRoute();

	UFUNCTION(BlueprintPure, Category = "Route")
	FMGNavWaypoint GetNextWaypoint() const;

	UFUNCTION(BlueprintPure, Category = "Route")
	EMGNavInstruction GetCurrentInstruction() const;

	UFUNCTION(BlueprintPure, Category = "Route")
	float GetDistanceToNextTurn() const;

	UFUNCTION(BlueprintCallable, Category = "Route")
	void SetRoutePreferences(bool bAvoidHighways, bool bAvoidPolice, bool bShortestRoute);

	// POI Management
	UFUNCTION(BlueprintCallable, Category = "POI")
	void RegisterPOI(const FMGPointOfInterest& POI);

	UFUNCTION(BlueprintCallable, Category = "POI")
	void UnregisterPOI(FName POIID);

	UFUNCTION(BlueprintPure, Category = "POI")
	FMGPointOfInterest GetPOI(FName POIID) const;

	UFUNCTION(BlueprintPure, Category = "POI")
	TArray<FMGPointOfInterest> GetAllPOIs() const;

	UFUNCTION(BlueprintPure, Category = "POI")
	TArray<FMGPointOfInterest> GetPOIsByType(EMGPOIType Type) const;

	UFUNCTION(BlueprintPure, Category = "POI")
	TArray<FMGPointOfInterest> GetNearbyPOIs(FVector Location, float Radius) const;

	UFUNCTION(BlueprintCallable, Category = "POI")
	void DiscoverPOI(FName POIID);

	UFUNCTION(BlueprintCallable, Category = "POI")
	void CompletePOI(FName POIID);

	// Regions
	UFUNCTION(BlueprintCallable, Category = "Regions")
	void RegisterRegion(const FMGMapRegion& Region);

	UFUNCTION(BlueprintPure, Category = "Regions")
	FMGMapRegion GetCurrentRegion() const;

	UFUNCTION(BlueprintPure, Category = "Regions")
	TArray<FMGMapRegion> GetAllRegions() const;

	UFUNCTION(BlueprintCallable, Category = "Regions")
	void UnlockRegion(FName RegionID);

	UFUNCTION(BlueprintPure, Category = "Regions")
	bool IsRegionUnlocked(FName RegionID) const;

	// Custom Markers
	UFUNCTION(BlueprintCallable, Category = "Markers")
	FGuid AddCustomMarker(FVector Location, FLinearColor Color, const FText& Label);

	UFUNCTION(BlueprintCallable, Category = "Markers")
	void RemoveCustomMarker(const FGuid& MarkerID);

	UFUNCTION(BlueprintCallable, Category = "Markers")
	void ClearAllCustomMarkers();

	UFUNCTION(BlueprintPure, Category = "Markers")
	TArray<FMGMapMarker> GetAllMarkers() const;

	// Player Position
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UpdatePlayerPosition(FVector Position, FRotator Rotation);

	UFUNCTION(BlueprintPure, Category = "Player")
	FVector GetPlayerPosition() const { return PlayerPosition; }

	UFUNCTION(BlueprintPure, Category = "Player")
	FRotator GetPlayerRotation() const { return PlayerRotation; }

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetPlayerSpeed() const { return PlayerSpeed; }

	// Minimap
	UFUNCTION(BlueprintPure, Category = "Minimap")
	FVector2D WorldToMapCoordinates(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	FVector MapToWorldCoordinates(FVector2D MapLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMinimapZoom(float ZoomLevel);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetMinimapZoom() const { return MinimapZoom; }

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMinimapRotation(bool bRotateWithPlayer);

	// GPS Voice
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetGPSVoiceEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsGPSVoiceEnabled() const { return bGPSVoiceEnabled; }

protected:
	void UpdateNavigation();
	void CheckPOIProximity();
	void CheckRegionChange();
	FMGNavRoute CalculateRoute(FVector Start, FVector End);

private:
	FTimerHandle UpdateTimerHandle;

	// Current state
	UPROPERTY()
	bool bHasDestination = false;

	UPROPERTY()
	FMGPointOfInterest CurrentDestinationPOI;

	UPROPERTY()
	FMGNavRoute CurrentRoute;

	UPROPERTY()
	int32 CurrentWaypointIndex = 0;

	// POIs and Regions
	UPROPERTY()
	TMap<FName, FMGPointOfInterest> RegisteredPOIs;

	UPROPERTY()
	TMap<FName, FMGMapRegion> RegisteredRegions;

	UPROPERTY()
	FName CurrentRegionID;

	// Custom markers
	UPROPERTY()
	TArray<FMGMapMarker> CustomMarkers;

	// Player tracking
	UPROPERTY()
	FVector PlayerPosition = FVector::ZeroVector;

	UPROPERTY()
	FRotator PlayerRotation = FRotator::ZeroRotator;

	UPROPERTY()
	float PlayerSpeed = 0.0f;

	// Settings
	UPROPERTY()
	float MinimapZoom = 1.0f;

	UPROPERTY()
	bool bMinimapRotatesWithPlayer = true;

	UPROPERTY()
	bool bGPSVoiceEnabled = true;

	UPROPERTY()
	bool bAvoidHighways = false;

	UPROPERTY()
	bool bAvoidPolice = false;

	UPROPERTY()
	bool bShortestRoute = true;

	// Map bounds
	UPROPERTY()
	FVector2D MapWorldMin = FVector2D(-100000, -100000);

	UPROPERTY()
	FVector2D MapWorldMax = FVector2D(100000, 100000);
};
