// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MG_PLYR_State.h"
#include "Net/UnrealNetwork.h"

AMGPlayerState::AMGPlayerState()
{
	bReplicates = true;
	NetUpdateFrequency = 30.0f; // Higher update rate for racing game
}

void AMGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGPlayerState, PlatformID);
	DOREPLIFETIME(AMGPlayerState, ProfileLevel);
	DOREPLIFETIME(AMGPlayerState, CrewName);
	DOREPLIFETIME(AMGPlayerState, bIsSessionHost);
	DOREPLIFETIME(AMGPlayerState, ReadyState);
	DOREPLIFETIME(AMGPlayerState, VehicleSelection);
	DOREPLIFETIME(AMGPlayerState, RaceStatus);
	DOREPLIFETIME(AMGPlayerState, RaceSnapshot);
	DOREPLIFETIME(AMGPlayerState, LapTimes);
	DOREPLIFETIME(AMGPlayerState, FinishPosition);
}

void AMGPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AMGPlayerState* MG_PLYR_State = Cast<AMGPlayerState>(PlayerState))
	{
		// Copy our custom properties when seamless traveling
		MG_PLYR_State->PlatformID = PlatformID;
		MG_PLYR_State->ProfileLevel = ProfileLevel;
		MG_PLYR_State->CrewName = CrewName;
		MG_PLYR_State->VehicleSelection = VehicleSelection;
	}
}

void AMGPlayerState::SetReadyState(EMGPlayerReadyState NewState)
{
	if (HasAuthority())
	{
		if (ReadyState != NewState)
		{
			ReadyState = NewState;
			OnRep_ReadyState();
		}
	}
}

void AMGPlayerState::ServerToggleReady_Implementation()
{
	if (ReadyState == EMGPlayerReadyState::NotReady)
	{
		SetReadyState(EMGPlayerReadyState::Ready);
	}
	else if (ReadyState == EMGPlayerReadyState::Ready)
	{
		SetReadyState(EMGPlayerReadyState::NotReady);
	}
}

void AMGPlayerState::SelectVehicle(const FMGVehicleSelection& Selection)
{
	if (HasAuthority())
	{
		VehicleSelection = Selection;
	}
	else
	{
		ServerSelectVehicle(Selection);
	}
}

void AMGPlayerState::ServerSelectVehicle_Implementation(const FMGVehicleSelection& Selection)
{
	VehicleSelection = Selection;
}

void AMGPlayerState::AuthUpdateRaceSnapshot(const FMGRaceSnapshot& NewSnapshot)
{
	if (!HasAuthority())
	{
		return;
	}

	// Store previous values for change detection
	PreviousPosition = RaceSnapshot.Position;
	PreviousLap = RaceSnapshot.CurrentLap;

	RaceSnapshot = NewSnapshot;

	// Trigger rep notify on server too
	OnRep_RaceSnapshot();
}

void AMGPlayerState::AuthSetRaceStatus(EMGPlayerRaceStatus NewStatus)
{
	if (!HasAuthority())
	{
		return;
	}

	if (RaceStatus != NewStatus)
	{
		RaceStatus = NewStatus;
		OnRep_RaceStatus();
	}
}

void AMGPlayerState::AuthRecordLapTime(float LapTime)
{
	if (!HasAuthority())
	{
		return;
	}

	LapTimes.Add(LapTime);

	// Update best lap time
	if (LapTime < RaceSnapshot.BestLapTime || RaceSnapshot.BestLapTime <= 0.0f)
	{
		RaceSnapshot.BestLapTime = LapTime;
	}
}

void AMGPlayerState::AuthSetFinishPosition(int32 Position)
{
	if (!HasAuthority())
	{
		return;
	}

	FinishPosition = Position;
}

void AMGPlayerState::AuthSetSessionHost(bool bIsHost)
{
	if (HasAuthority())
	{
		bIsSessionHost = bIsHost;
	}
}

void AMGPlayerState::AuthInitializeFromPlatform(const FString& InPlatformID, const FString& InDisplayName, int32 InLevel, const FString& InCrew)
{
	if (!HasAuthority())
	{
		return;
	}

	PlatformID = InPlatformID;
	SetPlayerName(InDisplayName);
	ProfileLevel = InLevel;
	CrewName = InCrew;
}

// ==========================================
// REP NOTIFIES
// ==========================================

void AMGPlayerState::OnRep_ReadyState()
{
	OnReadyStateChanged.Broadcast(this, ReadyState);
}

void AMGPlayerState::OnRep_RaceStatus()
{
	OnRaceStatusChanged.Broadcast(this, RaceStatus);
}

void AMGPlayerState::OnRep_RaceSnapshot()
{
	// Check for position change
	if (RaceSnapshot.Position != PreviousPosition)
	{
		OnPositionChanged.Broadcast(this, RaceSnapshot.Position);
		PreviousPosition = RaceSnapshot.Position;
	}

	// Check for lap change
	if (RaceSnapshot.CurrentLap != PreviousLap)
	{
		OnLapChanged.Broadcast(this, RaceSnapshot.CurrentLap);
		PreviousLap = RaceSnapshot.CurrentLap;
	}
}
