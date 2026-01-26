// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "MGSpectatorPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputComponent;
class UMGSpectatorSubsystem;

/**
 * Spectator Pawn
 * Custom spectator pawn with camera controls
 */
UCLASS()
class MIDNIGHTGRIND_API AMGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AMGSpectatorPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ==========================================
	// CAMERA
	// ==========================================

	/** Get camera component */
	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	/** Set field of view */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetFieldOfView(float FOV);

	/** Get field of view */
	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetFieldOfView() const;

	/** Reset camera to default FOV */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetFieldOfView();

	// ==========================================
	// MOVEMENT
	// ==========================================

	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 1000.0f;

	/** Fast movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FastMoveSpeed = 3000.0f;

	/** Slow movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SlowMoveSpeed = 200.0f;

	/** Look sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LookSensitivity = 1.0f;

	/** Is fast mode active */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bFastMode = false;

	/** Is slow mode active */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bSlowMode = false;

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	// ==========================================
	// SUBSYSTEM REFERENCE
	// ==========================================

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	// ==========================================
	// INPUT
	// ==========================================

	/** Move forward/backward */
	void MoveForward(float Value);

	/** Move right/left */
	void MoveRight(float Value);

	/** Move up/down */
	void MoveUp(float Value);

	/** Look up/down */
	void LookUp(float Value);

	/** Look right/left */
	void Turn(float Value);

	/** Start fast mode */
	void StartFastMode();

	/** Stop fast mode */
	void StopFastMode();

	/** Start slow mode */
	void StartSlowMode();

	/** Stop slow mode */
	void StopSlowMode();

	/** Cycle camera mode */
	void CycleCameraMode();

	/** Cycle target next */
	void CycleTargetNext();

	/** Cycle target previous */
	void CycleTargetPrevious();

	/** Zoom in */
	void ZoomIn();

	/** Zoom out */
	void ZoomOut();

	/** Toggle auto-director */
	void ToggleAutoDirector();

	/** Exit spectator mode */
	void ExitSpectator();

	// ==========================================
	// STATE
	// ==========================================

	/** Default FOV */
	float DefaultFOV = 90.0f;

	/** Current FOV */
	float CurrentFOV = 90.0f;

	/** Min FOV (zoomed in) */
	float MinFOV = 30.0f;

	/** Max FOV (zoomed out) */
	float MaxFOV = 120.0f;

	/** FOV zoom speed */
	float FOVZoomSpeed = 30.0f;

	/** Target FOV for smooth zoom */
	float TargetFOV = 90.0f;
};

/**
 * Spectator Camera Actor
 * Placeable camera for broadcast/trackside views
 */
UCLASS()
class MIDNIGHTGRIND_API AMGSpectatorCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AMGSpectatorCameraActor();

	virtual void BeginPlay() override;

	/** Get camera settings */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FMGBroadcastCameraPoint GetCameraSettings() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	/** Camera priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 Priority = 1;

	/** Track range for this camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector2D TrackRange = FVector2D(0.0f, 100.0f);

	/** Is zoom camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsZoomCamera = false;

	/** Auto-track targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoTrack = true;

	/** Auto-register on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoRegister = true;
};
