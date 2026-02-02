// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGNavigationSubsystem.h
 * @brief GPS Navigation, Minimap, and Point of Interest (POI) System
 * @author Midnight Grind Team
 * @version 1.0
 *
 * @section overview Overview
 * ============================================================================
 * MGNavigationSubsystem.h
 * Midnight Grind - GPS Navigation, Minimap, and Point of Interest System
 * ============================================================================
 *
 * This subsystem handles everything related to in-game navigation and the world map.
 * It's like the GPS system in your car combined with the game's minimap UI system.
 *
 * @section features What This Subsystem Does
 *
 * @subsection gps 1. GPS Navigation
 * - Set destinations and get turn-by-turn directions
 * - Calculate routes between any two points
 * - Provide distance and ETA estimates
 * - Voice guidance (optional "GPS Voice" feature)
 *
 * @subsection poi 2. Points of Interest (POIs)
 * - Race events, garages, car dealers, parts shops
 * - Safe houses, meetup spots, rival encounters
 * - Collectibles, photo spots, speed traps
 * - Drift zones, speed zones, jump ramps
 *
 * @subsection minimap 3. Minimap/Map System
 * - Track player position and rotation
 * - Custom waypoint markers
 * - POI icons with distance indicators
 * - Zoom control and rotation modes
 *
 * @subsection regions 4. Map Regions/Districts
 * - City divided into distinct regions
 * - Regions can be locked/unlocked by reputation
 * - Each region has unique police/racing activity levels
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection poiconcept 1. Point of Interest (FMGPointOfInterest)
 * A location in the world that the player can interact with or navigate to.
 * Has properties like:
 * - Type (race, garage, collectible, etc.)
 * - Position and interaction radius
 * - Discovery state (undiscovered POIs appear as "?" on map)
 * - Completion state (for collectibles and one-time events)
 *
 * @subsection routeconcept 2. Navigation Route (FMGNavRoute)
 * A calculated path from point A to point B containing:
 * - Array of waypoints with turn instructions
 * - Total distance and estimated time
 * - Route preferences (avoid highways, avoid police, shortest path)
 *
 * @subsection instructionconcept 3. Navigation Instruction (EMGNavInstruction)
 * Turn-by-turn directions like "Turn Left", "Sharp Right", "U-Turn"
 * Used for the GPS guidance overlay and voice prompts.
 *
 * @subsection regionconcept 4. Map Region (FMGMapRegion)
 * A district of the city with bounds, unlock requirements, and activity levels.
 * Example: "Downtown" might have high police activity but many race events.
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem (this is a WorldSubsystem, exists per-level)
 * UMGNavigationSubsystem* Nav = GetWorld()->GetSubsystem<UMGNavigationSubsystem>();
 *
 * // === DESTINATION SETTING ===
 * // Set a destination to a POI by ID
 * Nav->SetDestination(FName("GarageMain"));
 *
 * // Or set a destination to any world location
 * Nav->SetDestinationLocation(FVector(1000, 2000, 0));
 *
 * // Clear navigation when arrived or cancelled
 * Nav->ClearDestination();
 *
 * // === NAVIGATION INFO ===
 * // Get distance to destination (in Unreal units, cm)
 * float Distance = Nav->GetDistanceToDestination();
 *
 * // Get estimated time of arrival (seconds)
 * float ETA = Nav->GetETAToDestination();
 *
 * // Get the next turn instruction for GPS UI
 * EMGNavInstruction NextTurn = Nav->GetCurrentInstruction();
 * float DistanceToTurn = Nav->GetDistanceToNextTurn();
 *
 * // === POI MANAGEMENT ===
 * // Find nearby POIs (within 5000 units)
 * TArray<FMGPointOfInterest> NearbyPOIs = Nav->GetNearbyPOIs(PlayerLocation, 5000.0f);
 *
 * // Get all race event POIs
 * TArray<FMGPointOfInterest> RaceEvents = Nav->GetPOIsByType(EMGPOIType::Race);
 *
 * // Mark a POI as discovered when player finds it
 * Nav->DiscoverPOI(FName("SecretGarage_01"));
 *
 * // Mark a POI as complete (e.g., collected the collectible)
 * Nav->CompletePOI(FName("Collectible_Billboard_05"));
 *
 * // === CUSTOM MARKERS ===
 * // Add a custom waypoint marker
 * FGuid MarkerId = Nav->AddCustomMarker(TargetLocation, FLinearColor::Yellow, FText::FromString("Meet here"));
 *
 * // Remove a marker
 * Nav->RemoveCustomMarker(MarkerId);
 *
 * // Clear all custom markers
 * Nav->ClearAllCustomMarkers();
 *
 * // === PLAYER TRACKING ===
 * // Update player position (typically called every frame by the player vehicle)
 * Nav->UpdatePlayerPosition(VehicleLocation, VehicleRotation);
 *
 * // === MINIMAP ===
 * // Convert between world and map coordinates
 * FVector2D MapPos = Nav->WorldToMapCoordinates(WorldLocation);
 * FVector WorldPos = Nav->MapToWorldCoordinates(MapPos);
 *
 * // Control minimap zoom and rotation
 * Nav->SetMinimapZoom(1.5f);
 * Nav->SetMinimapRotation(true);  // Rotate with player
 *
 * // === REGIONS ===
 * // Get current region player is in
 * FMGMapRegion CurrentRegion = Nav->GetCurrentRegion();
 *
 * // Check if a region is unlocked
 * bool bUnlocked = Nav->IsRegionUnlocked(FName("Downtown"));
 *
 * // Unlock a region (e.g., when player earns enough REP)
 * Nav->UnlockRegion(FName("Industrial"));
 *
 * // === EVENT LISTENERS ===
 * Nav->OnDestinationReached.AddDynamic(this, &UMyClass::HandleArrival);
 * Nav->OnPOIDiscovered.AddDynamic(this, &UMyClass::HandleDiscovery);
 * Nav->OnRegionEntered.AddDynamic(this, &UMyClass::HandleRegionChange);
 * @endcode
 *
 * @section notes Important Notes
 * - This is a WORLD SUBSYSTEM (UWorldSubsystem), not a GameInstance subsystem.
 *   It gets created fresh for each level and is destroyed on level unload.
 *
 * - POI discovery and completion states should be persisted to save data.
 *   Use DiscoverPOI() when player finds a new location.
 *   Use CompletePOI() when player finishes an event at that location.
 *
 * - The minimap coordinate system converts world coordinates to 2D map space.
 *   Use WorldToMapCoordinates() and MapToWorldCoordinates() for conversion.
 *
 * @section delegates Available Delegates
 * - OnDestinationSet: Player selected a new destination
 * - OnDestinationCleared: Navigation cancelled
 * - OnDestinationReached: Player arrived at destination
 * - OnNavInstructionChanged: Turn instruction updated (for UI/voice)
 * - OnRouteRecalculated: Route was recalculated (player went off-route)
 * - OnPOIDiscovered: Player found a new point of interest
 * - OnRegionEntered: Player entered a new district
 * - OnWaypointAdded: Custom marker was added
 *
 * @see UMGRaceFlowSubsystem Interacts with POIs for race event starts
 * @see UMGMinimapWidget UI widget that visualizes this subsystem's data
 * ============================================================================
 */

