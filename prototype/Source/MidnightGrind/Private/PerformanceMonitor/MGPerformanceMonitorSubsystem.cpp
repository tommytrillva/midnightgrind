// Copyright Midnight Grind. All Rights Reserved.

#include "PerformanceMonitor/MGPerformanceMonitorSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "RenderCore.h"
#include "RHI.h"

void UMGPerformanceMonitorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize thresholds with default values
	Thresholds.TargetFPS = 60.0f;
	Thresholds.MinAcceptableFPS = 30.0f;
	Thresholds.MaxFrameTimeMs = 33.33f;
	Thresholds.MaxMemoryUsagePercent = 85.0f;
	Thresholds.MaxGPUTimeMs = 16.0f;
	Thresholds.MaxPingMs = 100.0f;
	Thresholds.MaxPacketLossPercent = 2.0f;
	Thresholds.MaxDrawCalls = 5000;

	// Initialize dynamic quality settings
	DynamicQualitySettings.bEnabled = true;
	DynamicQualitySettings.TargetFrameRate = 60.0f;
	DynamicQualitySettings.TolerancePercent = 10.0f;
	DynamicQualitySettings.AdjustmentInterval = 2.0f;
	DynamicQualitySettings.bAllowResolutionScaling = true;
	DynamicQualitySettings.MinResolutionScale = 0.7f;
	DynamicQualitySettings.MaxResolutionScale = 1.0f;

	// Initialize frame time buffer
	FrameTimeBuffer.SetNum(FrameTimeBufferSize);
	for (int32 i = 0; i < FrameTimeBufferSize; ++i)
	{
		FrameTimeBuffer[i] = 16.67f;
	}

	CurrentResolutionScale = 1.0f;
	CurrentQualityPreset = EMGQualityPreset::High;

	StartMonitoring();
}

void UMGPerformanceMonitorSubsystem::Deinitialize()
{
	StopMonitoring();
	StopBenchmark();
	Super::Deinitialize();
}

bool UMGPerformanceMonitorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FMGPerformanceSnapshot UMGPerformanceMonitorSubsystem::GetAverageSnapshot(float DurationSeconds) const
{
	FMGPerformanceSnapshot Average;

	if (PerformanceHistory.Num() == 0)
	{
		return CurrentSnapshot;
	}

	int32 SamplesToAverage = FMath::Min(PerformanceHistory.Num(), FMath::CeilToInt(DurationSeconds / SampleInterval));

	float TotalFPS = 0.0f;
	float TotalFrameTime = 0.0f;
	float TotalGPUTime = 0.0f;
	float TotalCPUTime = 0.0f;
	int64 TotalMemory = 0;

	int32 StartIndex = FMath::Max(0, PerformanceHistory.Num() - SamplesToAverage);
	int32 Count = 0;

	for (int32 i = StartIndex; i < PerformanceHistory.Num(); ++i)
	{
		const FMGPerformanceSnapshot& Snapshot = PerformanceHistory[i];
		TotalFPS += Snapshot.FrameStats.CurrentFPS;
		TotalFrameTime += Snapshot.FrameStats.FrameTimeMs;
		TotalGPUTime += Snapshot.GPUStats.GPUTimeMs;
		TotalCPUTime += Snapshot.CPUStats.GameThreadTimeMs;
		TotalMemory += Snapshot.MemoryStats.UsedPhysicalMemoryMB;
		Count++;
	}

	if (Count > 0)
	{
		Average.FrameStats.AverageFPS = TotalFPS / Count;
		Average.FrameStats.FrameTimeMs = TotalFrameTime / Count;
		Average.GPUStats.GPUTimeMs = TotalGPUTime / Count;
		Average.CPUStats.GameThreadTimeMs = TotalCPUTime / Count;
		Average.MemoryStats.UsedPhysicalMemoryMB = TotalMemory / Count;
	}

	return Average;
}

void UMGPerformanceMonitorSubsystem::ClearHistory()
{
	PerformanceHistory.Empty();
}

void UMGPerformanceMonitorSubsystem::ClearAlerts()
{
	ActiveAlerts.Empty();
}

