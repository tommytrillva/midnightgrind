// Copyright Midnight Grind. All Rights Reserved.

#include "Race/MGRaceCountdownManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGRaceCountdownManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMGRaceCountdownManager::Deinitialize()
{
	CancelCountdown();
	Super::Deinitialize();
}

// ==========================================
// COUNTDOWN CONTROL
// ==========================================

void UMGRaceCountdownManager::StartCountdown(int32 FromValue, EMGCountdownStyle Style)
{
	StartValue = FMath::Max(FromValue, 1);
	CurrentStyle = Style;

	// Set timing based on style
	switch (Style)
	{
	case EMGCountdownStyle::Classic:
		PreDelayDuration = 0.5f;
		TickDuration = 1.0f;
		GoDuration = 1.0f;
		break;

	case EMGCountdownStyle::TrafficLights:
		PreDelayDuration = 1.0f;
		TickDuration = 0.8f;
		GoDuration = 0.5f;
		break;

	case EMGCountdownStyle::ChristmasTree:
		PreDelayDuration = 0.5f;
		TickDuration = 0.5f; // Faster tree staging
		GoDuration = 0.5f;
		break;

	case EMGCountdownStyle::Wangan:
		PreDelayDuration = 0.3f;
		TickDuration = 0.7f;
		GoDuration = 0.5f;
		break;

	default:
		break;
	}

	StartCountdownCustom(StartValue, PreDelayDuration, TickDuration, GoDuration);
}

void UMGRaceCountdownManager::StartCountdownCustom(int32 FromValue, float PreDelaySeconds, float TickDurationSeconds, float GoDurationSeconds)
{
	// Cancel any existing countdown
	CancelCountdown();

	StartValue = FMath::Max(FromValue, 1);
	PreDelayDuration = PreDelaySeconds;
	TickDuration = TickDurationSeconds;
	GoDuration = GoDurationSeconds;

	CurrentValue = StartValue;
	TotalElapsedTime = 0.0f;
	CurrentTickTimer = 0.0f;
	bIsPaused = false;

	// Start with pre-delay
	if (PreDelayDuration > 0.0f)
	{
		SetState(EMGCountdownState::PreDelay);
		CurrentTickTimer = PreDelayDuration;
	}
	else
	{
		SetState(EMGCountdownState::Counting);
		CurrentTickTimer = TickDuration;
		OnCountdownTick.Broadcast(CurrentValue);
		PlayTickSound(CurrentValue);
	}

	OnCountdownStarted.Broadcast();

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGRaceCountdownManager::OnTick,
			1.0f / 60.0f, // 60 Hz update for smooth progress
			true
		);
	}
}

void UMGRaceCountdownManager::CancelCountdown()
{
	if (CurrentState == EMGCountdownState::Inactive || CurrentState == EMGCountdownState::Complete)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	SetState(EMGCountdownState::Inactive);
	OnCountdownCancelled.Broadcast();
}

void UMGRaceCountdownManager::PauseCountdown()
{
	if (IsCountdownActive() && !bIsPaused)
	{
		bIsPaused = true;
	}
}

void UMGRaceCountdownManager::ResumeCountdown()
{
	if (IsCountdownActive() && bIsPaused)
	{
		bIsPaused = false;
	}
}

void UMGRaceCountdownManager::SkipToGo()
{
	if (CurrentState == EMGCountdownState::Inactive || CurrentState == EMGCountdownState::Complete)
	{
		return;
	}

	EnterGoState();
}

// ==========================================
// STATE QUERIES
// ==========================================

FMGCountdownTick UMGRaceCountdownManager::GetTickData() const
{
	FMGCountdownTick Data;
	Data.Value = CurrentValue;
	Data.TimeRemaining = CurrentTickTimer;
	Data.TotalElapsed = TotalElapsedTime;
	Data.bIsGo = (CurrentState == EMGCountdownState::Go);

	// Calculate progress through current tick
	float CurrentDuration = TickDuration;
	if (CurrentState == EMGCountdownState::PreDelay)
	{
		CurrentDuration = PreDelayDuration;
	}
	else if (CurrentState == EMGCountdownState::Go)
	{
		CurrentDuration = GoDuration;
	}

	if (CurrentDuration > 0.0f)
	{
		Data.Progress = 1.0f - (CurrentTickTimer / CurrentDuration);
	}

	return Data;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGRaceCountdownManager::OnTick()
{
	if (bIsPaused)
	{
		return;
	}

	float DeltaTime = 1.0f / 60.0f;
	UpdateCountdown(DeltaTime);
}

void UMGRaceCountdownManager::UpdateCountdown(float DeltaTime)
{
	TotalElapsedTime += DeltaTime;
	CurrentTickTimer -= DeltaTime;

	if (CurrentTickTimer <= 0.0f)
	{
		switch (CurrentState)
		{
		case EMGCountdownState::PreDelay:
			// Pre-delay complete, start counting
			SetState(EMGCountdownState::Counting);
			CurrentTickTimer = TickDuration;
			OnCountdownTick.Broadcast(CurrentValue);
			PlayTickSound(CurrentValue);
			break;

		case EMGCountdownState::Counting:
			NextTick();
			break;

		case EMGCountdownState::Go:
			CompleteCountdown();
			break;

		default:
			break;
		}
	}
}

void UMGRaceCountdownManager::NextTick()
{
	CurrentValue--;

	if (CurrentValue <= 0)
	{
		EnterGoState();
	}
	else
	{
		CurrentTickTimer = TickDuration;
		OnCountdownTick.Broadcast(CurrentValue);
		PlayTickSound(CurrentValue);
	}
}

void UMGRaceCountdownManager::EnterGoState()
{
	CurrentValue = 0;
	SetState(EMGCountdownState::Go);
	CurrentTickTimer = GoDuration;

	OnCountdownGo.Broadcast();
	PlayGoSound();
}

void UMGRaceCountdownManager::CompleteCountdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	SetState(EMGCountdownState::Complete);
	OnCountdownComplete.Broadcast();
}

void UMGRaceCountdownManager::PlayTickSound(int32 Value)
{
	if (!bSoundEnabled)
	{
		return;
	}

	// Would integrate with Audio subsystem
	// Based on countdown style and current value:
	// - Classic: Beep sounds
	// - TrafficLights: Light activation sounds
	// - ChristmasTree: Stage light sounds
	// - Wangan: Minimal tone
}

void UMGRaceCountdownManager::PlayGoSound()
{
	if (!bSoundEnabled)
	{
		return;
	}

	// Would integrate with Audio subsystem
	// GO sound varies by style
}

void UMGRaceCountdownManager::SetState(EMGCountdownState NewState)
{
	CurrentState = NewState;
}
