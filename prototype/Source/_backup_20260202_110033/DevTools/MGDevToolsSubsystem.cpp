// Copyright Midnight Grind. All Rights Reserved.

#include "DevTools/MGDevToolsSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGDevToolsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterBuiltInCheats();

#if UE_BUILD_SHIPPING
	AccessLevel = EMGDevToolAccess::Disabled;
#else
	AccessLevel = EMGDevToolAccess::DevBuildOnly;
#endif

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MetricsTimerHandle,
			this,
			&UMGDevToolsSubsystem::UpdateMetrics,
			0.5f,
			true
		);
	}
}

bool UMGDevToolsSubsystem::ExecuteCommand(const FString& Command)
{
	TArray<FString> Parts;
	Command.ParseIntoArray(Parts, TEXT(" "), true);

	if (Parts.Num() == 0)
		return false;

	FString CommandName = Parts[0].ToLower();
	TArray<FString> Parameters;
	for (int32 i = 1; i < Parts.Num(); i++)
	{
		Parameters.Add(Parts[i]);
	}

	for (const FMGCheatCommand& Cheat : RegisteredCheats)
	{
		if (Cheat.Command.ToLower() == CommandName)
		{
			if (!CanExecuteCheat(Cheat))
				return false;

			// Execute based on command
			if (CommandName == TEXT("givecash") && Parameters.Num() > 0)
				GiveGrindCash(FCString::Atoi64(*Parameters[0]));
			else if (CommandName == TEXT("givecredits") && Parameters.Num() > 0)
				GiveNeonCredits(FCString::Atoi64(*Parameters[0]));
			else if (CommandName == TEXT("unlockallcars"))
				UnlockAllVehicles();
			else if (CommandName == TEXT("unlockalltracks"))
				UnlockAllTracks();
			else if (CommandName == TEXT("speed") && Parameters.Num() > 0)
				SetPlayerSpeed(FCString::Atof(*Parameters[0]));
			else if (CommandName == TEXT("aispeed") && Parameters.Num() > 0)
				SetAISpeed(FCString::Atof(*Parameters[0]));
			else if (CommandName == TEXT("winrace"))
				WinRace();
			else if (CommandName == TEXT("timescale") && Parameters.Num() > 0)
				SetTimeScale(FCString::Atof(*Parameters[0]));

			OnCheatExecuted.Broadcast(Cheat.CommandID, Parameters);
			return true;
		}
	}

	return false;
}

void UMGDevToolsSubsystem::RegisterCheat(const FMGCheatCommand& Cheat)
{
	RegisteredCheats.Add(Cheat);
}

TArray<FMGCheatCommand> UMGDevToolsSubsystem::GetAvailableCheats() const
{
	TArray<FMGCheatCommand> Available;
	for (const FMGCheatCommand& Cheat : RegisteredCheats)
	{
		if (CanExecuteCheat(Cheat))
			Available.Add(Cheat);
	}
	return Available;
}

bool UMGDevToolsSubsystem::IsDevBuild() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return true;
#endif
}

void UMGDevToolsSubsystem::GiveGrindCash(int64 Amount)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGCurrencySubsystem* Currency = GI->GetSubsystem<UMGCurrencySubsystem>())
		{
			Currency->EarnCurrency(EMGCurrencyType::GrindCash, Amount, EMGEarnSource::FirstTimeBonus, TEXT("Dev cheat"));
		}
	}
}

void UMGDevToolsSubsystem::GiveNeonCredits(int64 Amount)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGCurrencySubsystem* Currency = GI->GetSubsystem<UMGCurrencySubsystem>())
		{
			Currency->EarnCurrency(EMGCurrencyType::NeonCredits, Amount, EMGEarnSource::FirstTimeBonus, TEXT("Dev cheat"));
		}
	}
}

void UMGDevToolsSubsystem::UnlockAllVehicles()
{
	// Would unlock all vehicles
}

void UMGDevToolsSubsystem::UnlockAllTracks()
{
	// Would unlock all tracks
}

void UMGDevToolsSubsystem::SetPlayerSpeed(float Multiplier)
{
	// Would set player vehicle speed multiplier
}

void UMGDevToolsSubsystem::SetAISpeed(float Multiplier)
{
	// Would set AI vehicle speed multiplier
}

void UMGDevToolsSubsystem::TeleportToCheckpoint(int32 CheckpointIndex)
{
	// Would teleport player to specified checkpoint
}

void UMGDevToolsSubsystem::WinRace()
{
	// Would trigger race win
}