void UMGPerformanceMonitorSubsystem::SetThresholds(const FMGPerformanceThresholds& NewThresholds)
{
	Thresholds = NewThresholds;
}

void UMGPerformanceMonitorSubsystem::ApplyQualityPreset(EMGQualityPreset Preset)
{
	switch (Preset)
	{
	case EMGQualityPreset::Low:
		ApplyLowQualitySettings();
		break;
	case EMGQualityPreset::Medium:
		ApplyMediumQualitySettings();
		break;
	case EMGQualityPreset::High:
		ApplyHighQualitySettings();
		break;
	case EMGQualityPreset::Ultra:
		ApplyUltraQualitySettings();
		break;
	case EMGQualityPreset::Auto:
		SetDynamicQualityEnabled(true);
		break;
	default:
		break;
	}

	CurrentQualityPreset = Preset;
	OnQualitySettingsChanged.Broadcast(Preset);
}

void UMGPerformanceMonitorSubsystem::SetResolutionScale(float Scale)
{
	float OldScale = CurrentResolutionScale;
	CurrentResolutionScale = FMath::Clamp(Scale, 0.5f, 2.0f);

	if (GEngine)
	{
		static IConsoleVariable* CVarScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
		if (CVarScreenPercentage)
		{
			CVarScreenPercentage->Set(CurrentResolutionScale * 100.0f);
		}
	}

	if (!FMath::IsNearlyEqual(OldScale, CurrentResolutionScale))
	{
		OnDynamicResolutionChanged.Broadcast(CurrentResolutionScale);
	}
}

void UMGPerformanceMonitorSubsystem::SetDynamicQualityEnabled(bool bEnabled)
{
	DynamicQualitySettings.bEnabled = bEnabled;

	if (UWorld* World = GetWorld())
	{
		if (bEnabled)
		{
			World->GetTimerManager().SetTimer(
				DynamicQualityTimerHandle,
				this,
				&UMGPerformanceMonitorSubsystem::ProcessDynamicQuality,
				DynamicQualitySettings.AdjustmentInterval,
				true
			);
		}
		else
		{
			World->GetTimerManager().ClearTimer(DynamicQualityTimerHandle);
		}
	}
}

void UMGPerformanceMonitorSubsystem::SetDynamicQualitySettings(const FMGDynamicQualitySettings& Settings)
{
	DynamicQualitySettings = Settings;

	if (DynamicQualitySettings.bEnabled)
	{
		SetDynamicQualityEnabled(true);
	}
}

void UMGPerformanceMonitorSubsystem::StartMonitoring()
{
	if (bIsMonitoring)
	{
		return;
	}

	bIsMonitoring = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MonitoringTimerHandle,
			this,
			&UMGPerformanceMonitorSubsystem::CollectStats,
			SampleInterval,
			true
		);
	}
}

void UMGPerformanceMonitorSubsystem::StopMonitoring()
{
	if (!bIsMonitoring)
	{
		return;
	}

	bIsMonitoring = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
	}
}

void UMGPerformanceMonitorSubsystem::SetSampleRate(float SamplesPerSecond)
{
	SampleInterval = 1.0f / FMath::Max(1.0f, SamplesPerSecond);

	if (bIsMonitoring)
	{
		StopMonitoring();
		StartMonitoring();
	}
}

void UMGPerformanceMonitorSubsystem::StartBenchmark(float DurationSeconds)
{
	if (bIsBenchmarking)
	{
		return;
	}

	bIsBenchmarking = true;
	BenchmarkDuration = DurationSeconds;
	BenchmarkElapsed = 0.0f;
	BenchmarkResults = FMGPerformanceSnapshot();

	ClearHistory();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BenchmarkTimerHandle,
			[this]()
			{
				BenchmarkElapsed += 0.1f;
				if (BenchmarkElapsed >= BenchmarkDuration)
				{
					StopBenchmark();
				}
			},
			0.1f,
			true
		);
	}
}

void UMGPerformanceMonitorSubsystem::StopBenchmark()
{
	if (!bIsBenchmarking)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BenchmarkTimerHandle);
	}

	BenchmarkResults = GetAverageSnapshot(BenchmarkDuration);
	bIsBenchmarking = false;
}

