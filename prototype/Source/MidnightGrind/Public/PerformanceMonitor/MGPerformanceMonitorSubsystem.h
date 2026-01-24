// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPerformanceMonitorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGPerformanceLevel : uint8
{
	Critical,
	Low,
	Medium,
	High,
	Excellent
};

UENUM(BlueprintType)
enum class EMGPerformanceCategory : uint8
{
	FrameRate,
	Memory,
	GPU,
	CPU,
	Network,
	Streaming,
	Physics,
	Audio
};

UENUM(BlueprintType)
enum class EMGQualityPreset : uint8
{
	Low,
	Medium,
	High,
	Ultra,
	Custom,
	Auto
};

USTRUCT(BlueprintType)
struct FMGFrameTimeStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrameTimeMs = 16.67f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrameTimeVariance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrameDropCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OnePercentLow = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointOnePercentLow = 60.0f;
};

USTRUCT(BlueprintType)
struct FMGMemoryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 UsedPhysicalMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AvailablePhysicalMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalPhysicalMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 UsedVirtualMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TextureMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MeshMemoryMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MemoryUsagePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMemoryPressure = false;
};

USTRUCT(BlueprintType)
struct FMGGPUStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GPUTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GPUUtilization = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 VRAMUsedMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 VRAMAvailableMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DrawCalls = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Triangles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RenderThreadTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RHIThreadTimeMs = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGCPUStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameThreadTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CPUUtilization = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveThreads = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicsTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AITimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimationTimeMs = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGNetworkStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PacketLossPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncomingBandwidthKBps = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutgoingBandwidthKBps = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsSent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsLost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Jitter = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGPerformanceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGFrameTimeStats FrameStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMemoryStats MemoryStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGPUStats GPUStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCPUStats CPUStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGNetworkStats NetworkStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceLevel OverallLevel = EMGPerformanceLevel::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentMapName;
};

USTRUCT(BlueprintType)
struct FMGPerformanceAlert
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceCategory Category = EMGPerformanceCategory::FrameRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText AlertMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceLevel Severity = EMGPerformanceLevel::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SuggestedAction;
};

USTRUCT(BlueprintType)
struct FMGPerformanceThresholds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAcceptableFPS = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFrameTimeMs = 33.33f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMemoryUsagePercent = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxGPUTimeMs = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPingMs = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPacketLossPercent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDrawCalls = 5000;
};

USTRUCT(BlueprintType)
struct FMGDynamicQualitySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFrameRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TolerancePercent = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AdjustmentInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowResolutionScaling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinResolutionScale = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxResolutionScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowShadowQualityAdjustment = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowEffectsQualityAdjustment = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowFoliageDensityAdjustment = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceAlert, const FMGPerformanceAlert&, Alert);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceLevelChanged, EMGPerformanceLevel, OldLevel, EMGPerformanceLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualitySettingsChanged, EMGQualityPreset, NewPreset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicResolutionChanged, float, NewScale);

