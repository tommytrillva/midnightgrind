// Copyright Midnight Grind. All Rights Reserved.

#include "Broadcast/MGBroadcastSubsystem.h"

void UMGBroadcastSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeCameraPresets();
}

void UMGBroadcastSubsystem::StartBroadcast(const FString& SessionID)
{
	if (bIsBroadcasting)
		return;

	CurrentSessionID = SessionID;
	bIsBroadcasting = true;
	bCleanFeed = false;

	// Set default camera
	SetCameraPreset(EMGBroadcastCameraPreset::WideStart);

	// Set default overlay
	CurrentOverlay = FMGBroadcastOverlay();
	CurrentOverlay.OverlayID = FName(TEXT("Default"));

	OnBroadcastStarted.Broadcast();
}

void UMGBroadcastSubsystem::EndBroadcast()
{
	if (!bIsBroadcasting)
		return;

	bIsBroadcasting = false;
	CurrentSessionID.Empty();
	DisablePictureInPicture();

	OnBroadcastEnded.Broadcast();
}

void UMGBroadcastSubsystem::SetCamera(const FMGBroadcastCamera& Camera)
{
	CurrentCamera = Camera;
	OnBroadcastCameraChanged.Broadcast(CurrentCamera);
}

void UMGBroadcastSubsystem::SetCameraPreset(EMGBroadcastCameraPreset Preset)
{
	FMGBroadcastCamera PresetCam = GetPresetCamera(Preset);
	SetCamera(PresetCam);

	// Update preset index for cycling
	CurrentPresetIndex = static_cast<int32>(Preset);
}

void UMGBroadcastSubsystem::CycleCamera()
{
	CurrentPresetIndex = (CurrentPresetIndex + 1) % static_cast<int32>(EMGBroadcastCameraPreset::Custom);
	EMGBroadcastCameraPreset NextPreset = static_cast<EMGBroadcastCameraPreset>(CurrentPresetIndex);
	SetCameraPreset(NextPreset);
}

void UMGBroadcastSubsystem::SetCameraTarget(const FString& PlayerID)
{
	CurrentCamera.TargetPlayerID = PlayerID;
	OnBroadcastCameraChanged.Broadcast(CurrentCamera);
}

void UMGBroadcastSubsystem::SetBlendTime(float Seconds)
{
	CurrentCamera.BlendTime = FMath::Clamp(Seconds, 0.0f, 5.0f);
}

void UMGBroadcastSubsystem::SetOverlay(const FMGBroadcastOverlay& Overlay)
{
	CurrentOverlay = Overlay;
	OnOverlayChanged.Broadcast(CurrentOverlay);
}

void UMGBroadcastSubsystem::SetCleanFeed(bool bClean)
{
	bCleanFeed = bClean;

	if (bCleanFeed)
	{
		// Disable all overlay elements
		CurrentOverlay.bShowLeaderboard = false;
		CurrentOverlay.bShowRaceProgress = false;
		CurrentOverlay.bShowDriverStats = false;
		CurrentOverlay.bShowMinimap = false;
		CurrentOverlay.bShowTimer = false;
		CurrentOverlay.bShowGapTimes = false;
		CurrentOverlay.bShowSpeedometer = false;
	}

	OnOverlayChanged.Broadcast(CurrentOverlay);
}

void UMGBroadcastSubsystem::EnablePictureInPicture(const FMGPictureInPicture& Settings)
{
	PiPSettings = Settings;
	PiPSettings.bEnabled = true;
}

void UMGBroadcastSubsystem::DisablePictureInPicture()
{
	PiPSettings.bEnabled = false;
	PiPSettings.bShowReplay = false;
}

void UMGBroadcastSubsystem::ShowReplayInPiP(float Duration)
{
	PiPSettings.bEnabled = true;
	PiPSettings.bShowReplay = true;

	// Would set timer to disable replay after duration
}

void UMGBroadcastSubsystem::RegisterCameraHotkey(int32 HotkeyIndex, const FMGBroadcastCamera& Camera)
{
	if (HotkeyIndex >= 0 && HotkeyIndex <= 9)
	{
		CameraHotkeys.Add(HotkeyIndex, Camera);
	}
}

void UMGBroadcastSubsystem::TriggerCameraHotkey(int32 HotkeyIndex)
{
	if (const FMGBroadcastCamera* HotkeyCamera = CameraHotkeys.Find(HotkeyIndex))
	{
		SetCamera(*HotkeyCamera);
	}
}