void UMGPerformanceMonitorSubsystem::BeginProfileScope(FName ScopeName)
{
	ProfileScopes.Add(ScopeName, FPlatformTime::Seconds());
}

void UMGPerformanceMonitorSubsystem::EndProfileScope(FName ScopeName)
{
	if (double* StartTime = ProfileScopes.Find(ScopeName))
	{
		double EndTime = FPlatformTime::Seconds();
		double Duration = (EndTime - *StartTime) * 1000.0;
		ProfileScopeTimes.Add(ScopeName, Duration);
		ProfileScopes.Remove(ScopeName);
	}
}

float UMGPerformanceMonitorSubsystem::GetProfileScopeTime(FName ScopeName) const
{
	if (const double* Time = ProfileScopeTimes.Find(ScopeName))
	{
		return static_cast<float>(*Time);
	}
	return 0.0f;
}

void UMGPerformanceMonitorSubsystem::CollectStats()
{
	CurrentSnapshot.Timestamp = FDateTime::UtcNow();

	UpdateFrameStats();
	UpdateMemoryStats();
	UpdateGPUStats();
	UpdateCPUStats();
	UpdateNetworkStats();

	EvaluatePerformanceLevel();
	CheckThresholds();

	// Store in history
	PerformanceHistory.Add(CurrentSnapshot);
	if (PerformanceHistory.Num() > MaxHistorySize)
	{
		PerformanceHistory.RemoveAt(0);
	}
}

void UMGPerformanceMonitorSubsystem::UpdateFrameStats()
{
	float DeltaTime = FApp::GetDeltaTime();
	float CurrentFrameTime = DeltaTime * 1000.0f;

	FrameTimeBuffer[FrameTimeBufferIndex] = CurrentFrameTime;
	FrameTimeBufferIndex = (FrameTimeBufferIndex + 1) % FrameTimeBufferSize;

	CurrentSnapshot.FrameStats.FrameTimeMs = CurrentFrameTime;
	CurrentSnapshot.FrameStats.CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;

	// Calculate average, min, max
	float Sum = 0.0f;
	float Min = FrameTimeBuffer[0];
	float Max = FrameTimeBuffer[0];

	for (int32 i = 0; i < FrameTimeBufferSize; ++i)
	{
		Sum += FrameTimeBuffer[i];
		Min = FMath::Min(Min, FrameTimeBuffer[i]);
		Max = FMath::Max(Max, FrameTimeBuffer[i]);
	}

	float AverageFrameTime = Sum / FrameTimeBufferSize;
	CurrentSnapshot.FrameStats.AverageFPS = 1000.0f / AverageFrameTime;
	CurrentSnapshot.FrameStats.MinFPS = 1000.0f / Max;
	CurrentSnapshot.FrameStats.MaxFPS = 1000.0f / Min;

	// Calculate 1% and 0.1% lows
	TArray<float> SortedFrameTimes = FrameTimeBuffer;
	SortedFrameTimes.Sort([](float A, float B) { return A > B; });

	int32 OnePercentIndex = FMath::Max(0, FrameTimeBufferSize / 100);
	int32 PointOnePercentIndex = FMath::Max(0, FrameTimeBufferSize / 1000);

	CurrentSnapshot.FrameStats.OnePercentLow = 1000.0f / SortedFrameTimes[OnePercentIndex];
	CurrentSnapshot.FrameStats.PointOnePercentLow = 1000.0f / SortedFrameTimes[PointOnePercentIndex];

	// Calculate variance
	float VarianceSum = 0.0f;
	for (int32 i = 0; i < FrameTimeBufferSize; ++i)
	{
		float Diff = FrameTimeBuffer[i] - AverageFrameTime;
		VarianceSum += Diff * Diff;
	}
	CurrentSnapshot.FrameStats.FrameTimeVariance = FMath::Sqrt(VarianceSum / FrameTimeBufferSize);

	// Count frame drops (frames taking >50% longer than target)
	float TargetFrameTime = 1000.0f / Thresholds.TargetFPS;
	static int32 FrameDropCounter = 0;
	if (CurrentFrameTime > TargetFrameTime * 1.5f)
	{
		FrameDropCounter++;
	}
	CurrentSnapshot.FrameStats.FrameDropCount = FrameDropCounter;
}

