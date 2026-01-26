// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDevToolsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGDevToolAccess : uint8
{
	Disabled,
	DevBuildOnly,
	AllBuilds
};

USTRUCT(BlueprintType)
struct FMGCheatCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CommandID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Command;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDevBuild = true;
};

USTRUCT(BlueprintType)
struct FMGPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float FPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float FrameTimeMS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float GameThreadMS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RenderThreadMS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float GPUTimeMS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 DrawCalls = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 TrianglesRendered = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 MemoryUsedMB = 0;

	UPROPERTY(BlueprintReadOnly)
	float NetworkLatencyMS = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGDebugVisualization
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFPS = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowNetStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowAIDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCheckpoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpawnPoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWireframeMode = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCheatExecuted, FName, CommandID, const TArray<FString>&, Parameters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnDevConsoleToggled);

UCLASS()
class MIDNIGHTGRIND_API UMGDevToolsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Console Commands
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	bool ExecuteCommand(const FString& Command);

	UFUNCTION(BlueprintCallable, Category = "DevTools")
	void RegisterCheat(const FMGCheatCommand& Cheat);

	UFUNCTION(BlueprintPure, Category = "DevTools")
	TArray<FMGCheatCommand> GetAvailableCheats() const;

	UFUNCTION(BlueprintPure, Category = "DevTools")
	bool IsDevBuild() const;

	// Quick Cheats
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void GiveGrindCash(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void GiveNeonCredits(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void UnlockAllVehicles();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void UnlockAllTracks();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SetPlayerSpeed(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SetAISpeed(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void TeleportToCheckpoint(int32 CheckpointIndex);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void WinRace();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SkipToPosition(int32 Position);

	// Performance Monitoring
	UFUNCTION(BlueprintPure, Category = "DevTools|Performance")
	FMGPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void StartProfiling(const FString& ProfileName);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void StopProfiling();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void CaptureFrame();

	// Debug Visualization
	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void SetVisualization(const FMGDebugVisualization& Settings);

	UFUNCTION(BlueprintPure, Category = "DevTools|Debug")
	FMGDebugVisualization GetVisualization() const { return DebugVis; }

	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void ToggleFPSDisplay();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void ToggleNetStats();

	// Time Control
	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void StepFrame();

	// Access Control
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	void SetAccessLevel(EMGDevToolAccess Access);

	UFUNCTION(BlueprintPure, Category = "DevTools")
	EMGDevToolAccess GetAccessLevel() const { return AccessLevel; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "DevTools|Events")
	FMGOnCheatExecuted OnCheatExecuted;

	UPROPERTY(BlueprintAssignable, Category = "DevTools|Events")
	FMGOnDevConsoleToggled OnDevConsoleToggled;

protected:
	void RegisterBuiltInCheats();
	void UpdateMetrics();
	bool CanExecuteCheat(const FMGCheatCommand& Cheat) const;

private:
	UPROPERTY()
	TArray<FMGCheatCommand> RegisteredCheats;

	FMGPerformanceMetrics CurrentMetrics;
	FMGDebugVisualization DebugVis;
	EMGDevToolAccess AccessLevel = EMGDevToolAccess::DevBuildOnly;
	bool bIsProfiling = false;
	FTimerHandle MetricsTimerHandle;
};
