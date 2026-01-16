// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGSpectatorSubsystem.generated.h"

class AMGSpectatorPawn;
class APlayerController;
class APawn;
class UCameraComponent;

/**
 * Spectator camera mode
 */
UENUM(BlueprintType)
enum class EMGSpectatorCameraMode : uint8
{
	/** Free camera - full control */
	FreeCam,
	/** Follow behind vehicle */
	Chase,
	/** Orbit around vehicle */
	Orbit,
	/** Cockpit/interior view */
	Cockpit,
	/** Hood cam */
	Hood,
	/** Bumper cam */
	Bumper,
	/** TV-style broadcast cameras */
	Broadcast,
	/** Helicopter view */
	Helicopter,
	/** Track-side fixed cameras */
	TrackSide,
	/** Cinematic auto-director */
	Director
};

/**
 * Spectator target info
 */
USTRUCT(BlueprintType)
struct FMGSpectatorTarget
{
	GENERATED_BODY()

	/** Target actor */
	UPROPERTY(BlueprintReadOnly)
	AActor* Target = nullptr;

	/** Target name */
	UPROPERTY(BlueprintReadOnly)
	FText TargetName;

	/** Current position in race */
	UPROPERTY(BlueprintReadOnly)
	int32 RacePosition = 0;

	/** Current lap */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 0;

	/** Current speed */
	UPROPERTY(BlueprintReadOnly)
	float CurrentSpeed = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/** Is local player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;

	/** Is AI */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAI = false;

	/** Team/crew color */
	UPROPERTY(BlueprintReadOnly)
	FLinearColor TeamColor = FLinearColor::White;
};

/**
 * Broadcast camera point
 */
USTRUCT(BlueprintType)
struct FMGBroadcastCameraPoint
{
	GENERATED_BODY()

	/** Camera location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Camera rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	/** Priority (higher = more likely to be used) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 1;

	/** Track range (min/max distance along track where this camera is valid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D TrackRange = FVector2D(0.0f, 100.0f);

	/** Is zoom camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsZoomCamera = false;

	/** Auto-track targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoTrack = true;
};

/**
 * Camera cut/transition info
 */
USTRUCT(BlueprintType)
struct FMGCameraCut
{
	GENERATED_BODY()

	/** Previous camera mode */
	UPROPERTY(BlueprintReadOnly)
	EMGSpectatorCameraMode FromMode;

	/** New camera mode */
	UPROPERTY(BlueprintReadOnly)
	EMGSpectatorCameraMode ToMode;

	/** Cut time */
	UPROPERTY(BlueprintReadOnly)
	float CutTime = 0.0f;

	/** Is smooth transition */
	UPROPERTY(BlueprintReadOnly)
	bool bSmoothTransition = false;
};

/**
 * Spectator overlay info
 */
USTRUCT(BlueprintType)
struct FMGSpectatorOverlay
{
	GENERATED_BODY()

	/** Show race standings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowStandings = true;

	/** Show current target info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTargetInfo = true;

	/** Show lap times */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLapTimes = true;

	/** Show speedometer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedometer = true;

	/** Show mini-map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMiniMap = true;

	/** Show camera info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCameraInfo = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectatorTargetChanged, const FMGSpectatorTarget&, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectatorCameraModeChanged, EMGSpectatorCameraMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectatorCameraCut, const FMGCameraCut&, CutInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpectatorModeEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpectatorModeExited);

/**
 * Spectator Subsystem
 * Manages spectator camera and target tracking
 *
 * Features:
 * - Multiple camera modes
 * - Target cycling
 * - Auto-director for cinematic viewing
 * - Broadcast-style cameras
 * - Smooth camera transitions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSpectatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatorTargetChanged OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatorCameraModeChanged OnCameraModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatorCameraCut OnCameraCut;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatorModeEntered OnSpectatorModeEntered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatorModeExited OnSpectatorModeExited;

	// ==========================================
	// SPECTATOR MODE
	// ==========================================

	/** Enter spectator mode */
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void EnterSpectatorMode(APlayerController* Controller);

	/** Exit spectator mode */
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void ExitSpectatorMode(APlayerController* Controller);

	/** Is in spectator mode */
	UFUNCTION(BlueprintPure, Category = "Spectator")
	bool IsInSpectatorMode() const { return bIsSpectating; }

	/** Get spectator pawn */
	UFUNCTION(BlueprintPure, Category = "Spectator")
	AMGSpectatorPawn* GetSpectatorPawn() const { return SpectatorPawn; }

	// ==========================================
	// CAMERA MODE
	// ==========================================

	/** Set camera mode */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(EMGSpectatorCameraMode NewMode);

	/** Get current camera mode */
	UFUNCTION(BlueprintPure, Category = "Camera")
	EMGSpectatorCameraMode GetCameraMode() const { return CurrentCameraMode; }

	/** Cycle to next camera mode */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CycleNextCameraMode();