UCLASS()
class MIDNIGHTGRIND_API UMGPerformanceMonitorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Real-time Stats
	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGFrameTimeStats GetFrameStats() const { return CurrentSnapshot.FrameStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGMemoryStats GetMemoryStats() const { return CurrentSnapshot.MemoryStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGGPUStats GetGPUStats() const { return CurrentSnapshot.GPUStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGCPUStats GetCPUStats() const { return CurrentSnapshot.CPUStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGNetworkStats GetNetworkStats() const { return CurrentSnapshot.NetworkStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGPerformanceSnapshot GetCurrentSnapshot() const { return CurrentSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	EMGPerformanceLevel GetOverallPerformanceLevel() const { return CurrentSnapshot.OverallLevel; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	float GetCurrentFPS() const { return CurrentSnapshot.FrameStats.CurrentFPS; }

	// Performance History
	UFUNCTION(BlueprintPure, Category = "Performance|History")
	TArray<FMGPerformanceSnapshot> GetPerformanceHistory() const { return PerformanceHistory; }

	UFUNCTION(BlueprintPure, Category = "Performance|History")
	FMGPerformanceSnapshot GetAverageSnapshot(float DurationSeconds = 60.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Performance|History")
	void ClearHistory();

	// Alerts
	UFUNCTION(BlueprintPure, Category = "Performance|Alerts")
	TArray<FMGPerformanceAlert> GetActiveAlerts() const { return ActiveAlerts; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Alerts")
	void ClearAlerts();

	UFUNCTION(BlueprintCallable, Category = "Performance|Alerts")
	void SetThresholds(const FMGPerformanceThresholds& NewThresholds);

	UFUNCTION(BlueprintPure, Category = "Performance|Alerts")
	FMGPerformanceThresholds GetThresholds() const { return Thresholds; }

	// Quality Settings
	UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
	void ApplyQualityPreset(EMGQualityPreset Preset);

	UFUNCTION(BlueprintPure, Category = "Performance|Quality")
	EMGQualityPreset GetCurrentQualityPreset() const { return CurrentQualityPreset; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
	void SetResolutionScale(float Scale);

	UFUNCTION(BlueprintPure, Category = "Performance|Quality")
	float GetResolutionScale() const { return CurrentResolutionScale; }

	// Dynamic Quality
	UFUNCTION(BlueprintCallable, Category = "Performance|Dynamic")
	void SetDynamicQualityEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Performance|Dynamic")
	bool IsDynamicQualityEnabled() const { return DynamicQualitySettings.bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Dynamic")
	void SetDynamicQualitySettings(const FMGDynamicQualitySettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Performance|Dynamic")
	FMGDynamicQualitySettings GetDynamicQualitySettings() const { return DynamicQualitySettings; }

	// Monitoring Control
	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void StartMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void StopMonitoring();

	UFUNCTION(BlueprintPure, Category = "Performance|Control")
	bool IsMonitoring() const { return bIsMonitoring; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void SetSampleRate(float SamplesPerSecond);

	// Benchmarking
	UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
	void StartBenchmark(float DurationSeconds = 60.0f);

	UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
	void StopBenchmark();

	UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
	bool IsBenchmarking() const { return bIsBenchmarking; }

	UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
	FMGPerformanceSnapshot GetBenchmarkResults() const { return BenchmarkResults; }

	// Profiling
	UFUNCTION(BlueprintCallable, Category = "Performance|Profile")
	void BeginProfileScope(FName ScopeName);

	UFUNCTION(BlueprintCallable, Category = "Performance|Profile")
	void EndProfileScope(FName ScopeName);

	UFUNCTION(BlueprintPure, Category = "Performance|Profile")
	float GetProfileScopeTime(FName ScopeName) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnPerformanceAlert OnPerformanceAlert;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnPerformanceLevelChanged OnPerformanceLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnQualitySettingsChanged OnQualitySettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnDynamicResolutionChanged OnDynamicResolutionChanged;

protected:
	void CollectStats();
	void UpdateFrameStats();
	void UpdateMemoryStats();
	void UpdateGPUStats();
	void UpdateCPUStats();
	void UpdateNetworkStats();
	void EvaluatePerformanceLevel();
	void CheckThresholds();
	void ProcessDynamicQuality();
	void AddAlert(EMGPerformanceCategory Category, const FString& Message, EMGPerformanceLevel Severity);
	void ApplyLowQualitySettings();
	void ApplyMediumQualitySettings();
	void ApplyHighQualitySettings();
	void ApplyUltraQualitySettings();

	UPROPERTY()
	FMGPerformanceSnapshot CurrentSnapshot;

	UPROPERTY()
	TArray<FMGPerformanceSnapshot> PerformanceHistory;

	UPROPERTY()
	TArray<FMGPerformanceAlert> ActiveAlerts;

	UPROPERTY()
	FMGPerformanceThresholds Thresholds;

	UPROPERTY()
	FMGDynamicQualitySettings DynamicQualitySettings;

	UPROPERTY()
	EMGQualityPreset CurrentQualityPreset = EMGQualityPreset::High;

	UPROPERTY()
	float CurrentResolutionScale = 1.0f;

	UPROPERTY()
	bool bIsMonitoring = false;

	UPROPERTY()
	bool bIsBenchmarking = false;

	UPROPERTY()
	FMGPerformanceSnapshot BenchmarkResults;

	UPROPERTY()
	float SampleInterval = 0.1f;

	UPROPERTY()
	int32 MaxHistorySize = 600;

	UPROPERTY()
	TMap<FName, double> ProfileScopes;

	UPROPERTY()
	TMap<FName, double> ProfileScopeTimes;

	TArray<float> FrameTimeBuffer;
	int32 FrameTimeBufferIndex = 0;
	static const int32 FrameTimeBufferSize = 120;

	FTimerHandle MonitoringTimerHandle;
	FTimerHandle DynamicQualityTimerHandle;
	FTimerHandle BenchmarkTimerHandle;
	float BenchmarkDuration = 0.0f;
	float BenchmarkElapsed = 0.0f;
};
