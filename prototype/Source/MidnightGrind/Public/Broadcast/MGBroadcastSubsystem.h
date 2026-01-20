// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBroadcastSubsystem.generated.h"

/**
 * Broadcast System - For esports and content creation
 * - Multiple camera presets for cinematic shots
 * - Overlay management for stats/graphics
 * - Smooth transitions between cameras
 * - Picture-in-picture for replays
 * - Clean feed options (no UI) for production
 */

UENUM(BlueprintType)
enum class EMGBroadcastCameraPreset : uint8
{
	WideStart,
	HelicopterFollow,
	TrackSideA,
	TrackSideB,
	OnboardLeader,
	OnboardBattle,
	FinishLine,
	PodiumCeremony,
	Replay,
	Custom
};

USTRUCT(BlueprintType)
struct FMGBroadcastCamera
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CameraID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBroadcastCameraPreset Preset = EMGBroadcastCameraPreset::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDepthOfField = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMotionBlur = true;
};

USTRUCT(BlueprintType)
struct FMGBroadcastOverlay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OverlayID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLeaderboard = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRaceProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDriverStats = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMinimap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTimer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGapTimes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedometer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayOpacity = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGPictureInPicture
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D(0.7f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size = FVector2D(0.25f, 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowReplay = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBroadcastCameraChanged, const FMGBroadcastCamera&, Camera);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnOverlayChanged, const FMGBroadcastOverlay&, Overlay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBroadcastStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBroadcastEnded);

UCLASS()
class MIDNIGHTGRIND_API UMGBroadcastSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Broadcast Mode
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void StartBroadcast(const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void EndBroadcast();

	UFUNCTION(BlueprintPure, Category = "Broadcast")
	bool IsBroadcasting() const { return bIsBroadcasting; }

	// Camera Control
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCamera(const FMGBroadcastCamera& Camera);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCameraPreset(EMGBroadcastCameraPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void CycleCamera();

	UFUNCTION(BlueprintPure, Category = "Broadcast|Camera")
	FMGBroadcastCamera GetCurrentCamera() const { return CurrentCamera; }

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCameraTarget(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetBlendTime(float Seconds);

	// Overlay Control
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Overlay")
	void SetOverlay(const FMGBroadcastOverlay& Overlay);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Overlay")
	void SetCleanFeed(bool bClean);

	UFUNCTION(BlueprintPure, Category = "Broadcast|Overlay")
	bool IsCleanFeed() const { return bCleanFeed; }

	UFUNCTION(BlueprintPure, Category = "Broadcast|Overlay")
	FMGBroadcastOverlay GetCurrentOverlay() const { return CurrentOverlay; }

	// Picture-in-Picture
	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void EnablePictureInPicture(const FMGPictureInPicture& Settings);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void DisablePictureInPicture();

	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void ShowReplayInPiP(float Duration);

	UFUNCTION(BlueprintPure, Category = "Broadcast|PiP")
	FMGPictureInPicture GetPictureInPictureSettings() const { return PiPSettings; }

	// Hotkeys/Shortcuts
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void RegisterCameraHotkey(int32 HotkeyIndex, const FMGBroadcastCamera& Camera);

	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void TriggerCameraHotkey(int32 HotkeyIndex);

	// Output
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Output")
	void SetOutputResolution(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable, Category = "Broadcast|Output")
	void CaptureScreenshot(const FString& Filename);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastCameraChanged OnBroadcastCameraChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnOverlayChanged OnOverlayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastStarted OnBroadcastStarted;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastEnded OnBroadcastEnded;

protected:
	void InitializeCameraPresets();
	FMGBroadcastCamera GetPresetCamera(EMGBroadcastCameraPreset Preset) const;

private:
	FMGBroadcastCamera CurrentCamera;
	FMGBroadcastOverlay CurrentOverlay;
	FMGPictureInPicture PiPSettings;
	TMap<int32, FMGBroadcastCamera> CameraHotkeys;
	TMap<EMGBroadcastCameraPreset, FMGBroadcastCamera> PresetCameras;
	FString CurrentSessionID;
	bool bIsBroadcasting = false;
	bool bCleanFeed = false;
	int32 CurrentPresetIndex = 0;
};
