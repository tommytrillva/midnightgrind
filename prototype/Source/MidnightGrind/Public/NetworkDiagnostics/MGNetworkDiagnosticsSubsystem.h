// MGNetworkDiagnosticsSubsystem.h
// Midnight Grind - Network Diagnostics and Connection Quality System
// Provides real-time network monitoring, latency tracking, and connection health analysis

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNetworkDiagnosticsSubsystem.generated.h"

// Connection quality levels
UENUM(BlueprintType)
enum class EMGConnectionQuality : uint8
{
    Excellent               UMETA(DisplayName = "Excellent"),
    Good                    UMETA(DisplayName = "Good"),
    Fair                    UMETA(DisplayName = "Fair"),
    Poor                    UMETA(DisplayName = "Poor"),
    Critical                UMETA(DisplayName = "Critical"),
    Disconnected            UMETA(DisplayName = "Disconnected")
};

// Network issue types
UENUM(BlueprintType)
enum class EMGNetworkIssue : uint8
{
    None                    UMETA(DisplayName = "None"),
    HighLatency             UMETA(DisplayName = "High Latency"),
    PacketLoss              UMETA(DisplayName = "Packet Loss"),
    Jitter                  UMETA(DisplayName = "Jitter"),
    Bandwidth               UMETA(DisplayName = "Bandwidth"),
    Timeout                 UMETA(DisplayName = "Timeout"),
    ServerUnreachable       UMETA(DisplayName = "Server Unreachable"),
    NATIssue                UMETA(DisplayName = "NAT Issue"),
    DNSFailure              UMETA(DisplayName = "DNS Failure"),
    AuthenticationError     UMETA(DisplayName = "Authentication Error")
};

// Network region identifiers
UENUM(BlueprintType)
enum class EMGNetworkRegion : uint8
{
    Auto                    UMETA(DisplayName = "Auto Select"),
    NAEast                  UMETA(DisplayName = "North America East"),
    NAWest                  UMETA(DisplayName = "North America West"),
    SouthAmerica            UMETA(DisplayName = "South America"),
    EuropeWest              UMETA(DisplayName = "Europe West"),
    EuropeNorth             UMETA(DisplayName = "Europe North"),
    AsiaPacific             UMETA(DisplayName = "Asia Pacific"),
    Japan                   UMETA(DisplayName = "Japan"),
    Oceania                 UMETA(DisplayName = "Oceania"),
    MiddleEast              UMETA(DisplayName = "Middle East")
};

// Diagnostic test types
UENUM(BlueprintType)
enum class EMGDiagnosticTest : uint8
{
    Ping                    UMETA(DisplayName = "Ping Test"),
    Bandwidth               UMETA(DisplayName = "Bandwidth Test"),
    PacketLoss              UMETA(DisplayName = "Packet Loss Test"),
    TraceRoute              UMETA(DisplayName = "Trace Route"),
    NATType                 UMETA(DisplayName = "NAT Type Detection"),
    PortCheck               UMETA(DisplayName = "Port Accessibility"),
    ServerHealth            UMETA(DisplayName = "Server Health"),
    FullDiagnostic          UMETA(DisplayName = "Full Diagnostic")
};

// Connection protocol types
UENUM(BlueprintType)
enum class EMGConnectionProtocol : uint8
{
    UDP                     UMETA(DisplayName = "UDP"),
    TCP                     UMETA(DisplayName = "TCP"),
    WebSocket               UMETA(DisplayName = "WebSocket"),
    QUIC                    UMETA(DisplayName = "QUIC")
};

// NAT type categories
UENUM(BlueprintType)
enum class EMGNATType : uint8
{
    Unknown                 UMETA(DisplayName = "Unknown"),
    Open                    UMETA(DisplayName = "Open"),
    Moderate                UMETA(DisplayName = "Moderate"),
    Strict                  UMETA(DisplayName = "Strict"),
    Symmetric               UMETA(DisplayName = "Symmetric"),
    DoubleNAT               UMETA(DisplayName = "Double NAT")
};