/**
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This subsystem handles everything related to in-game navigation and the world map.
 * It's like the GPS system in your car combined with the game's minimap UI system.
 *
 * WHAT THIS SUBSYSTEM DOES:
 * -------------------------
 * 1. GPS NAVIGATION
 *    - Set destinations and get turn-by-turn directions
 *    - Calculate routes between any two points
 *    - Provide distance and ETA estimates
 *    - Voice guidance (optional "GPS Voice" feature)
 *
 * 2. POINTS OF INTEREST (POIs)
 *    - Race events, garages, car dealers, parts shops
 *    - Safe houses, meetup spots, rival encounters
 *    - Collectibles, photo spots, speed traps
 *    - Drift zones, speed zones, jump ramps
 *
 * 3. MINIMAP/MAP SYSTEM
 *    - Track player position and rotation
 *    - Custom waypoint markers
 *    - POI icons with distance indicators
 *    - Zoom control and rotation modes
 *
 * 4. MAP REGIONS/DISTRICTS
 *    - City divided into distinct regions
 *    - Regions can be locked/unlocked by reputation
 *    - Each region has unique police/racing activity levels
 *
 * KEY CONCEPTS:
 * -------------
 * 1. POINT OF INTEREST (FMGPointOfInterest)
 *    A location in the world that the player can interact with or navigate to.
 *    Has properties like:
 *    - Type (race, garage, collectible, etc.)
 *    - Position and interaction radius
 *    - Discovery state (undiscovered POIs appear as "?" on map)
 *    - Completion state (for collectibles and one-time events)
 *
 * 2. NAVIGATION ROUTE (FMGNavRoute)
 *    A calculated path from point A to point B containing:
 *    - Array of waypoints with turn instructions
 *    - Total distance and estimated time
 *    - Route preferences (avoid highways, avoid police, shortest path)
 *
 * 3. NAVIGATION INSTRUCTION (EMGNavInstruction)
 *    Turn-by-turn directions like "Turn Left", "Sharp Right", "U-Turn"
 *    Used for the GPS guidance overlay and voice prompts.
 *
 * 4. MAP REGION (FMGMapRegion)
 *    A district of the city with bounds, unlock requirements, and activity levels.
 *    Example: "Downtown" might have high police activity but many race events.
 *
 * HOW TO USE THIS SUBSYSTEM:
 * --------------------------
 * @code
 * // Get the subsystem (this is a WorldSubsystem, exists per-level)
 * UMGNavigationSubsystem* Nav = GetWorld()->GetSubsystem<UMGNavigationSubsystem>();
 *
 * // Set a destination to a POI
 * Nav->SetDestination(FName("GarageMain"));
 *
 * // Or set a destination to any world location
 * Nav->SetDestinationLocation(FVector(1000, 2000, 0));
 *
 * // Get distance to destination
 * float Distance = Nav->GetDistanceToDestination();
 *
 * // Get the next turn instruction
 * EMGNavInstruction NextTurn = Nav->GetCurrentInstruction();
 * float DistanceToTurn = Nav->GetDistanceToNextTurn();
 *
 * // Find nearby POIs
 * TArray<FMGPointOfInterest> NearbyPOIs = Nav->GetNearbyPOIs(PlayerLocation, 5000.0f);
 *
 * // Add a custom waypoint marker
 * FGuid MarkerId = Nav->AddCustomMarker(TargetLocation, FLinearColor::Yellow, FText::FromString("Meet here"));
 *
 * // Update player position (typically called every frame by the player vehicle)
 * Nav->UpdatePlayerPosition(VehicleLocation, VehicleRotation);
 * @endcode
 *
 * IMPORTANT NOTES:
 * ----------------
 * - This is a WORLD SUBSYSTEM (UWorldSubsystem), not a GameInstance subsystem.
 *   It gets created fresh for each level and is destroyed on level unload.
 *
 * - POI discovery and completion states should be persisted to save data.
 *   Use DiscoverPOI() when player finds a new location.
 *   Use CompletePOI() when player finishes an event at that location.
 *
 * - The minimap coordinate system converts world coordinates to 2D map space.
 *   Use WorldToMapCoordinates() and MapToWorldCoordinates() for conversion.
 *
 * EVENTS TO LISTEN FOR:
 * ---------------------
 * - OnDestinationSet: Player selected a new destination
 * - OnDestinationReached: Player arrived at destination
 * - OnNavInstructionChanged: Turn instruction updated (for UI/voice)
 * - OnPOIDiscovered: Player found a new point of interest
 * - OnRegionEntered: Player entered a new district
 *
 * @see UMGRaceFlowSubsystem - Interacts with POIs for race event starts
 * @see UMGMinimapWidget - UI widget that visualizes this subsystem's data
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGNavigationSubsystem.generated.h"

/**
 * Point of interest type - Categories of locations on the map
 *
 * POI types determine:
 * - The icon shown on the minimap/world map
 * - The color of the marker
 * - What happens when the player arrives (e.g., start race, open shop)
 * - Whether it appears on the minimap by default
 */