	/** Cycle to previous camera mode */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CyclePreviousCameraMode();

	/** Get camera mode display name */
	UFUNCTION(BlueprintPure, Category = "Camera")
	static FText GetCameraModeDisplayName(EMGSpectatorCameraMode Mode);

	// ==========================================
	// TARGET TRACKING
	// ==========================================

	/** Set spectator target */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetTarget(AActor* NewTarget);

	/** Get current target */
	UFUNCTION(BlueprintPure, Category = "Target")
	FMGSpectatorTarget GetCurrentTarget() const { return CurrentTarget; }

	/** Cycle to next target */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void CycleNextTarget();

	/** Cycle to previous target */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void CyclePreviousTarget();

	/** Focus on race leader */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void FocusOnLeader();

	/** Focus on local player */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void FocusOnLocalPlayer();

	/** Focus on position */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void FocusOnPosition(int32 Position);

	/** Get all available targets */
	UFUNCTION(BlueprintPure, Category = "Target")
	TArray<FMGSpectatorTarget> GetAllTargets() const;

	/** Register spectatable actor */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void RegisterTarget(AActor* Target, FText DisplayName, bool bIsAI = false);

	/** Unregister spectatable actor */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void UnregisterTarget(AActor* Target);

	// ==========================================
	// AUTO-DIRECTOR
	// ==========================================

	/** Enable auto-director mode */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void EnableAutoDirector(bool bEnabled);

	/** Is auto-director enabled */
	UFUNCTION(BlueprintPure, Category = "Director")
	bool IsAutoDirectorEnabled() const { return bAutoDirectorEnabled; }

	/** Set auto-director cut interval */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void SetDirectorCutInterval(float MinSeconds, float MaxSeconds);

	/** Set auto-director target switch interval */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void SetDirectorTargetInterval(float MinSeconds, float MaxSeconds);

	// ==========================================
	// BROADCAST CAMERAS
	// ==========================================

	/** Register broadcast camera point */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void RegisterBroadcastCamera(const FMGBroadcastCameraPoint& CameraPoint);

	/** Clear broadcast cameras */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void ClearBroadcastCameras();

	/** Get broadcast cameras */
	UFUNCTION(BlueprintPure, Category = "Broadcast")
	TArray<FMGBroadcastCameraPoint> GetBroadcastCameras() const { return BroadcastCameras; }

	/** Force use specific broadcast camera */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void ForceUseBroadcastCamera(int32 CameraIndex);

	// ==========================================
	// OVERLAY SETTINGS
	// ==========================================

	/** Get overlay settings */
	UFUNCTION(BlueprintPure, Category = "Overlay")
	FMGSpectatorOverlay GetOverlaySettings() const { return OverlaySettings; }

	/** Set overlay settings */
	UFUNCTION(BlueprintCallable, Category = "Overlay")
	void SetOverlaySettings(const FMGSpectatorOverlay& Settings);

	/** Toggle overlay element */
	UFUNCTION(BlueprintCallable, Category = "Overlay")
	void ToggleOverlayElement(FName ElementName);

	// ==========================================
	// FREE CAM CONTROL
	// ==========================================

	/** Set free cam speed */
	UFUNCTION(BlueprintCallable, Category = "FreeCam")
	void SetFreeCamSpeed(float Speed) { FreeCamSpeed = Speed; }

	/** Get free cam speed */
	UFUNCTION(BlueprintPure, Category = "FreeCam")
	float GetFreeCamSpeed() const { return FreeCamSpeed; }

	/** Set free cam position */
	UFUNCTION(BlueprintCallable, Category = "FreeCam")
	void SetFreeCamPosition(FVector Position, FRotator Rotation);

	// ==========================================
	// CHASE CAM SETTINGS
	// ==========================================

	/** Set chase cam distance */
	UFUNCTION(BlueprintCallable, Category = "ChaseCam")
	void SetChaseDistance(float Distance) { ChaseDistance = Distance; }

	/** Set chase cam height */
	UFUNCTION(BlueprintCallable, Category = "ChaseCam")
	void SetChaseHeight(float Height) { ChaseHeight = Height; }

	/** Set chase cam lag */
	UFUNCTION(BlueprintCallable, Category = "ChaseCam")
	void SetChaseLag(float Lag) { ChaseLagSpeed = Lag; }

	// ==========================================
	// ORBIT CAM SETTINGS
	// ==========================================

	/** Set orbit distance */
	UFUNCTION(BlueprintCallable, Category = "OrbitCam")
	void SetOrbitDistance(float Distance) { OrbitDistance = Distance; }

	/** Set orbit angle */
	UFUNCTION(BlueprintCallable, Category = "OrbitCam")
	void SetOrbitAngle(float YawAngle, float PitchAngle);

	/** Auto-rotate orbit */
	UFUNCTION(BlueprintCallable, Category = "OrbitCam")
	void SetOrbitAutoRotate(bool bEnabled, float Speed = 30.0f);

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Is spectating */
	bool bIsSpectating = false;