void UMGPerformanceMonitorSubsystem::UpdateMemoryStats()
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	CurrentSnapshot.MemoryStats.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024 * 1024);
	CurrentSnapshot.MemoryStats.AvailablePhysicalMemoryMB = MemStats.AvailablePhysical / (1024 * 1024);
	CurrentSnapshot.MemoryStats.TotalPhysicalMemoryMB = (MemStats.UsedPhysical + MemStats.AvailablePhysical) / (1024 * 1024);
	CurrentSnapshot.MemoryStats.UsedVirtualMemoryMB = MemStats.UsedVirtual / (1024 * 1024);

	if (CurrentSnapshot.MemoryStats.TotalPhysicalMemoryMB > 0)
	{
		CurrentSnapshot.MemoryStats.MemoryUsagePercent =
			(static_cast<float>(CurrentSnapshot.MemoryStats.UsedPhysicalMemoryMB) /
			 static_cast<float>(CurrentSnapshot.MemoryStats.TotalPhysicalMemoryMB)) * 100.0f;
	}

	CurrentSnapshot.MemoryStats.bMemoryPressure =
		CurrentSnapshot.MemoryStats.MemoryUsagePercent > Thresholds.MaxMemoryUsagePercent;
}

void UMGPerformanceMonitorSubsystem::UpdateGPUStats()
{
	// GPU time from render thread
	if (GEngine && GEngine->GameViewport)
	{
		// Draw calls and triangles from rendering stats
		CurrentSnapshot.GPUStats.DrawCalls = GNumDrawCallsRHI;
		CurrentSnapshot.GPUStats.Triangles = GNumPrimitivesDrawnRHI;
	}

	// VRAM stats
	FTextureMemoryStats TextureStats;
	RHIGetTextureMemoryStats(TextureStats);

	if (TextureStats.DedicatedVideoMemory > 0)
	{
		CurrentSnapshot.GPUStats.VRAMUsedMB = (TextureStats.DedicatedVideoMemory - TextureStats.StreamingPool) / (1024 * 1024);
		CurrentSnapshot.GPUStats.VRAMAvailableMB = TextureStats.StreamingPool / (1024 * 1024);
	}
}

void UMGPerformanceMonitorSubsystem::UpdateCPUStats()
{
	if (GEngine)
	{
		// Get game thread time
		extern ENGINE_API float GAverageFPS;
		CurrentSnapshot.CPUStats.GameThreadTimeMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
	}
}

void UMGPerformanceMonitorSubsystem::UpdateNetworkStats()
{
	// Network stats would be populated from the networking subsystem
	// Placeholder implementation
	CurrentSnapshot.NetworkStats.PingMs = 0.0f;
	CurrentSnapshot.NetworkStats.PacketLossPercent = 0.0f;
}

void UMGPerformanceMonitorSubsystem::EvaluatePerformanceLevel()
{
	EMGPerformanceLevel OldLevel = CurrentSnapshot.OverallLevel;

	float FPS = CurrentSnapshot.FrameStats.AverageFPS;
	float MemUsage = CurrentSnapshot.MemoryStats.MemoryUsagePercent;

	if (FPS < 20.0f || MemUsage > 95.0f)
	{
		CurrentSnapshot.OverallLevel = EMGPerformanceLevel::Critical;
	}
	else if (FPS < 30.0f || MemUsage > 90.0f)
	{
		CurrentSnapshot.OverallLevel = EMGPerformanceLevel::Low;
	}
	else if (FPS < 45.0f || MemUsage > 80.0f)
	{
		CurrentSnapshot.OverallLevel = EMGPerformanceLevel::Medium;
	}
	else if (FPS < 55.0f)
	{
		CurrentSnapshot.OverallLevel = EMGPerformanceLevel::High;
	}
	else
	{
		CurrentSnapshot.OverallLevel = EMGPerformanceLevel::Excellent;
	}

	if (OldLevel != CurrentSnapshot.OverallLevel)
	{
		OnPerformanceLevelChanged.Broadcast(OldLevel, CurrentSnapshot.OverallLevel);
	}
}