UENUM(BlueprintType)
enum class EMGPOIType : uint8
{
	/** Race event start point - player can initiate a race here */
	Race UMETA(DisplayName = "Race Event"),
	/** Player's garage - vehicle storage, customization, tuning */
	Garage UMETA(DisplayName = "Garage"),
	/** Dealership - purchase new vehicles */
	CarDealer UMETA(DisplayName = "Car Dealer"),
	/** Parts shop - buy upgrade parts and cosmetics */
	PartsShop UMETA(DisplayName = "Parts Shop"),
	/** Gas station - refuel, minor repairs, convenience items */
	GasStation UMETA(DisplayName = "Gas Station"),
	/** Safe house - save game, change vehicles, escape police */
	SafeHouse UMETA(DisplayName = "Safe House"),
	/** Street meetup - find other racers, crew events */
	Meetup UMETA(DisplayName = "Street Meetup"),
	/** Rival encounter - challenge a story rival to race */
	RivalEncounter UMETA(DisplayName = "Rival"),
	/** Hidden/secret area - unlocks something special */
	SecretArea UMETA(DisplayName = "Secret"),
	/** Collectible item location (billboards, tokens, etc.) */
	Collectible UMETA(DisplayName = "Collectible"),
	/** Photo opportunity spot for photography mode */
	PhotoSpot UMETA(DisplayName = "Photo Spot"),
	/** Speed trap camera - achieve target speed to complete */
	SpeedTrap UMETA(DisplayName = "Speed Trap"),
	/** Police station - avoid this area or lose heat */
	PoliceStation UMETA(DisplayName = "Police Station"),
	/** Shortcut entry point - secret route through the city */
	Shortcut UMETA(DisplayName = "Shortcut"),
	/** Jump ramp - get airtime for points/challenges */
	JumpRamp UMETA(DisplayName = "Jump Ramp"),
	/** Drift zone - score points for drifting through this area */
	DriftZone UMETA(DisplayName = "Drift Zone"),
	/** Speed zone - maintain high average speed through this area */
	SpeedZone UMETA(DisplayName = "Speed Zone"),
	/** Player-placed custom waypoint */
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
 * Point of interest data - Complete information about a map location
 *
 * POIs are the interactive locations throughout the game world.
 * They can be race starts, shops, collectibles, or any other location
 * the player might want to visit or navigate to.
 *
 * DISCOVERY SYSTEM:
 * Undiscovered POIs (bDiscovered = false) appear as "?" icons.
 * Once discovered, they show their actual icon and name.
 * This encourages exploration of the game world.
 */
USTRUCT(BlueprintType)
struct FMGPointOfInterest
{
	GENERATED_BODY()