void UMGBroadcastSubsystem::SetOutputResolution(int32 Width, int32 Height)
{
	// Would configure render target resolution
	// Common broadcast resolutions: 1920x1080, 2560x1440, 3840x2160
}

void UMGBroadcastSubsystem::CaptureScreenshot(const FString& Filename)
{
	// Would capture high-res screenshot for broadcast graphics
}

void UMGBroadcastSubsystem::InitializeCameraPresets()
{
	// Wide establishing shot
	FMGBroadcastCamera WideStart;
	WideStart.CameraID = FName(TEXT("WideStart"));
	WideStart.Preset = EMGBroadcastCameraPreset::WideStart;
	WideStart.FOV = 90.0f;
	WideStart.BlendTime = 2.0f;
	WideStart.bUseDepthOfField = false;
	PresetCameras.Add(EMGBroadcastCameraPreset::WideStart, WideStart);

	// Helicopter follow cam
	FMGBroadcastCamera Helicopter;
	Helicopter.CameraID = FName(TEXT("HelicopterFollow"));
	Helicopter.Preset = EMGBroadcastCameraPreset::HelicopterFollow;
	Helicopter.FOV = 60.0f;
	Helicopter.BlendTime = 1.5f;
	Helicopter.bUseMotionBlur = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::HelicopterFollow, Helicopter);

	// Track side cameras
	FMGBroadcastCamera TrackSideA;
	TrackSideA.CameraID = FName(TEXT("TrackSideA"));
	TrackSideA.Preset = EMGBroadcastCameraPreset::TrackSideA;
	TrackSideA.FOV = 45.0f;
	TrackSideA.BlendTime = 0.5f;
	TrackSideA.bUseDepthOfField = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::TrackSideA, TrackSideA);

	FMGBroadcastCamera TrackSideB;
	TrackSideB.CameraID = FName(TEXT("TrackSideB"));
	TrackSideB.Preset = EMGBroadcastCameraPreset::TrackSideB;
	TrackSideB.FOV = 50.0f;
	TrackSideB.BlendTime = 0.5f;
	TrackSideB.bUseDepthOfField = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::TrackSideB, TrackSideB);

	// Onboard cameras
	FMGBroadcastCamera OnboardLeader;
	OnboardLeader.CameraID = FName(TEXT("OnboardLeader"));
	OnboardLeader.Preset = EMGBroadcastCameraPreset::OnboardLeader;
	OnboardLeader.FOV = 75.0f;
	OnboardLeader.BlendTime = 1.0f;
	OnboardLeader.bUseMotionBlur = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::OnboardLeader, OnboardLeader);

	FMGBroadcastCamera OnboardBattle;
	OnboardBattle.CameraID = FName(TEXT("OnboardBattle"));
	OnboardBattle.Preset = EMGBroadcastCameraPreset::OnboardBattle;
	OnboardBattle.FOV = 80.0f;
	OnboardBattle.BlendTime = 0.75f;
	OnboardBattle.bUseMotionBlur = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::OnboardBattle, OnboardBattle);

	// Finish line
	FMGBroadcastCamera FinishLine;
	FinishLine.CameraID = FName(TEXT("FinishLine"));
	FinishLine.Preset = EMGBroadcastCameraPreset::FinishLine;
	FinishLine.FOV = 35.0f;
	FinishLine.BlendTime = 1.0f;
	FinishLine.bUseDepthOfField = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::FinishLine, FinishLine);

	// Podium ceremony
	FMGBroadcastCamera Podium;
	Podium.CameraID = FName(TEXT("PodiumCeremony"));
	Podium.Preset = EMGBroadcastCameraPreset::PodiumCeremony;
	Podium.FOV = 55.0f;
	Podium.BlendTime = 2.0f;
	Podium.bUseDepthOfField = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::PodiumCeremony, Podium);

	// Replay cam
	FMGBroadcastCamera Replay;
	Replay.CameraID = FName(TEXT("Replay"));
	Replay.Preset = EMGBroadcastCameraPreset::Replay;
	Replay.FOV = 60.0f;
	Replay.BlendTime = 0.5f;
	Replay.bUseMotionBlur = true;
	Replay.bUseDepthOfField = true;
	PresetCameras.Add(EMGBroadcastCameraPreset::Replay, Replay);
}

FMGBroadcastCamera UMGBroadcastSubsystem::GetPresetCamera(EMGBroadcastCameraPreset Preset) const
{
	if (const FMGBroadcastCamera* PresetCam = PresetCameras.Find(Preset))
	{
		return *PresetCam;
	}

	return FMGBroadcastCamera();
}