void UMGDevToolsSubsystem::SkipToPosition(int32 Position)
{
	// Would skip player to specified position
}

void UMGDevToolsSubsystem::StartProfiling(const FString& ProfileName)
{
	bIsProfiling = true;
	// Would start CPU/GPU profiling
}

void UMGDevToolsSubsystem::StopProfiling()
{
	bIsProfiling = false;
	// Would stop profiling and save results
}

void UMGDevToolsSubsystem::CaptureFrame()
{
	// Would capture frame for GPU profiler
}

void UMGDevToolsSubsystem::SetVisualization(const FMGDebugVisualization& Settings)
{
	DebugVis = Settings;
	// Would update debug rendering
}

void UMGDevToolsSubsystem::ToggleFPSDisplay()
{
	DebugVis.bShowFPS = !DebugVis.bShowFPS;
}

void UMGDevToolsSubsystem::ToggleNetStats()
{
	DebugVis.bShowNetStats = !DebugVis.bShowNetStats;
}

void UMGDevToolsSubsystem::SetTimeScale(float Scale)
{
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(FMath::Clamp(Scale, 0.1f, 10.0f));
	}
}

void UMGDevToolsSubsystem::PauseGame()
{
	SetTimeScale(0.0f);
}

void UMGDevToolsSubsystem::StepFrame()
{
	// Would advance one frame while paused
}

void UMGDevToolsSubsystem::SetAccessLevel(EMGDevToolAccess Access)
{
	AccessLevel = Access;
}

void UMGDevToolsSubsystem::RegisterBuiltInCheats()
{
	FMGCheatCommand GiveCash;
	GiveCash.CommandID = FName(TEXT("GiveCash"));
	GiveCash.Command = TEXT("givecash");
	GiveCash.Description = FText::FromString(TEXT("Give Grind Cash"));
	GiveCash.Parameters.Add(TEXT("amount"));
	RegisteredCheats.Add(GiveCash);

	FMGCheatCommand GiveCredits;
	GiveCredits.CommandID = FName(TEXT("GiveCredits"));
	GiveCredits.Command = TEXT("givecredits");
	GiveCredits.Description = FText::FromString(TEXT("Give Neon Credits"));
	GiveCredits.Parameters.Add(TEXT("amount"));
	RegisteredCheats.Add(GiveCredits);

	FMGCheatCommand UnlockCars;
	UnlockCars.CommandID = FName(TEXT("UnlockAllCars"));
	UnlockCars.Command = TEXT("unlockallcars");
	UnlockCars.Description = FText::FromString(TEXT("Unlock all vehicles"));
	RegisteredCheats.Add(UnlockCars);

	FMGCheatCommand UnlockTracks;
	UnlockTracks.CommandID = FName(TEXT("UnlockAllTracks"));
	UnlockTracks.Command = TEXT("unlockalltracks");
	UnlockTracks.Description = FText::FromString(TEXT("Unlock all tracks"));
	RegisteredCheats.Add(UnlockTracks);

	FMGCheatCommand Speed;
	Speed.CommandID = FName(TEXT("Speed"));
	Speed.Command = TEXT("speed");
	Speed.Description = FText::FromString(TEXT("Set player speed multiplier"));
	Speed.Parameters.Add(TEXT("multiplier"));
	RegisteredCheats.Add(Speed);

	FMGCheatCommand Win;
	Win.CommandID = FName(TEXT("WinRace"));
	Win.Command = TEXT("winrace");
	Win.Description = FText::FromString(TEXT("Instantly win current race"));
	RegisteredCheats.Add(Win);

	FMGCheatCommand TimeScale;
	TimeScale.CommandID = FName(TEXT("TimeScale"));
	TimeScale.Command = TEXT("timescale");
	TimeScale.Description = FText::FromString(TEXT("Set game time scale"));
	TimeScale.Parameters.Add(TEXT("scale"));
	RegisteredCheats.Add(TimeScale);
}

void UMGDevToolsSubsystem::UpdateMetrics()
{
	// Update performance metrics
	CurrentMetrics.FPS = 1.0f / FApp::GetDeltaTime();
	CurrentMetrics.FrameTimeMS = FApp::GetDeltaTime() * 1000.0f;

	// Would query actual thread times and GPU times
}

bool UMGDevToolsSubsystem::CanExecuteCheat(const FMGCheatCommand& Cheat) const
{
	if (AccessLevel == EMGDevToolAccess::Disabled)
		return false;

	if (AccessLevel == EMGDevToolAccess::DevBuildOnly && Cheat.bRequiresDevBuild)
		return IsDevBuild();

	return true;
}