	/** Unique identifier used to reference this POI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FName POIID;

	/** Human-readable name shown in UI (e.g., "Downtown Garage") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FText DisplayName;

	/** Category of this POI - determines icon and behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	EMGPOIType Type = EMGPOIType::Race;

	/** World-space position of this POI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FVector WorldLocation = FVector::ZeroVector;

	/** Facing direction for spawning/positioning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FRotator Rotation = FRotator::ZeroRotator;

	/** How close player must be to interact (in Unreal units, cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	float InteractionRadius = 500.0f;

	/** Custom icon texture for the map (uses default if null) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	TSoftObjectPtr<UTexture2D> MapIcon;

	/** Tint color for the map icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FLinearColor IconColor = FLinearColor::White;

	/** Should this POI appear on the world map? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bShowOnMap = true;

	/** Should this POI show a 3D marker when player is nearby? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bShowWhenClose = true;

	/** Max distance at which this POI is visible (0 = always visible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	float VisibilityDistance = 10000.0f;

	/** Has the player discovered this POI? False = shows as "?" on map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bDiscovered = false;

	/** Has the player completed this POI? (e.g., collected the collectible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bCompleted = false;

	/** ID of the event/mission associated with this POI (if any) */
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
 * Map region/district - A distinct area of the game world
 *
 * The game world is divided into regions (like city districts).
 * Each region has its own personality:
 * - Different types of events available
 * - Different levels of police/racing activity
 * - May be locked until player earns enough reputation
 *
 * Examples:
 * - "Downtown": High police activity, many street races
 * - "Industrial": Low police, lots of drag strips
 * - "Hills": Touge racing, scenic routes
 */
USTRUCT(BlueprintType)
struct FMGMapRegion
{
	GENERATED_BODY()

	/** Unique identifier for this region */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FName RegionID;

	/** Display name shown on map (e.g., "Downtown", "Industrial Zone") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FText DisplayName;

	/** Lower-left corner of region bounds in world coordinates (XY only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FVector2D MapBoundsMin = FVector2D::ZeroVector;

	/** Upper-right corner of region bounds in world coordinates (XY only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FVector2D MapBoundsMax = FVector2D::ZeroVector;

	/** Color used to highlight this region on the world map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FLinearColor RegionColor = FLinearColor::Gray;

	/** Is this region accessible to the player? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	bool bUnlocked = false;

	/** Reputation level required to unlock this region */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 RequiredREP = 0;

	/** Police patrol frequency in this region (0.0 = none, 1.0 = heavy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	float PoliceActivity = 0.5f;

	/** Street racing activity level (0.0 = dead, 1.0 = very active) */
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