// Latency sample data
USTRUCT(BlueprintType)
struct FMGLatencySample
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JitterMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPacketLost = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ServerEndpoint;
};

// Latency statistics
USTRUCT(BlueprintType)
struct FMGLatencyStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentLatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinLatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxLatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JitterMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StandardDeviation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Percentile95Ms = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Percentile99Ms = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SampleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SamplePeriodSeconds = 0.0f;
};

// Packet loss statistics
USTRUCT(BlueprintType)
struct FMGPacketLossStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PacketsSent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PacketsReceived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PacketsLost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LossPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutOfOrderPackets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DuplicatePackets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLossBurstLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLossBurstLength = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime MeasurementStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MeasurementDurationSeconds = 0.0f;
};

// Bandwidth statistics
USTRUCT(BlueprintType)
struct FMGBandwidthStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DownloadSpeedMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UploadSpeedMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentDownloadUsageMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentUploadUsageMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeakDownloadMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeakUploadMbps = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesDownloaded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesUploaded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SessionDownloadMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SessionUploadMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBandwidthTestComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastTestTime;
};

// Server endpoint information
USTRUCT(BlueprintType)
struct FMGServerEndpoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ServerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Address;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Port = 7777;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGNetworkRegion Region = EMGNetworkRegion::Auto;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RegionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PacketLossPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPlayers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPlayers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOnline = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRecommended = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGConnectionQuality ConnectionQuality = EMGConnectionQuality::Disconnected;
};

// Network hop information for traceroute
USTRUCT(BlueprintType)
struct FMGNetworkHop
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HopNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Address;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Hostname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTimedOut = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ISP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Location;
};

// Diagnostic test result
USTRUCT(BlueprintType)
struct FMGDiagnosticResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid TestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGDiagnosticTest TestType = EMGDiagnosticTest::Ping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPassed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResultSummary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Recommendations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TestDurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGNetworkIssue> DetectedIssues;
};

// Connection health snapshot
USTRUCT(BlueprintType)
struct FMGConnectionHealth
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGConnectionQuality OverallQuality = EMGConnectionQuality::Disconnected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGLatencyStats LatencyStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGPacketLossStats PacketLossStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGBandwidthStats BandwidthStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGNATType NATType = EMGNATType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConnected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConnectionUptime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ReconnectAttempts = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastConnectedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGNetworkIssue> ActiveIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float QualityScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StabilityScore = 0.0f;
};

// Network event log entry
USTRUCT(BlueprintType)
struct FMGNetworkEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid EventId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGNetworkIssue IssueType = EMGNetworkIssue::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LatencyAtEvent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PacketLossAtEvent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWasAutoResolved = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResolutionDetails;
};

// Network quality thresholds configuration
USTRUCT(BlueprintType)
struct FMGNetworkQualityThresholds
{
    GENERATED_BODY()

    // Latency thresholds (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExcellentLatency = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodLatency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FairLatency = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PoorLatency = 150.0f;

    // Packet loss thresholds (percentage)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExcellentPacketLoss = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodPacketLoss = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FairPacketLoss = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PoorPacketLoss = 5.0f;

    // Jitter thresholds (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExcellentJitter = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodJitter = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FairJitter = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PoorJitter = 50.0f;
};

// Network configuration settings
USTRUCT(BlueprintType)
struct FMGNetworkConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGNetworkRegion PreferredRegion = EMGNetworkRegion::Auto;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGConnectionProtocol PreferredProtocol = EMGConnectionProtocol::UDP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoReconnect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxReconnectAttempts = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReconnectDelaySeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReconnectBackoffMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoSwitchRegion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RegionSwitchThresholdMs = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PingSampleInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PingSampleHistorySize = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePacketCompression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableClientPrediction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ServerReconciliationThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGNetworkQualityThresholds QualityThresholds;
};