void UMGPerformanceMonitorSubsystem::CheckThresholds()
{
	// Check FPS threshold
	if (CurrentSnapshot.FrameStats.AverageFPS < Thresholds.MinAcceptableFPS)
	{
		AddAlert(EMGPerformanceCategory::FrameRate,
			FString::Printf(TEXT("FPS dropped to %.1f (below minimum %.1f)"),
				CurrentSnapshot.FrameStats.AverageFPS, Thresholds.MinAcceptableFPS),
			EMGPerformanceLevel::Critical);
	}
	else if (CurrentSnapshot.FrameStats.AverageFPS < Thresholds.TargetFPS)
	{
		AddAlert(EMGPerformanceCategory::FrameRate,
			FString::Printf(TEXT("FPS at %.1f (below target %.1f)"),
				CurrentSnapshot.FrameStats.AverageFPS, Thresholds.TargetFPS),
			EMGPerformanceLevel::Low);
	}

	// Check memory threshold
	if (CurrentSnapshot.MemoryStats.bMemoryPressure)
	{
		AddAlert(EMGPerformanceCategory::Memory,
			FString::Printf(TEXT("Memory usage at %.1f%% (above threshold %.1f%%)"),
				CurrentSnapshot.MemoryStats.MemoryUsagePercent, Thresholds.MaxMemoryUsagePercent),
			EMGPerformanceLevel::Low);
	}

	// Check draw calls
	if (CurrentSnapshot.GPUStats.DrawCalls > Thresholds.MaxDrawCalls)
	{
		AddAlert(EMGPerformanceCategory::GPU,
			FString::Printf(TEXT("Draw calls at %d (above threshold %d)"),
				CurrentSnapshot.GPUStats.DrawCalls, Thresholds.MaxDrawCalls),
			EMGPerformanceLevel::Medium);
	}

	// Check network
	if (CurrentSnapshot.NetworkStats.PingMs > Thresholds.MaxPingMs)
	{
		AddAlert(EMGPerformanceCategory::Network,
			FString::Printf(TEXT("Ping at %.0fms (above threshold %.0fms)"),
				CurrentSnapshot.NetworkStats.PingMs, Thresholds.MaxPingMs),
			EMGPerformanceLevel::Medium);
	}

	if (CurrentSnapshot.NetworkStats.PacketLossPercent > Thresholds.MaxPacketLossPercent)
	{
		AddAlert(EMGPerformanceCategory::Network,
			FString::Printf(TEXT("Packet loss at %.1f%% (above threshold %.1f%%)"),
				CurrentSnapshot.NetworkStats.PacketLossPercent, Thresholds.MaxPacketLossPercent),
			EMGPerformanceLevel::Low);
	}
}

void UMGPerformanceMonitorSubsystem::ProcessDynamicQuality()
{
	if (!DynamicQualitySettings.bEnabled)
	{
		return;
	}

	float CurrentFPS = CurrentSnapshot.FrameStats.AverageFPS;
	float TargetFPS = DynamicQualitySettings.TargetFrameRate;
	float Tolerance = TargetFPS * (DynamicQualitySettings.TolerancePercent / 100.0f);

	if (CurrentFPS < TargetFPS - Tolerance)
	{
		// Performance too low, reduce quality
		if (DynamicQualitySettings.bAllowResolutionScaling && CurrentResolutionScale > DynamicQualitySettings.MinResolutionScale)
		{
			float NewScale = FMath::Max(DynamicQualitySettings.MinResolutionScale, CurrentResolutionScale - 0.05f);
			SetResolutionScale(NewScale);
		}
	}
	else if (CurrentFPS > TargetFPS + Tolerance)
	{
		// Performance has headroom, increase quality
		if (DynamicQualitySettings.bAllowResolutionScaling && CurrentResolutionScale < DynamicQualitySettings.MaxResolutionScale)
		{
			float NewScale = FMath::Min(DynamicQualitySettings.MaxResolutionScale, CurrentResolutionScale + 0.02f);
			SetResolutionScale(NewScale);
		}
	}
}