	/** Current camera mode */
	EMGSpectatorCameraMode CurrentCameraMode = EMGSpectatorCameraMode::Chase;

	/** Previous camera mode */
	EMGSpectatorCameraMode PreviousCameraMode = EMGSpectatorCameraMode::Chase;

	/** Current target */
	FMGSpectatorTarget CurrentTarget;

	/** All available targets */
	UPROPERTY()
	TArray<FMGSpectatorTarget> AvailableTargets;

	/** Current target index */
	int32 CurrentTargetIndex = 0;

	// ==========================================
	// SPECTATOR PAWN
	// ==========================================

	/** Spectator pawn class */
	UPROPERTY()
	TSubclassOf<AMGSpectatorPawn> SpectatorPawnClass;

	/** Current spectator pawn */
	UPROPERTY()
	AMGSpectatorPawn* SpectatorPawn;

	/** Original player pawn */
	UPROPERTY()
	APawn* OriginalPawn;

	/** Spectating controller */
	UPROPERTY()
	APlayerController* SpectatingController;

	// ==========================================
	// AUTO-DIRECTOR
	// ==========================================

	/** Auto-director enabled */
	bool bAutoDirectorEnabled = false;

	/** Time until next camera cut */
	float DirectorNextCutTime = 0.0f;

	/** Time until next target switch */
	float DirectorNextTargetTime = 0.0f;

	/** Cut interval range */
	FVector2D DirectorCutInterval = FVector2D(5.0f, 15.0f);

	/** Target switch interval range */
	FVector2D DirectorTargetInterval = FVector2D(10.0f, 30.0f);

	// ==========================================
	// BROADCAST CAMERAS
	// ==========================================

	/** Broadcast camera points */
	UPROPERTY()
	TArray<FMGBroadcastCameraPoint> BroadcastCameras;

	/** Current broadcast camera index */
	int32 CurrentBroadcastCameraIndex = 0;

	/** Forced broadcast camera index (-1 = auto) */
	int32 ForcedBroadcastCameraIndex = -1;

	// ==========================================
	// OVERLAY
	// ==========================================

	/** Overlay settings */
	FMGSpectatorOverlay OverlaySettings;

	// ==========================================
	// CAMERA SETTINGS
	// ==========================================

	/** Free cam speed */
	float FreeCamSpeed = 1000.0f;

	/** Chase distance */
	float ChaseDistance = 600.0f;

	/** Chase height */
	float ChaseHeight = 200.0f;

	/** Chase lag speed */
	float ChaseLagSpeed = 10.0f;

	/** Orbit distance */
	float OrbitDistance = 800.0f;

	/** Orbit yaw */
	float OrbitYaw = 0.0f;

	/** Orbit pitch */
	float OrbitPitch = -20.0f;

	/** Orbit auto-rotate */
	bool bOrbitAutoRotate = false;

	/** Orbit auto-rotate speed */
	float OrbitAutoRotateSpeed = 30.0f;

	// ==========================================
	// CAMERA TRANSITION
	// ==========================================

	/** Is transitioning cameras */
	bool bIsTransitioning = false;

	/** Transition progress */
	float TransitionProgress = 0.0f;

	/** Transition duration */
	float TransitionDuration = 0.5f;

	/** Transition start transform */
	FTransform TransitionStartTransform;

	/** Transition end transform */
	FTransform TransitionEndTransform;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update auto-director */
	void UpdateAutoDirector(float DeltaTime);

	/** Update camera position */
	void UpdateCameraPosition(float DeltaTime);

	/** Update chase camera */
	void UpdateChaseCamera(float DeltaTime);

	/** Update orbit camera */
	void UpdateOrbitCamera(float DeltaTime);

	/** Update broadcast camera */
	void UpdateBroadcastCamera(float DeltaTime);

	/** Update helicopter camera */
	void UpdateHelicopterCamera(float DeltaTime);

	/** Update trackside camera */
	void UpdateTracksideCamera(float DeltaTime);

	/** Update camera transition */
	void UpdateCameraTransition(float DeltaTime);

	/** Select best broadcast camera */
	int32 SelectBestBroadcastCamera();

	/** Select interesting target (for auto-director) */
	AActor* SelectInterestingTarget();

	/** Select dramatic camera mode (for auto-director) */
	EMGSpectatorCameraMode SelectDramaticCameraMode();

	/** Update target info */
	void UpdateTargetInfo();

	/** Sort targets by position */
	void SortTargetsByPosition();

	/** Spawn spectator pawn */
	void SpawnSpectatorPawn(APlayerController* Controller);

	/** Destroy spectator pawn */
	void DestroySpectatorPawn();

	/** Begin camera transition */
	void BeginCameraTransition(const FTransform& StartTransform, const FTransform& EndTransform, float Duration);
};