// Full diagnostic report
USTRUCT(BlueprintType)
struct FMGDiagnosticReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ReportId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GeneratedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGConnectionHealth ConnectionHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDiagnosticResult> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGNetworkHop> TraceRouteHops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGServerEndpoint> TestedEndpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> OverallRecommendations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SystemInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ISPInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMeetsMinimumRequirements = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnConnectionQualityChanged, EMGConnectionQuality, OldQuality, EMGConnectionQuality, NewQuality);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNetworkIssueDetected, EMGNetworkIssue, Issue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNetworkIssueResolved, EMGNetworkIssue, Issue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLatencyUpdated, float, CurrentLatencyMs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPacketLossUpdated, float, LossPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDiagnosticComplete, const FMGDiagnosticResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnFullDiagnosticComplete, const FMGDiagnosticReport&, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnConnectionLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnConnectionRestored);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRegionSwitched, EMGNetworkRegion, NewRegion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBandwidthTestComplete, const FMGBandwidthStats&, Stats);

/**
 * UMGNetworkDiagnosticsSubsystem
 * Comprehensive network diagnostics and connection quality monitoring
 * Tracks latency, packet loss, jitter, and provides troubleshooting tools
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNetworkDiagnosticsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Connection management
    UFUNCTION(BlueprintCallable, Category = "Network|Connection")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Network|Connection")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Network|Connection")
    void SetPreferredRegion(EMGNetworkRegion Region);

    UFUNCTION(BlueprintCallable, Category = "Network|Connection")
    void ForceReconnect();

    UFUNCTION(BlueprintCallable, Category = "Network|Connection")
    void SwitchToOptimalRegion();

    UFUNCTION(BlueprintPure, Category = "Network|Connection")
    bool IsMonitoring() const { return bIsMonitoring; }

    UFUNCTION(BlueprintPure, Category = "Network|Connection")
    bool IsConnected() const { return CurrentHealth.bIsConnected; }

    UFUNCTION(BlueprintPure, Category = "Network|Connection")
    EMGConnectionQuality GetConnectionQuality() const { return CurrentHealth.OverallQuality; }

    // Latency monitoring
    UFUNCTION(BlueprintCallable, Category = "Network|Latency")
    void RecordLatencySample(float LatencyMs, const FString& ServerEndpoint);

    UFUNCTION(BlueprintPure, Category = "Network|Latency")
    float GetCurrentLatency() const { return CurrentHealth.LatencyStats.CurrentLatencyMs; }

    UFUNCTION(BlueprintPure, Category = "Network|Latency")
    float GetAverageLatency() const { return CurrentHealth.LatencyStats.AverageLatencyMs; }

    UFUNCTION(BlueprintPure, Category = "Network|Latency")
    float GetJitter() const { return CurrentHealth.LatencyStats.JitterMs; }

    UFUNCTION(BlueprintPure, Category = "Network|Latency")
    FMGLatencyStats GetLatencyStats() const { return CurrentHealth.LatencyStats; }

    UFUNCTION(BlueprintPure, Category = "Network|Latency")
    TArray<FMGLatencySample> GetLatencyHistory() const { return LatencyHistory; }

    // Packet loss monitoring
    UFUNCTION(BlueprintCallable, Category = "Network|PacketLoss")
    void RecordPacketSent();

    UFUNCTION(BlueprintCallable, Category = "Network|PacketLoss")
    void RecordPacketReceived(int32 SequenceNumber);

    UFUNCTION(BlueprintCallable, Category = "Network|PacketLoss")
    void RecordPacketLost();

    UFUNCTION(BlueprintPure, Category = "Network|PacketLoss")
    float GetPacketLossPercentage() const { return CurrentHealth.PacketLossStats.LossPercentage; }

    UFUNCTION(BlueprintPure, Category = "Network|PacketLoss")
    FMGPacketLossStats GetPacketLossStats() const { return CurrentHealth.PacketLossStats; }

    // Bandwidth monitoring
    UFUNCTION(BlueprintCallable, Category = "Network|Bandwidth")
    void StartBandwidthTest();

    UFUNCTION(BlueprintCallable, Category = "Network|Bandwidth")
    void CancelBandwidthTest();

    UFUNCTION(BlueprintCallable, Category = "Network|Bandwidth")
    void RecordBytesTransferred(int64 BytesSent, int64 BytesReceived, float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Network|Bandwidth")
    FMGBandwidthStats GetBandwidthStats() const { return CurrentHealth.BandwidthStats; }

    UFUNCTION(BlueprintPure, Category = "Network|Bandwidth")
    bool IsBandwidthTestRunning() const { return bBandwidthTestRunning; }

    // Diagnostic tests
    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void RunDiagnosticTest(EMGDiagnosticTest TestType);

    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void RunFullDiagnostic();

    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void CancelDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void RunPingTest(const FString& ServerAddress);

    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void RunTraceRoute(const FString& TargetAddress);

    UFUNCTION(BlueprintCallable, Category = "Network|Diagnostics")
    void DetectNATType();

    UFUNCTION(BlueprintPure, Category = "Network|Diagnostics")
    EMGNATType GetNATType() const { return CurrentHealth.NATType; }

    UFUNCTION(BlueprintPure, Category = "Network|Diagnostics")
    bool IsDiagnosticRunning() const { return bDiagnosticRunning; }

    UFUNCTION(BlueprintPure, Category = "Network|Diagnostics")
    TArray<FMGDiagnosticResult> GetDiagnosticHistory() const { return DiagnosticHistory; }

    UFUNCTION(BlueprintPure, Category = "Network|Diagnostics")
    FMGDiagnosticReport GetLastDiagnosticReport() const { return LastDiagnosticReport; }

    // Server management
    UFUNCTION(BlueprintCallable, Category = "Network|Servers")
    void RefreshServerList();

    UFUNCTION(BlueprintCallable, Category = "Network|Servers")
    void PingAllServers();

    UFUNCTION(BlueprintCallable, Category = "Network|Servers")
    void AddCustomServer(const FMGServerEndpoint& Server);

    UFUNCTION(BlueprintCallable, Category = "Network|Servers")
    void RemoveCustomServer(const FString& ServerId);

    UFUNCTION(BlueprintPure, Category = "Network|Servers")
    TArray<FMGServerEndpoint> GetAvailableServers() const { return AvailableServers; }

    UFUNCTION(BlueprintPure, Category = "Network|Servers")
    FMGServerEndpoint GetBestServer() const;

    UFUNCTION(BlueprintPure, Category = "Network|Servers")
    TArray<FMGServerEndpoint> GetServersByRegion(EMGNetworkRegion Region) const;

    UFUNCTION(BlueprintPure, Category = "Network|Servers")
    FMGServerEndpoint GetCurrentServer() const { return CurrentServer; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Network|Config")
    void ApplyNetworkConfig(const FMGNetworkConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Network|Config")
    void SetQualityThresholds(const FMGNetworkQualityThresholds& Thresholds);

    UFUNCTION(BlueprintCallable, Category = "Network|Config")
    void SetPingSampleInterval(float IntervalSeconds);

    UFUNCTION(BlueprintCallable, Category = "Network|Config")
    void SetAutoReconnect(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Network|Config")
    FMGNetworkConfig GetNetworkConfig() const { return NetworkConfig; }

    // Health and status
    UFUNCTION(BlueprintPure, Category = "Network|Health")
    FMGConnectionHealth GetConnectionHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Network|Health")
    float GetQualityScore() const { return CurrentHealth.QualityScore; }

    UFUNCTION(BlueprintPure, Category = "Network|Health")
    float GetStabilityScore() const { return CurrentHealth.StabilityScore; }

    UFUNCTION(BlueprintPure, Category = "Network|Health")
    TArray<EMGNetworkIssue> GetActiveIssues() const { return CurrentHealth.ActiveIssues; }

    UFUNCTION(BlueprintPure, Category = "Network|Health")
    bool HasActiveIssue(EMGNetworkIssue Issue) const;

    // Event logging
    UFUNCTION(BlueprintPure, Category = "Network|Events")
    TArray<FMGNetworkEvent> GetNetworkEventLog() const { return NetworkEventLog; }

    UFUNCTION(BlueprintCallable, Category = "Network|Events")
    void ClearNetworkEventLog();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Network|Utility")
    FString GenerateNetworkReport() const;

    UFUNCTION(BlueprintCallable, Category = "Network|Utility")
    void ExportDiagnosticReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Network|Utility")
    void CopyDiagnosticToClipboard();

    UFUNCTION(BlueprintPure, Category = "Network|Utility")
    FString GetConnectionQualityDisplayString() const;

    UFUNCTION(BlueprintPure, Category = "Network|Utility")
    FLinearColor GetConnectionQualityColor() const;

    UFUNCTION(BlueprintPure, Category = "Network|Utility")
    FString FormatLatency(float LatencyMs) const;

    UFUNCTION(BlueprintPure, Category = "Network|Utility")
    FString GetRecommendationsForIssue(EMGNetworkIssue Issue) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnConnectionQualityChanged OnConnectionQualityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnNetworkIssueDetected OnNetworkIssueDetected;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnNetworkIssueResolved OnNetworkIssueResolved;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnLatencyUpdated OnLatencyUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnPacketLossUpdated OnPacketLossUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnDiagnosticComplete OnDiagnosticComplete;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnFullDiagnosticComplete OnFullDiagnosticComplete;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnConnectionLost OnConnectionLost;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnConnectionRestored OnConnectionRestored;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnRegionSwitched OnRegionSwitched;

    UPROPERTY(BlueprintAssignable, Category = "Network|Events")
    FMGOnBandwidthTestComplete OnBandwidthTestComplete;

protected:
    UPROPERTY()
    FMGConnectionHealth CurrentHealth;

    UPROPERTY()
    FMGNetworkConfig NetworkConfig;

    UPROPERTY()
    TArray<FMGLatencySample> LatencyHistory;

    UPROPERTY()
    TArray<FMGServerEndpoint> AvailableServers;

    UPROPERTY()
    FMGServerEndpoint CurrentServer;

    UPROPERTY()
    TArray<FMGDiagnosticResult> DiagnosticHistory;

    UPROPERTY()
    FMGDiagnosticReport LastDiagnosticReport;

    UPROPERTY()
    TArray<FMGNetworkEvent> NetworkEventLog;

    UPROPERTY()
    bool bIsMonitoring = false;

    UPROPERTY()
    bool bBandwidthTestRunning = false;

    UPROPERTY()
    bool bDiagnosticRunning = false;

    UPROPERTY()
    int32 NextExpectedSequence = 0;

    UPROPERTY()
    int32 CurrentLossBurstLength = 0;

    FTimerHandle PingTimerHandle;
    FTimerHandle BandwidthTestHandle;
    FTimerHandle DiagnosticHandle;
    FTimerHandle ReconnectHandle;

    void UpdateLatencyStats();
    void UpdatePacketLossStats();
    void UpdateConnectionQuality();
    void UpdateQualityScore();
    void CheckForIssues();
    void LogNetworkEvent(EMGNetworkIssue Issue, const FString& Description);
    void AttemptReconnect();
    void ProcessDiagnosticQueue();
    void SimulatePing();
    void SimulateBandwidthTest();
    EMGConnectionQuality CalculateQualityFromStats() const;
    void InitializeDefaultServers();
    TArray<EMGDiagnosticTest> PendingDiagnosticTests;
};