void UMGPerformanceMonitorSubsystem::AddAlert(EMGPerformanceCategory Category, const FString& Message, EMGPerformanceLevel Severity)
{
	// Check if similar alert already exists
	for (const FMGPerformanceAlert& Existing : ActiveAlerts)
	{
		if (Existing.Category == Category && Existing.Severity == Severity)
		{
			return; // Don't spam duplicate alerts
		}
	}

	FMGPerformanceAlert Alert;
	Alert.Category = Category;
	Alert.AlertMessage = FText::FromString(Message);
	Alert.Severity = Severity;
	Alert.Timestamp = FDateTime::UtcNow();

	// Add suggested action based on category
	switch (Category)
	{
	case EMGPerformanceCategory::FrameRate:
		Alert.SuggestedAction = TEXT("Consider lowering graphics settings or resolution");
		break;
	case EMGPerformanceCategory::Memory:
		Alert.SuggestedAction = TEXT("Close background applications or lower texture quality");
		break;
	case EMGPerformanceCategory::GPU:
		Alert.SuggestedAction = TEXT("Reduce shadow quality or view distance");
		break;
	case EMGPerformanceCategory::Network:
		Alert.SuggestedAction = TEXT("Check network connection or find a closer server");
		break;
	default:
		Alert.SuggestedAction = TEXT("Enable dynamic quality settings for automatic optimization");
		break;
	}

	ActiveAlerts.Add(Alert);
	OnPerformanceAlert.Broadcast(Alert);

	// Keep alert list manageable
	if (ActiveAlerts.Num() > 20)
	{
		ActiveAlerts.RemoveAt(0);
	}
}

void UMGPerformanceMonitorSubsystem::ApplyLowQualitySettings()
{
	if (GEngine)
	{
		GEngine->Exec(nullptr, TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(nullptr, TEXT("sg.TextureQuality 0"));
		GEngine->Exec(nullptr, TEXT("sg.EffectsQuality 0"));
		GEngine->Exec(nullptr, TEXT("sg.FoliageQuality 0"));
		GEngine->Exec(nullptr, TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(nullptr, TEXT("sg.ViewDistanceQuality 0"));
	}
	SetResolutionScale(0.75f);
}

void UMGPerformanceMonitorSubsystem::ApplyMediumQualitySettings()
{
	if (GEngine)
	{
		GEngine->Exec(nullptr, TEXT("sg.ShadowQuality 1"));
		GEngine->Exec(nullptr, TEXT("sg.TextureQuality 1"));
		GEngine->Exec(nullptr, TEXT("sg.EffectsQuality 1"));
		GEngine->Exec(nullptr, TEXT("sg.FoliageQuality 1"));
		GEngine->Exec(nullptr, TEXT("sg.PostProcessQuality 1"));
		GEngine->Exec(nullptr, TEXT("sg.ViewDistanceQuality 1"));
	}
	SetResolutionScale(0.85f);
}

void UMGPerformanceMonitorSubsystem::ApplyHighQualitySettings()
{
	if (GEngine)
	{
		GEngine->Exec(nullptr, TEXT("sg.ShadowQuality 2"));
		GEngine->Exec(nullptr, TEXT("sg.TextureQuality 2"));
		GEngine->Exec(nullptr, TEXT("sg.EffectsQuality 2"));
		GEngine->Exec(nullptr, TEXT("sg.FoliageQuality 2"));
		GEngine->Exec(nullptr, TEXT("sg.PostProcessQuality 2"));
		GEngine->Exec(nullptr, TEXT("sg.ViewDistanceQuality 2"));
	}
	SetResolutionScale(1.0f);
}

void UMGPerformanceMonitorSubsystem::ApplyUltraQualitySettings()
{
	if (GEngine)
	{
		GEngine->Exec(nullptr, TEXT("sg.ShadowQuality 3"));
		GEngine->Exec(nullptr, TEXT("sg.TextureQuality 3"));
		GEngine->Exec(nullptr, TEXT("sg.EffectsQuality 3"));
		GEngine->Exec(nullptr, TEXT("sg.FoliageQuality 3"));
		GEngine->Exec(nullptr, TEXT("sg.PostProcessQuality 3"));
		GEngine->Exec(nullptr, TEXT("sg.ViewDistanceQuality 3"));
	}
	SetResolutionScale(1.0f);
}
