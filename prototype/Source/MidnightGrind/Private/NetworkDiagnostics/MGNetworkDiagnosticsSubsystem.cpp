// Copyright Midnight Grind. All Rights Reserved.

// MGNetworkDiagnosticsSubsystem.cpp
// Midnight Grind - Network Diagnostics and Connection Quality System

#include "NetworkDiagnostics/MGNetworkDiagnosticsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Guid.h"
#include "HAL/PlatformMisc.h"
#include "Misc/DateTime.h"

void UMGNetworkDiagnosticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default configuration
    NetworkConfig.PreferredRegion = EMGNetworkRegion::Auto;
    NetworkConfig.PreferredProtocol = EMGConnectionProtocol::UDP;
    NetworkConfig.bAutoReconnect = true;
    NetworkConfig.MaxReconnectAttempts = 5;
    NetworkConfig.ReconnectDelaySeconds = 2.0f;
    NetworkConfig.ReconnectBackoffMultiplier = 1.5f;
    NetworkConfig.bAutoSwitchRegion = true;
    NetworkConfig.RegionSwitchThresholdMs = 100.0f;
    NetworkConfig.PingSampleInterval = 1.0f;
    NetworkConfig.PingSampleHistorySize = 60;
    NetworkConfig.bEnablePacketCompression = true;
    NetworkConfig.bEnableClientPrediction = true;
    NetworkConfig.ServerReconciliationThreshold = 0.1f;

    // Initialize default thresholds
    NetworkConfig.QualityThresholds.ExcellentLatency = 30.0f;
    NetworkConfig.QualityThresholds.GoodLatency = 60.0f;
    NetworkConfig.QualityThresholds.FairLatency = 100.0f;
    NetworkConfig.QualityThresholds.PoorLatency = 150.0f;
    NetworkConfig.QualityThresholds.ExcellentPacketLoss = 0.0f;
    NetworkConfig.QualityThresholds.GoodPacketLoss = 1.0f;
    NetworkConfig.QualityThresholds.FairPacketLoss = 3.0f;
    NetworkConfig.QualityThresholds.PoorPacketLoss = 5.0f;
    NetworkConfig.QualityThresholds.ExcellentJitter = 5.0f;
    NetworkConfig.QualityThresholds.GoodJitter = 15.0f;
    NetworkConfig.QualityThresholds.FairJitter = 30.0f;
    NetworkConfig.QualityThresholds.PoorJitter = 50.0f;

    // Initialize health data
    CurrentHealth.OverallQuality = EMGConnectionQuality::Disconnected;
    CurrentHealth.bIsConnected = false;
    CurrentHealth.QualityScore = 0.0f;
    CurrentHealth.StabilityScore = 0.0f;

    // Initialize packet loss stats
    CurrentHealth.PacketLossStats.MeasurementStartTime = FDateTime::Now();

    // Initialize default servers
    InitializeDefaultServers();
}

void UMGNetworkDiagnosticsSubsystem::Deinitialize()
{
    StopMonitoring();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PingTimerHandle);
        World->GetTimerManager().ClearTimer(BandwidthTestHandle);
        World->GetTimerManager().ClearTimer(DiagnosticHandle);
        World->GetTimerManager().ClearTimer(ReconnectHandle);
    }

    Super::Deinitialize();
}

void UMGNetworkDiagnosticsSubsystem::InitializeDefaultServers()
{
    AvailableServers.Empty();

    // North America East
    FMGServerEndpoint NAEast;
    NAEast.ServerId = TEXT("na-east-01");
    NAEast.Address = TEXT("game-na-east.midnightgrind.com");
    NAEast.Port = 7777;
    NAEast.Region = EMGNetworkRegion::NAEast;
    NAEast.RegionName = TEXT("North America East");
    NAEast.MaxPlayers = 16;
    NAEast.bIsOnline = true;
    AvailableServers.Add(NAEast);

    // North America West
    FMGServerEndpoint NAWest;
    NAWest.ServerId = TEXT("na-west-01");
    NAWest.Address = TEXT("game-na-west.midnightgrind.com");
    NAWest.Port = 7777;
    NAWest.Region = EMGNetworkRegion::NAWest;
    NAWest.RegionName = TEXT("North America West");
    NAWest.MaxPlayers = 16;
    NAWest.bIsOnline = true;
    AvailableServers.Add(NAWest);

    // Europe West
    FMGServerEndpoint EUWest;
    EUWest.ServerId = TEXT("eu-west-01");
    EUWest.Address = TEXT("game-eu-west.midnightgrind.com");
    EUWest.Port = 7777;
    EUWest.Region = EMGNetworkRegion::EuropeWest;
    EUWest.RegionName = TEXT("Europe West");
    EUWest.MaxPlayers = 16;
    EUWest.bIsOnline = true;
    AvailableServers.Add(EUWest);

    // Europe North
    FMGServerEndpoint EUNorth;
    EUNorth.ServerId = TEXT("eu-north-01");
    EUNorth.Address = TEXT("game-eu-north.midnightgrind.com");
    EUNorth.Port = 7777;
    EUNorth.Region = EMGNetworkRegion::EuropeNorth;
    EUNorth.RegionName = TEXT("Europe North");
    EUNorth.MaxPlayers = 16;
    EUNorth.bIsOnline = true;
    AvailableServers.Add(EUNorth);

    // Asia Pacific
    FMGServerEndpoint APAC;
    APAC.ServerId = TEXT("apac-01");
    APAC.Address = TEXT("game-apac.midnightgrind.com");
    APAC.Port = 7777;
    APAC.Region = EMGNetworkRegion::AsiaPacific;
    APAC.RegionName = TEXT("Asia Pacific");
    APAC.MaxPlayers = 16;
    APAC.bIsOnline = true;
    AvailableServers.Add(APAC);

    // Japan
    FMGServerEndpoint Japan;
    Japan.ServerId = TEXT("jp-01");
    Japan.Address = TEXT("game-jp.midnightgrind.com");
    Japan.Port = 7777;
    Japan.Region = EMGNetworkRegion::Japan;
    Japan.RegionName = TEXT("Japan");
    Japan.MaxPlayers = 16;
    Japan.bIsOnline = true;
    AvailableServers.Add(Japan);

    // Oceania
    FMGServerEndpoint Oceania;
    Oceania.ServerId = TEXT("oce-01");
    Oceania.Address = TEXT("game-oce.midnightgrind.com");
    Oceania.Port = 7777;
    Oceania.Region = EMGNetworkRegion::Oceania;
    Oceania.RegionName = TEXT("Oceania");
    Oceania.MaxPlayers = 16;
    Oceania.bIsOnline = true;
    AvailableServers.Add(Oceania);

    // South America
    FMGServerEndpoint SAmerica;
    SAmerica.ServerId = TEXT("sa-01");
    SAmerica.Address = TEXT("game-sa.midnightgrind.com");
    SAmerica.Port = 7777;
    SAmerica.Region = EMGNetworkRegion::SouthAmerica;
    SAmerica.RegionName = TEXT("South America");
    SAmerica.MaxPlayers = 16;
    SAmerica.bIsOnline = true;
    AvailableServers.Add(SAmerica);
}

void UMGNetworkDiagnosticsSubsystem::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    CurrentHealth.bIsConnected = true;
    CurrentHealth.LastConnectedTime = FDateTime::Now();
    CurrentHealth.ReconnectAttempts = 0;

    // Start periodic ping sampling
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            PingTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->SimulatePing();
                }
            },
            NetworkConfig.PingSampleInterval,
            true
        );
    }

    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Network monitoring started"));
}

void UMGNetworkDiagnosticsSubsystem::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PingTimerHandle);
    }

    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Network monitoring stopped"));
}

void UMGNetworkDiagnosticsSubsystem::SetPreferredRegion(EMGNetworkRegion Region)
{
    if (NetworkConfig.PreferredRegion != Region)
    {
        NetworkConfig.PreferredRegion = Region;

        if (Region != EMGNetworkRegion::Auto)
        {
            // Find best server in selected region
            TArray<FMGServerEndpoint> RegionServers = GetServersByRegion(Region);
            if (RegionServers.Num() > 0)
            {
                CurrentServer = RegionServers[0];
                OnRegionSwitched.Broadcast(Region);
            }
        }
        else
        {
            SwitchToOptimalRegion();
        }
    }
}

void UMGNetworkDiagnosticsSubsystem::ForceReconnect()
{
    if (CurrentHealth.bIsConnected)
    {
        CurrentHealth.bIsConnected = false;
        OnConnectionLost.Broadcast();
    }

    AttemptReconnect();
}

void UMGNetworkDiagnosticsSubsystem::SwitchToOptimalRegion()
{
    FMGServerEndpoint BestServer = GetBestServer();

    if (!BestServer.ServerId.IsEmpty() && BestServer.ServerId != CurrentServer.ServerId)
    {
        CurrentServer = BestServer;
        OnRegionSwitched.Broadcast(BestServer.Region);
        LogNetworkEvent(EMGNetworkIssue::None, FString::Printf(TEXT("Switched to optimal region: %s"), *BestServer.RegionName));
    }
}

void UMGNetworkDiagnosticsSubsystem::RecordLatencySample(float LatencyMs, const FString& ServerEndpoint)
{
    // Calculate jitter from previous sample
    float Jitter = 0.0f;
    if (LatencyHistory.Num() > 0)
    {
        Jitter = FMath::Abs(LatencyMs - LatencyHistory.Last().LatencyMs);
    }

    // Create new sample
    FMGLatencySample Sample;
    Sample.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Sample.LatencyMs = LatencyMs;
    Sample.JitterMs = Jitter;
    Sample.bPacketLost = false;
    Sample.ServerEndpoint = ServerEndpoint;

    // Add to history
    LatencyHistory.Add(Sample);

    // Trim history to configured size
    while (LatencyHistory.Num() > NetworkConfig.PingSampleHistorySize)
    {
        LatencyHistory.RemoveAt(0);
    }

    // Update statistics
    UpdateLatencyStats();
    UpdateConnectionQuality();
    CheckForIssues();

    OnLatencyUpdated.Broadcast(LatencyMs);
}

void UMGNetworkDiagnosticsSubsystem::RecordPacketSent()
{
    CurrentHealth.PacketLossStats.PacketsSent++;
}

void UMGNetworkDiagnosticsSubsystem::RecordPacketReceived(int32 SequenceNumber)
{
    CurrentHealth.PacketLossStats.PacketsReceived++;

    // Check for out of order packets
    if (SequenceNumber < NextExpectedSequence)
    {
        CurrentHealth.PacketLossStats.OutOfOrderPackets++;
    }
    else if (SequenceNumber == NextExpectedSequence)
    {
        NextExpectedSequence++;
        CurrentLossBurstLength = 0;
    }
    else
    {
        // Packets between NextExpectedSequence and SequenceNumber are lost
        int32 LostCount = SequenceNumber - NextExpectedSequence;
        CurrentHealth.PacketLossStats.PacketsLost += LostCount;
        CurrentLossBurstLength += LostCount;

        if (CurrentLossBurstLength > CurrentHealth.PacketLossStats.MaxLossBurstLength)
        {
            CurrentHealth.PacketLossStats.MaxLossBurstLength = CurrentLossBurstLength;
        }

        NextExpectedSequence = SequenceNumber + 1;
    }

    UpdatePacketLossStats();
}

void UMGNetworkDiagnosticsSubsystem::RecordPacketLost()
{
    CurrentHealth.PacketLossStats.PacketsLost++;
    CurrentLossBurstLength++;

    if (CurrentLossBurstLength > CurrentHealth.PacketLossStats.MaxLossBurstLength)
    {
        CurrentHealth.PacketLossStats.MaxLossBurstLength = CurrentLossBurstLength;
    }

    UpdatePacketLossStats();
    OnPacketLossUpdated.Broadcast(CurrentHealth.PacketLossStats.LossPercentage);
}

void UMGNetworkDiagnosticsSubsystem::StartBandwidthTest()
{
    if (bBandwidthTestRunning)
    {
        return;
    }

    bBandwidthTestRunning = true;
    CurrentHealth.BandwidthStats.bBandwidthTestComplete = false;

    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Bandwidth test started"));

    // Simulate bandwidth test
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            BandwidthTestHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->SimulateBandwidthTest();
                }
            },
            0.1f,
            true
        );

        // Complete test after 5 seconds
        FTimerHandle CompletionHandle;
        World->GetTimerManager().SetTimer(
            CompletionHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->bBandwidthTestRunning = false;
                    WeakThis->CurrentHealth.BandwidthStats.bBandwidthTestComplete = true;
                    WeakThis->CurrentHealth.BandwidthStats.LastTestTime = FDateTime::Now();

                    if (UWorld* InnerWorld = WeakThis->GetWorld())
                    {
                        InnerWorld->GetTimerManager().ClearTimer(WeakThis->BandwidthTestHandle);
                    }

                    WeakThis->OnBandwidthTestComplete.Broadcast(WeakThis->CurrentHealth.BandwidthStats);
                    WeakThis->LogNetworkEvent(EMGNetworkIssue::None, TEXT("Bandwidth test completed"));
                }
            },
            5.0f,
            false
        );
    }
}

void UMGNetworkDiagnosticsSubsystem::CancelBandwidthTest()
{
    if (!bBandwidthTestRunning)
    {
        return;
    }

    bBandwidthTestRunning = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BandwidthTestHandle);
    }

    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Bandwidth test cancelled"));
}

void UMGNetworkDiagnosticsSubsystem::RecordBytesTransferred(int64 BytesSent, int64 BytesReceived, float DeltaTime)
{
    CurrentHealth.BandwidthStats.TotalBytesDownloaded += BytesReceived;
    CurrentHealth.BandwidthStats.TotalBytesUploaded += BytesSent;

    if (DeltaTime > 0.0f)
    {
        // Calculate current throughput in Mbps
        float DownloadMbps = (BytesReceived * 8.0f) / (DeltaTime * 1000000.0f);
        float UploadMbps = (BytesSent * 8.0f) / (DeltaTime * 1000000.0f);

        CurrentHealth.BandwidthStats.CurrentDownloadUsageMbps = DownloadMbps;
        CurrentHealth.BandwidthStats.CurrentUploadUsageMbps = UploadMbps;

        // Track peaks
        if (DownloadMbps > CurrentHealth.BandwidthStats.PeakDownloadMbps)
        {
            CurrentHealth.BandwidthStats.PeakDownloadMbps = DownloadMbps;
        }
        if (UploadMbps > CurrentHealth.BandwidthStats.PeakUploadMbps)
        {
            CurrentHealth.BandwidthStats.PeakUploadMbps = UploadMbps;
        }
    }

    // Update session totals
    CurrentHealth.BandwidthStats.SessionDownloadMB = CurrentHealth.BandwidthStats.TotalBytesDownloaded / (1024.0f * 1024.0f);
    CurrentHealth.BandwidthStats.SessionUploadMB = CurrentHealth.BandwidthStats.TotalBytesUploaded / (1024.0f * 1024.0f);
}

void UMGNetworkDiagnosticsSubsystem::RunDiagnosticTest(EMGDiagnosticTest TestType)
{
    if (bDiagnosticRunning)
    {
        PendingDiagnosticTests.Add(TestType);
        return;
    }

    bDiagnosticRunning = true;

    FMGDiagnosticResult Result;
    Result.TestId = FGuid::NewGuid();
    Result.TestType = TestType;
    Result.TestTime = FDateTime::Now();

    float TestDuration = 2.0f;

    switch (TestType)
    {
    case EMGDiagnosticTest::Ping:
        Result.ResultSummary = FString::Printf(TEXT("Ping: %.1f ms average"), CurrentHealth.LatencyStats.AverageLatencyMs);
        Result.bPassed = CurrentHealth.LatencyStats.AverageLatencyMs < NetworkConfig.QualityThresholds.PoorLatency;
        Result.DetailedResults.Add(FString::Printf(TEXT("Current: %.1f ms"), CurrentHealth.LatencyStats.CurrentLatencyMs));
        Result.DetailedResults.Add(FString::Printf(TEXT("Average: %.1f ms"), CurrentHealth.LatencyStats.AverageLatencyMs));
        Result.DetailedResults.Add(FString::Printf(TEXT("Min: %.1f ms"), CurrentHealth.LatencyStats.MinLatencyMs));
        Result.DetailedResults.Add(FString::Printf(TEXT("Max: %.1f ms"), CurrentHealth.LatencyStats.MaxLatencyMs));
        Result.DetailedResults.Add(FString::Printf(TEXT("Jitter: %.1f ms"), CurrentHealth.LatencyStats.JitterMs));
        TestDuration = 1.0f;
        break;

    case EMGDiagnosticTest::PacketLoss:
        Result.ResultSummary = FString::Printf(TEXT("Packet Loss: %.2f%%"), CurrentHealth.PacketLossStats.LossPercentage);
        Result.bPassed = CurrentHealth.PacketLossStats.LossPercentage < NetworkConfig.QualityThresholds.PoorPacketLoss;
        Result.DetailedResults.Add(FString::Printf(TEXT("Packets Sent: %d"), CurrentHealth.PacketLossStats.PacketsSent));
        Result.DetailedResults.Add(FString::Printf(TEXT("Packets Received: %d"), CurrentHealth.PacketLossStats.PacketsReceived));
        Result.DetailedResults.Add(FString::Printf(TEXT("Packets Lost: %d"), CurrentHealth.PacketLossStats.PacketsLost));
        Result.DetailedResults.Add(FString::Printf(TEXT("Out of Order: %d"), CurrentHealth.PacketLossStats.OutOfOrderPackets));
        TestDuration = 1.0f;
        break;

    case EMGDiagnosticTest::Bandwidth:
        StartBandwidthTest();
        Result.ResultSummary = TEXT("Bandwidth test in progress...");
        TestDuration = 6.0f;
        break;

    case EMGDiagnosticTest::NATType:
        DetectNATType();
        Result.ResultSummary = FString::Printf(TEXT("NAT Type: %s"), *UEnum::GetDisplayValueAsText(CurrentHealth.NATType).ToString());
        Result.bPassed = CurrentHealth.NATType != EMGNATType::Strict && CurrentHealth.NATType != EMGNATType::Symmetric;
        TestDuration = 3.0f;
        break;

    case EMGDiagnosticTest::TraceRoute:
        if (!CurrentServer.Address.IsEmpty())
        {
            RunTraceRoute(CurrentServer.Address);
        }
        Result.ResultSummary = TEXT("Trace route in progress...");
        TestDuration = 5.0f;
        break;

    case EMGDiagnosticTest::PortCheck:
        Result.ResultSummary = TEXT("Checking port accessibility...");
        Result.bPassed = true;
        Result.DetailedResults.Add(TEXT("Port 7777 (Game): Open"));
        Result.DetailedResults.Add(TEXT("Port 7778 (Voice): Open"));
        Result.DetailedResults.Add(TEXT("Port 443 (HTTPS): Open"));
        TestDuration = 2.0f;
        break;

    case EMGDiagnosticTest::ServerHealth:
        Result.ResultSummary = TEXT("Checking server health...");
        Result.bPassed = CurrentServer.bIsOnline;
        Result.DetailedResults.Add(FString::Printf(TEXT("Server: %s"), *CurrentServer.ServerId));
        Result.DetailedResults.Add(FString::Printf(TEXT("Status: %s"), CurrentServer.bIsOnline ? TEXT("Online") : TEXT("Offline")));
        Result.DetailedResults.Add(FString::Printf(TEXT("Players: %d/%d"), CurrentServer.CurrentPlayers, CurrentServer.MaxPlayers));
        TestDuration = 1.0f;
        break;

    case EMGDiagnosticTest::FullDiagnostic:
        RunFullDiagnostic();
        return;
    }

    // Add recommendations based on results
    if (!Result.bPassed)
    {
        switch (TestType)
        {
        case EMGDiagnosticTest::Ping:
            Result.Recommendations.Add(TEXT("Try connecting to a closer server region"));
            Result.Recommendations.Add(TEXT("Check for bandwidth-heavy applications"));
            Result.Recommendations.Add(TEXT("Consider using a wired connection"));
            Result.DetectedIssues.Add(EMGNetworkIssue::HighLatency);
            break;
        case EMGDiagnosticTest::PacketLoss:
            Result.Recommendations.Add(TEXT("Check your network cable connections"));
            Result.Recommendations.Add(TEXT("Restart your router/modem"));
            Result.Recommendations.Add(TEXT("Contact your ISP if issue persists"));
            Result.DetectedIssues.Add(EMGNetworkIssue::PacketLoss);
            break;
        case EMGDiagnosticTest::NATType:
            Result.Recommendations.Add(TEXT("Enable UPnP on your router"));
            Result.Recommendations.Add(TEXT("Configure port forwarding for game ports"));
            Result.Recommendations.Add(TEXT("Check if your ISP uses CGNAT"));
            Result.DetectedIssues.Add(EMGNetworkIssue::NATIssue);
            break;
        default:
            break;
        }
    }

    // Complete test after simulated duration
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DiagnosticHandle,
            [WeakThis, Result]() mutable
            {
                if (!WeakThis.IsValid())
                {
                    return;
                }
                Result.TestDurationSeconds = (FDateTime::Now() - Result.TestTime).GetTotalSeconds();
                WeakThis->DiagnosticHistory.Add(Result);
                WeakThis->bDiagnosticRunning = false;

                WeakThis->OnDiagnosticComplete.Broadcast(Result);
                WeakThis->ProcessDiagnosticQueue();
            },
            TestDuration,
            false
        );
    }
}

void UMGNetworkDiagnosticsSubsystem::RunFullDiagnostic()
{
    bDiagnosticRunning = true;

    LastDiagnosticReport = FMGDiagnosticReport();
    LastDiagnosticReport.ReportId = FGuid::NewGuid();
    LastDiagnosticReport.GeneratedAt = FDateTime::Now();
    LastDiagnosticReport.ConnectionHealth = CurrentHealth;
    LastDiagnosticReport.TestedEndpoints = AvailableServers;

    // Queue all diagnostic tests
    PendingDiagnosticTests.Empty();
    PendingDiagnosticTests.Add(EMGDiagnosticTest::Ping);
    PendingDiagnosticTests.Add(EMGDiagnosticTest::PacketLoss);
    PendingDiagnosticTests.Add(EMGDiagnosticTest::Bandwidth);
    PendingDiagnosticTests.Add(EMGDiagnosticTest::NATType);
    PendingDiagnosticTests.Add(EMGDiagnosticTest::PortCheck);
    PendingDiagnosticTests.Add(EMGDiagnosticTest::ServerHealth);

    // System info
    LastDiagnosticReport.SystemInfo = FString::Printf(
        TEXT("Platform: %s, CPU Cores: %d"),
        *FPlatformMisc::GetCPUBrand(),
        FPlatformMisc::NumberOfCores()
    );

    bDiagnosticRunning = false;
    ProcessDiagnosticQueue();

    // Complete full diagnostic after all tests finish
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DiagnosticHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid() && WeakThis->PendingDiagnosticTests.Num() == 0 && !WeakThis->bDiagnosticRunning)
                {
                    // Compile final report
                    WeakThis->LastDiagnosticReport.TestResults = WeakThis->DiagnosticHistory;

                    // Determine if meets minimum requirements
                    WeakThis->LastDiagnosticReport.bMeetsMinimumRequirements =
                        WeakThis->CurrentHealth.LatencyStats.AverageLatencyMs < 200.0f &&
                        WeakThis->CurrentHealth.PacketLossStats.LossPercentage < 10.0f &&
                        WeakThis->CurrentHealth.NATType != EMGNATType::Symmetric;

                    // Generate overall recommendations
                    if (WeakThis->CurrentHealth.LatencyStats.AverageLatencyMs > WeakThis->NetworkConfig.QualityThresholds.FairLatency)
                    {
                        WeakThis->LastDiagnosticReport.OverallRecommendations.Add(TEXT("High latency detected. Consider selecting a closer server region."));
                    }
                    if (WeakThis->CurrentHealth.PacketLossStats.LossPercentage > WeakThis->NetworkConfig.QualityThresholds.FairPacketLoss)
                    {
                        WeakThis->LastDiagnosticReport.OverallRecommendations.Add(TEXT("Packet loss detected. Check your network connection quality."));
                    }
                    if (WeakThis->CurrentHealth.NATType == EMGNATType::Strict || WeakThis->CurrentHealth.NATType == EMGNATType::Symmetric)
                    {
                        WeakThis->LastDiagnosticReport.OverallRecommendations.Add(TEXT("Restrictive NAT detected. You may have difficulty connecting to other players."));
                    }

                    WeakThis->OnFullDiagnosticComplete.Broadcast(WeakThis->LastDiagnosticReport);
                    WeakThis->LogNetworkEvent(EMGNetworkIssue::None, TEXT("Full diagnostic completed"));
                }
            },
            15.0f,
            false
        );
    }
}

void UMGNetworkDiagnosticsSubsystem::CancelDiagnostics()
{
    bDiagnosticRunning = false;
    PendingDiagnosticTests.Empty();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DiagnosticHandle);
    }

    CancelBandwidthTest();
    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Diagnostics cancelled"));
}

void UMGNetworkDiagnosticsSubsystem::RunPingTest(const FString& ServerAddress)
{
    // Simulate ping test
    float SimulatedLatency = FMath::RandRange(20.0f, 100.0f);
    RecordLatencySample(SimulatedLatency, ServerAddress);

    FMGDiagnosticResult Result;
    Result.TestId = FGuid::NewGuid();
    Result.TestType = EMGDiagnosticTest::Ping;
    Result.TestTime = FDateTime::Now();
    Result.TestDurationSeconds = 0.1f;
    Result.bPassed = SimulatedLatency < NetworkConfig.QualityThresholds.PoorLatency;
    Result.ResultSummary = FString::Printf(TEXT("Ping to %s: %.1f ms"), *ServerAddress, SimulatedLatency);

    DiagnosticHistory.Add(Result);
    OnDiagnosticComplete.Broadcast(Result);
}

void UMGNetworkDiagnosticsSubsystem::RunTraceRoute(const FString& TargetAddress)
{
    // Simulate trace route with realistic hops
    LastDiagnosticReport.TraceRouteHops.Empty();

    TArray<FString> SimulatedAddresses = {
        TEXT("192.168.1.1"),
        TEXT("10.0.0.1"),
        TEXT("72.14.215.85"),
        TEXT("108.170.248.97"),
        TEXT("172.253.68.48"),
        TargetAddress
    };

    float CumulativeLatency = 0.0f;

    for (int32 i = 0; i < SimulatedAddresses.Num(); ++i)
    {
        FMGNetworkHop Hop;
        Hop.HopNumber = i + 1;
        Hop.Address = SimulatedAddresses[i];
        Hop.Hostname = FString::Printf(TEXT("hop%d.network.net"), i + 1);

        float HopLatency = FMath::RandRange(2.0f, 15.0f);
        CumulativeLatency += HopLatency;
        Hop.LatencyMs = CumulativeLatency;
        Hop.bTimedOut = false;

        if (i == 0)
        {
            Hop.ISP = TEXT("Local Router");
            Hop.Location = TEXT("Local");
        }
        else if (i < 3)
        {
            Hop.ISP = TEXT("Transit ISP");
            Hop.Location = TEXT("Regional");
        }
        else
        {
            Hop.ISP = TEXT("Game Server Network");
            Hop.Location = TEXT("Data Center");
        }

        LastDiagnosticReport.TraceRouteHops.Add(Hop);
    }

    LogNetworkEvent(EMGNetworkIssue::None, FString::Printf(TEXT("Trace route to %s completed: %d hops"), *TargetAddress, SimulatedAddresses.Num()));
}

void UMGNetworkDiagnosticsSubsystem::DetectNATType()
{
    // Simulate NAT detection
    int32 NATRoll = FMath::RandRange(0, 100);

    if (NATRoll < 40)
    {
        CurrentHealth.NATType = EMGNATType::Open;
    }
    else if (NATRoll < 70)
    {
        CurrentHealth.NATType = EMGNATType::Moderate;
    }
    else if (NATRoll < 90)
    {
        CurrentHealth.NATType = EMGNATType::Strict;
    }
    else
    {
        CurrentHealth.NATType = EMGNATType::Symmetric;
    }

    LogNetworkEvent(EMGNetworkIssue::None, FString::Printf(TEXT("NAT type detected: %s"), *UEnum::GetDisplayValueAsText(CurrentHealth.NATType).ToString()));
}

void UMGNetworkDiagnosticsSubsystem::RefreshServerList()
{
    // Re-initialize default servers (in real implementation, would query master server)
    InitializeDefaultServers();
    PingAllServers();
}

void UMGNetworkDiagnosticsSubsystem::PingAllServers()
{
    for (FMGServerEndpoint& Server : AvailableServers)
    {
        // Simulate latency based on region
        float BaseLatency = 30.0f;

        switch (Server.Region)
        {
        case EMGNetworkRegion::NAEast:
            BaseLatency = 35.0f;
            break;
        case EMGNetworkRegion::NAWest:
            BaseLatency = 45.0f;
            break;
        case EMGNetworkRegion::EuropeWest:
            BaseLatency = 85.0f;
            break;
        case EMGNetworkRegion::EuropeNorth:
            BaseLatency = 95.0f;
            break;
        case EMGNetworkRegion::AsiaPacific:
            BaseLatency = 150.0f;
            break;
        case EMGNetworkRegion::Japan:
            BaseLatency = 120.0f;
            break;
        case EMGNetworkRegion::Oceania:
            BaseLatency = 180.0f;
            break;
        case EMGNetworkRegion::SouthAmerica:
            BaseLatency = 140.0f;
            break;
        default:
            BaseLatency = 100.0f;
            break;
        }

        Server.LatencyMs = BaseLatency + FMath::RandRange(-10.0f, 20.0f);
        Server.PacketLossPercent = FMath::RandRange(0.0f, 2.0f);
        Server.CurrentPlayers = FMath::RandRange(0, Server.MaxPlayers);

        // Determine connection quality
        if (Server.LatencyMs < NetworkConfig.QualityThresholds.ExcellentLatency)
        {
            Server.ConnectionQuality = EMGConnectionQuality::Excellent;
            Server.bIsRecommended = true;
        }
        else if (Server.LatencyMs < NetworkConfig.QualityThresholds.GoodLatency)
        {
            Server.ConnectionQuality = EMGConnectionQuality::Good;
            Server.bIsRecommended = true;
        }
        else if (Server.LatencyMs < NetworkConfig.QualityThresholds.FairLatency)
        {
            Server.ConnectionQuality = EMGConnectionQuality::Fair;
            Server.bIsRecommended = false;
        }
        else if (Server.LatencyMs < NetworkConfig.QualityThresholds.PoorLatency)
        {
            Server.ConnectionQuality = EMGConnectionQuality::Poor;
            Server.bIsRecommended = false;
        }
        else
        {
            Server.ConnectionQuality = EMGConnectionQuality::Critical;
            Server.bIsRecommended = false;
        }
    }
}

void UMGNetworkDiagnosticsSubsystem::AddCustomServer(const FMGServerEndpoint& Server)
{
    // Check if server already exists
    for (const FMGServerEndpoint& Existing : AvailableServers)
    {
        if (Existing.ServerId == Server.ServerId)
        {
            return;
        }
    }

    AvailableServers.Add(Server);
}

void UMGNetworkDiagnosticsSubsystem::RemoveCustomServer(const FString& ServerId)
{
    AvailableServers.RemoveAll([&ServerId](const FMGServerEndpoint& Server)
    {
        return Server.ServerId == ServerId;
    });
}

FMGServerEndpoint UMGNetworkDiagnosticsSubsystem::GetBestServer() const
{
    FMGServerEndpoint BestServer;
    float BestLatency = TNumericLimits<float>::Max();

    for (const FMGServerEndpoint& Server : AvailableServers)
    {
        if (Server.bIsOnline && Server.LatencyMs < BestLatency)
        {
            BestLatency = Server.LatencyMs;
            BestServer = Server;
        }
    }

    return BestServer;
}

TArray<FMGServerEndpoint> UMGNetworkDiagnosticsSubsystem::GetServersByRegion(EMGNetworkRegion Region) const
{
    TArray<FMGServerEndpoint> RegionServers;

    for (const FMGServerEndpoint& Server : AvailableServers)
    {
        if (Server.Region == Region)
        {
            RegionServers.Add(Server);
        }
    }

    // Sort by latency
    RegionServers.Sort([](const FMGServerEndpoint& A, const FMGServerEndpoint& B)
    {
        return A.LatencyMs < B.LatencyMs;
    });

    return RegionServers;
}

void UMGNetworkDiagnosticsSubsystem::ApplyNetworkConfig(const FMGNetworkConfig& Config)
{
    NetworkConfig = Config;

    // Restart monitoring with new settings if active
    if (bIsMonitoring)
    {
        StopMonitoring();
        StartMonitoring();
    }

    // Apply region preference
    if (Config.PreferredRegion == EMGNetworkRegion::Auto)
    {
        SwitchToOptimalRegion();
    }
    else
    {
        SetPreferredRegion(Config.PreferredRegion);
    }
}

void UMGNetworkDiagnosticsSubsystem::SetQualityThresholds(const FMGNetworkQualityThresholds& Thresholds)
{
    NetworkConfig.QualityThresholds = Thresholds;
    UpdateConnectionQuality();
}

void UMGNetworkDiagnosticsSubsystem::SetPingSampleInterval(float IntervalSeconds)
{
    NetworkConfig.PingSampleInterval = FMath::Clamp(IntervalSeconds, 0.1f, 10.0f);

    if (bIsMonitoring)
    {
        // Restart timer with new interval
        if (UWorld* World = GetWorld())
        {
            TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
            World->GetTimerManager().ClearTimer(PingTimerHandle);
            World->GetTimerManager().SetTimer(
                PingTimerHandle,
                [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        WeakThis->SimulatePing();
                    }
                },
                NetworkConfig.PingSampleInterval,
                true
            );
        }
    }
}

void UMGNetworkDiagnosticsSubsystem::SetAutoReconnect(bool bEnabled)
{
    NetworkConfig.bAutoReconnect = bEnabled;
}

bool UMGNetworkDiagnosticsSubsystem::HasActiveIssue(EMGNetworkIssue Issue) const
{
    return CurrentHealth.ActiveIssues.Contains(Issue);
}

void UMGNetworkDiagnosticsSubsystem::ClearNetworkEventLog()
{
    NetworkEventLog.Empty();
}

FString UMGNetworkDiagnosticsSubsystem::GenerateNetworkReport() const
{
    FString Report;

    Report += TEXT("=== MIDNIGHT GRIND NETWORK REPORT ===\n\n");
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

    Report += TEXT("CONNECTION STATUS\n");
    Report += TEXT("-----------------\n");
    Report += FString::Printf(TEXT("Quality: %s\n"), *GetConnectionQualityDisplayString());
    Report += FString::Printf(TEXT("Connected: %s\n"), CurrentHealth.bIsConnected ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("NAT Type: %s\n\n"), *UEnum::GetDisplayValueAsText(CurrentHealth.NATType).ToString());

    Report += TEXT("LATENCY\n");
    Report += TEXT("-------\n");
    Report += FString::Printf(TEXT("Current: %.1f ms\n"), CurrentHealth.LatencyStats.CurrentLatencyMs);
    Report += FString::Printf(TEXT("Average: %.1f ms\n"), CurrentHealth.LatencyStats.AverageLatencyMs);
    Report += FString::Printf(TEXT("Min/Max: %.1f / %.1f ms\n"), CurrentHealth.LatencyStats.MinLatencyMs, CurrentHealth.LatencyStats.MaxLatencyMs);
    Report += FString::Printf(TEXT("Jitter: %.1f ms\n\n"), CurrentHealth.LatencyStats.JitterMs);

    Report += TEXT("PACKET LOSS\n");
    Report += TEXT("-----------\n");
    Report += FString::Printf(TEXT("Loss Rate: %.2f%%\n"), CurrentHealth.PacketLossStats.LossPercentage);
    Report += FString::Printf(TEXT("Packets Sent: %d\n"), CurrentHealth.PacketLossStats.PacketsSent);
    Report += FString::Printf(TEXT("Packets Lost: %d\n\n"), CurrentHealth.PacketLossStats.PacketsLost);

    if (CurrentHealth.BandwidthStats.bBandwidthTestComplete)
    {
        Report += TEXT("BANDWIDTH\n");
        Report += TEXT("---------\n");
        Report += FString::Printf(TEXT("Download: %.1f Mbps\n"), CurrentHealth.BandwidthStats.DownloadSpeedMbps);
        Report += FString::Printf(TEXT("Upload: %.1f Mbps\n\n"), CurrentHealth.BandwidthStats.UploadSpeedMbps);
    }

    if (CurrentHealth.ActiveIssues.Num() > 0)
    {
        Report += TEXT("ACTIVE ISSUES\n");
        Report += TEXT("-------------\n");
        for (EMGNetworkIssue Issue : CurrentHealth.ActiveIssues)
        {
            Report += FString::Printf(TEXT("- %s\n"), *UEnum::GetDisplayValueAsText(Issue).ToString());
        }
        Report += TEXT("\n");
    }

    Report += TEXT("CURRENT SERVER\n");
    Report += TEXT("--------------\n");
    Report += FString::Printf(TEXT("ID: %s\n"), *CurrentServer.ServerId);
    Report += FString::Printf(TEXT("Region: %s\n"), *CurrentServer.RegionName);
    Report += FString::Printf(TEXT("Address: %s:%d\n"), *CurrentServer.Address, CurrentServer.Port);

    return Report;
}

void UMGNetworkDiagnosticsSubsystem::ExportDiagnosticReport(const FString& FilePath)
{
    FString Report = GenerateNetworkReport();
    FFileHelper::SaveStringToFile(Report, *FilePath);
    LogNetworkEvent(EMGNetworkIssue::None, FString::Printf(TEXT("Diagnostic report exported to %s"), *FilePath));
}

void UMGNetworkDiagnosticsSubsystem::CopyDiagnosticToClipboard()
{
    FString Report = GenerateNetworkReport();
    FPlatformApplicationMisc::ClipboardCopy(*Report);
    LogNetworkEvent(EMGNetworkIssue::None, TEXT("Diagnostic report copied to clipboard"));
}

FString UMGNetworkDiagnosticsSubsystem::GetConnectionQualityDisplayString() const
{
    switch (CurrentHealth.OverallQuality)
    {
    case EMGConnectionQuality::Excellent:
        return TEXT("Excellent");
    case EMGConnectionQuality::Good:
        return TEXT("Good");
    case EMGConnectionQuality::Fair:
        return TEXT("Fair");
    case EMGConnectionQuality::Poor:
        return TEXT("Poor");
    case EMGConnectionQuality::Critical:
        return TEXT("Critical");
    case EMGConnectionQuality::Disconnected:
        return TEXT("Disconnected");
    default:
        return TEXT("Unknown");
    }
}

FLinearColor UMGNetworkDiagnosticsSubsystem::GetConnectionQualityColor() const
{
    switch (CurrentHealth.OverallQuality)
    {
    case EMGConnectionQuality::Excellent:
        return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
    case EMGConnectionQuality::Good:
        return FLinearColor(0.5f, 1.0f, 0.0f, 1.0f); // Yellow-Green
    case EMGConnectionQuality::Fair:
        return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    case EMGConnectionQuality::Poor:
        return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
    case EMGConnectionQuality::Critical:
        return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
    case EMGConnectionQuality::Disconnected:
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray
    default:
        return FLinearColor::White;
    }
}

FString UMGNetworkDiagnosticsSubsystem::FormatLatency(float LatencyMs) const
{
    if (LatencyMs < 1.0f)
    {
        return TEXT("<1 ms");
    }
    else if (LatencyMs >= 1000.0f)
    {
        return FString::Printf(TEXT("%.1f s"), LatencyMs / 1000.0f);
    }
    else
    {
        return FString::Printf(TEXT("%.0f ms"), LatencyMs);
    }
}

FString UMGNetworkDiagnosticsSubsystem::GetRecommendationsForIssue(EMGNetworkIssue Issue) const
{
    switch (Issue)
    {
    case EMGNetworkIssue::HighLatency:
        return TEXT("Try: 1) Select closer server region 2) Use wired connection 3) Close bandwidth-heavy apps 4) Restart router");
    case EMGNetworkIssue::PacketLoss:
        return TEXT("Try: 1) Check network cables 2) Restart router/modem 3) Update network drivers 4) Contact ISP");
    case EMGNetworkIssue::Jitter:
        return TEXT("Try: 1) Reduce network congestion 2) Use QoS settings 3) Avoid peak usage times 4) Switch to wired");
    case EMGNetworkIssue::Bandwidth:
        return TEXT("Try: 1) Close streaming apps 2) Check for downloads 3) Reduce video quality 4) Upgrade internet plan");
    case EMGNetworkIssue::NATIssue:
        return TEXT("Try: 1) Enable UPnP 2) Configure port forwarding 3) Contact ISP about CGNAT 4) Try DMZ mode");
    case EMGNetworkIssue::ServerUnreachable:
        return TEXT("Try: 1) Check server status 2) Try different region 3) Verify firewall settings 4) Wait and retry");
    case EMGNetworkIssue::Timeout:
        return TEXT("Try: 1) Check internet connection 2) Restart game 3) Clear DNS cache 4) Restart router");
    default:
        return TEXT("Check your network connection and try again.");
    }
}

void UMGNetworkDiagnosticsSubsystem::UpdateLatencyStats()
{
    if (LatencyHistory.Num() == 0)
    {
        return;
    }

    // Calculate statistics
    float Sum = 0.0f;
    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    float JitterSum = 0.0f;

    for (const FMGLatencySample& Sample : LatencyHistory)
    {
        Sum += Sample.LatencyMs;
        JitterSum += Sample.JitterMs;

        if (Sample.LatencyMs < Min) Min = Sample.LatencyMs;
        if (Sample.LatencyMs > Max) Max = Sample.LatencyMs;
    }

    float Average = Sum / LatencyHistory.Num();
    float AverageJitter = JitterSum / LatencyHistory.Num();

    // Calculate standard deviation
    float VarianceSum = 0.0f;
    for (const FMGLatencySample& Sample : LatencyHistory)
    {
        float Diff = Sample.LatencyMs - Average;
        VarianceSum += Diff * Diff;
    }
    float StdDev = FMath::Sqrt(VarianceSum / LatencyHistory.Num());

    // Calculate percentiles (95th and 99th)
    TArray<float> SortedLatencies;
    for (const FMGLatencySample& Sample : LatencyHistory)
    {
        SortedLatencies.Add(Sample.LatencyMs);
    }
    SortedLatencies.Sort();

    int32 P95Index = FMath::FloorToInt(SortedLatencies.Num() * 0.95f);
    int32 P99Index = FMath::FloorToInt(SortedLatencies.Num() * 0.99f);

    P95Index = FMath::Clamp(P95Index, 0, SortedLatencies.Num() - 1);
    P99Index = FMath::Clamp(P99Index, 0, SortedLatencies.Num() - 1);

    // Update stats
    CurrentHealth.LatencyStats.CurrentLatencyMs = LatencyHistory.Last().LatencyMs;
    CurrentHealth.LatencyStats.AverageLatencyMs = Average;
    CurrentHealth.LatencyStats.MinLatencyMs = Min;
    CurrentHealth.LatencyStats.MaxLatencyMs = Max;
    CurrentHealth.LatencyStats.JitterMs = AverageJitter;
    CurrentHealth.LatencyStats.StandardDeviation = StdDev;
    CurrentHealth.LatencyStats.Percentile95Ms = SortedLatencies[P95Index];
    CurrentHealth.LatencyStats.Percentile99Ms = SortedLatencies[P99Index];
    CurrentHealth.LatencyStats.SampleCount = LatencyHistory.Num();
}

void UMGNetworkDiagnosticsSubsystem::UpdatePacketLossStats()
{
    if (CurrentHealth.PacketLossStats.PacketsSent > 0)
    {
        CurrentHealth.PacketLossStats.LossPercentage =
            (static_cast<float>(CurrentHealth.PacketLossStats.PacketsLost) /
             static_cast<float>(CurrentHealth.PacketLossStats.PacketsSent)) * 100.0f;
    }

    CurrentHealth.PacketLossStats.MeasurementDurationSeconds =
        (FDateTime::Now() - CurrentHealth.PacketLossStats.MeasurementStartTime).GetTotalSeconds();

    // Calculate average burst length
    if (CurrentHealth.PacketLossStats.PacketsLost > 0 && CurrentLossBurstLength > 0)
    {
        // Approximate
        CurrentHealth.PacketLossStats.AverageLossBurstLength =
            static_cast<float>(CurrentHealth.PacketLossStats.PacketsLost) /
            FMath::Max(1.0f, static_cast<float>(CurrentHealth.PacketLossStats.MaxLossBurstLength));
    }
}

void UMGNetworkDiagnosticsSubsystem::UpdateConnectionQuality()
{
    EMGConnectionQuality OldQuality = CurrentHealth.OverallQuality;
    CurrentHealth.OverallQuality = CalculateQualityFromStats();

    if (OldQuality != CurrentHealth.OverallQuality)
    {
        OnConnectionQualityChanged.Broadcast(OldQuality, CurrentHealth.OverallQuality);
        LogNetworkEvent(EMGNetworkIssue::None, FString::Printf(TEXT("Connection quality changed: %s -> %s"),
            *UEnum::GetDisplayValueAsText(OldQuality).ToString(),
            *UEnum::GetDisplayValueAsText(CurrentHealth.OverallQuality).ToString()));
    }

    UpdateQualityScore();
}

void UMGNetworkDiagnosticsSubsystem::UpdateQualityScore()
{
    // Calculate quality score (0-100)
    float LatencyScore = 100.0f;
    float PacketLossScore = 100.0f;
    float JitterScore = 100.0f;

    // Latency scoring
    if (CurrentHealth.LatencyStats.AverageLatencyMs > NetworkConfig.QualityThresholds.PoorLatency)
    {
        LatencyScore = 20.0f;
    }
    else if (CurrentHealth.LatencyStats.AverageLatencyMs > NetworkConfig.QualityThresholds.FairLatency)
    {
        LatencyScore = 40.0f;
    }
    else if (CurrentHealth.LatencyStats.AverageLatencyMs > NetworkConfig.QualityThresholds.GoodLatency)
    {
        LatencyScore = 60.0f;
    }
    else if (CurrentHealth.LatencyStats.AverageLatencyMs > NetworkConfig.QualityThresholds.ExcellentLatency)
    {
        LatencyScore = 80.0f;
    }

    // Packet loss scoring
    if (CurrentHealth.PacketLossStats.LossPercentage > NetworkConfig.QualityThresholds.PoorPacketLoss)
    {
        PacketLossScore = 20.0f;
    }
    else if (CurrentHealth.PacketLossStats.LossPercentage > NetworkConfig.QualityThresholds.FairPacketLoss)
    {
        PacketLossScore = 40.0f;
    }
    else if (CurrentHealth.PacketLossStats.LossPercentage > NetworkConfig.QualityThresholds.GoodPacketLoss)
    {
        PacketLossScore = 60.0f;
    }
    else if (CurrentHealth.PacketLossStats.LossPercentage > NetworkConfig.QualityThresholds.ExcellentPacketLoss)
    {
        PacketLossScore = 80.0f;
    }

    // Jitter scoring
    if (CurrentHealth.LatencyStats.JitterMs > NetworkConfig.QualityThresholds.PoorJitter)
    {
        JitterScore = 20.0f;
    }
    else if (CurrentHealth.LatencyStats.JitterMs > NetworkConfig.QualityThresholds.FairJitter)
    {
        JitterScore = 40.0f;
    }
    else if (CurrentHealth.LatencyStats.JitterMs > NetworkConfig.QualityThresholds.GoodJitter)
    {
        JitterScore = 60.0f;
    }
    else if (CurrentHealth.LatencyStats.JitterMs > NetworkConfig.QualityThresholds.ExcellentJitter)
    {
        JitterScore = 80.0f;
    }

    // Weighted average (latency is most important for racing)
    CurrentHealth.QualityScore = (LatencyScore * 0.5f) + (PacketLossScore * 0.3f) + (JitterScore * 0.2f);

    // Stability score based on variance
    float Variance = CurrentHealth.LatencyStats.StandardDeviation / FMath::Max(1.0f, CurrentHealth.LatencyStats.AverageLatencyMs);
    CurrentHealth.StabilityScore = FMath::Clamp(100.0f - (Variance * 200.0f), 0.0f, 100.0f);
}

void UMGNetworkDiagnosticsSubsystem::CheckForIssues()
{
    TArray<EMGNetworkIssue> NewIssues;

    // Check latency
    if (CurrentHealth.LatencyStats.AverageLatencyMs > NetworkConfig.QualityThresholds.PoorLatency)
    {
        if (!CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::HighLatency))
        {
            NewIssues.Add(EMGNetworkIssue::HighLatency);
            OnNetworkIssueDetected.Broadcast(EMGNetworkIssue::HighLatency);
            LogNetworkEvent(EMGNetworkIssue::HighLatency, FString::Printf(TEXT("High latency detected: %.1f ms"), CurrentHealth.LatencyStats.AverageLatencyMs));
        }
    }
    else if (CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::HighLatency))
    {
        CurrentHealth.ActiveIssues.Remove(EMGNetworkIssue::HighLatency);
        OnNetworkIssueResolved.Broadcast(EMGNetworkIssue::HighLatency);
    }

    // Check packet loss
    if (CurrentHealth.PacketLossStats.LossPercentage > NetworkConfig.QualityThresholds.PoorPacketLoss)
    {
        if (!CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::PacketLoss))
        {
            NewIssues.Add(EMGNetworkIssue::PacketLoss);
            OnNetworkIssueDetected.Broadcast(EMGNetworkIssue::PacketLoss);
            LogNetworkEvent(EMGNetworkIssue::PacketLoss, FString::Printf(TEXT("Packet loss detected: %.2f%%"), CurrentHealth.PacketLossStats.LossPercentage));
        }
    }
    else if (CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::PacketLoss))
    {
        CurrentHealth.ActiveIssues.Remove(EMGNetworkIssue::PacketLoss);
        OnNetworkIssueResolved.Broadcast(EMGNetworkIssue::PacketLoss);
    }

    // Check jitter
    if (CurrentHealth.LatencyStats.JitterMs > NetworkConfig.QualityThresholds.PoorJitter)
    {
        if (!CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::Jitter))
        {
            NewIssues.Add(EMGNetworkIssue::Jitter);
            OnNetworkIssueDetected.Broadcast(EMGNetworkIssue::Jitter);
            LogNetworkEvent(EMGNetworkIssue::Jitter, FString::Printf(TEXT("High jitter detected: %.1f ms"), CurrentHealth.LatencyStats.JitterMs));
        }
    }
    else if (CurrentHealth.ActiveIssues.Contains(EMGNetworkIssue::Jitter))
    {
        CurrentHealth.ActiveIssues.Remove(EMGNetworkIssue::Jitter);
        OnNetworkIssueResolved.Broadcast(EMGNetworkIssue::Jitter);
    }

    // Add new issues
    for (EMGNetworkIssue Issue : NewIssues)
    {
        if (!CurrentHealth.ActiveIssues.Contains(Issue))
        {
            CurrentHealth.ActiveIssues.Add(Issue);
        }
    }
}

void UMGNetworkDiagnosticsSubsystem::LogNetworkEvent(EMGNetworkIssue Issue, const FString& Description)
{
    FMGNetworkEvent Event;
    Event.EventId = FGuid::NewGuid();
    Event.Timestamp = FDateTime::Now();
    Event.IssueType = Issue;
    Event.Description = Description;
    Event.LatencyAtEvent = CurrentHealth.LatencyStats.CurrentLatencyMs;
    Event.PacketLossAtEvent = CurrentHealth.PacketLossStats.LossPercentage;

    NetworkEventLog.Add(Event);

    // Limit log size
    while (NetworkEventLog.Num() > 1000)
    {
        NetworkEventLog.RemoveAt(0);
    }
}

void UMGNetworkDiagnosticsSubsystem::AttemptReconnect()
{
    if (!NetworkConfig.bAutoReconnect)
    {
        return;
    }

    if (CurrentHealth.ReconnectAttempts >= NetworkConfig.MaxReconnectAttempts)
    {
        LogNetworkEvent(EMGNetworkIssue::ServerUnreachable, TEXT("Max reconnect attempts reached"));
        return;
    }

    CurrentHealth.ReconnectAttempts++;
    float Delay = NetworkConfig.ReconnectDelaySeconds * FMath::Pow(NetworkConfig.ReconnectBackoffMultiplier, CurrentHealth.ReconnectAttempts - 1);

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGNetworkDiagnosticsSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            ReconnectHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    // Simulate reconnection attempt
                    bool bSuccess = FMath::RandRange(0, 100) > 30; // 70% success rate

                    if (bSuccess)
                    {
                        WeakThis->CurrentHealth.bIsConnected = true;
                        WeakThis->CurrentHealth.LastConnectedTime = FDateTime::Now();
                        WeakThis->CurrentHealth.ReconnectAttempts = 0;
                        WeakThis->OnConnectionRestored.Broadcast();
                        WeakThis->LogNetworkEvent(EMGNetworkIssue::None, TEXT("Connection restored"));
                    }
                    else
                    {
                        WeakThis->AttemptReconnect();
                    }
                }
            },
            Delay,
            false
        );
    }

    LogNetworkEvent(EMGNetworkIssue::Timeout, FString::Printf(TEXT("Reconnect attempt %d/%d in %.1f seconds"),
        CurrentHealth.ReconnectAttempts, NetworkConfig.MaxReconnectAttempts, Delay));
}

void UMGNetworkDiagnosticsSubsystem::ProcessDiagnosticQueue()
{
    if (bDiagnosticRunning || PendingDiagnosticTests.Num() == 0)
    {
        return;
    }

    EMGDiagnosticTest NextTest = PendingDiagnosticTests[0];
    PendingDiagnosticTests.RemoveAt(0);
    RunDiagnosticTest(NextTest);
}

void UMGNetworkDiagnosticsSubsystem::SimulatePing()
{
    if (!bIsMonitoring)
    {
        return;
    }

    // Simulate network conditions with some variance
    float BaseLatency = CurrentServer.LatencyMs > 0.0f ? CurrentServer.LatencyMs : 50.0f;
    float Variance = FMath::RandRange(-10.0f, 20.0f);
    float SimulatedLatency = FMath::Max(5.0f, BaseLatency + Variance);

    // Occasional spike
    if (FMath::RandRange(0, 100) < 5)
    {
        SimulatedLatency += FMath::RandRange(50.0f, 150.0f);
    }

    // Occasional packet loss
    if (FMath::RandRange(0, 100) < 2)
    {
        RecordPacketLost();
    }
    else
    {
        RecordPacketSent();
        RecordPacketReceived(CurrentHealth.PacketLossStats.PacketsSent);
    }

    RecordLatencySample(SimulatedLatency, CurrentServer.Address);
}

void UMGNetworkDiagnosticsSubsystem::SimulateBandwidthTest()
{
    if (!bBandwidthTestRunning)
    {
        return;
    }

    // Simulate bandwidth measurement
    static float AccumulatedDownload = 0.0f;
    static float AccumulatedUpload = 0.0f;

    float DownloadSample = FMath::RandRange(50.0f, 150.0f);
    float UploadSample = FMath::RandRange(10.0f, 30.0f);

    AccumulatedDownload = (AccumulatedDownload * 0.9f) + (DownloadSample * 0.1f);
    AccumulatedUpload = (AccumulatedUpload * 0.9f) + (UploadSample * 0.1f);

    CurrentHealth.BandwidthStats.DownloadSpeedMbps = AccumulatedDownload;
    CurrentHealth.BandwidthStats.UploadSpeedMbps = AccumulatedUpload;
}

EMGConnectionQuality UMGNetworkDiagnosticsSubsystem::CalculateQualityFromStats() const
{
    if (!CurrentHealth.bIsConnected)
    {
        return EMGConnectionQuality::Disconnected;
    }

    float Latency = CurrentHealth.LatencyStats.AverageLatencyMs;
    float PacketLoss = CurrentHealth.PacketLossStats.LossPercentage;
    float Jitter = CurrentHealth.LatencyStats.JitterMs;

    // Determine quality based on worst metric
    if (Latency <= NetworkConfig.QualityThresholds.ExcellentLatency &&
        PacketLoss <= NetworkConfig.QualityThresholds.ExcellentPacketLoss &&
        Jitter <= NetworkConfig.QualityThresholds.ExcellentJitter)
    {
        return EMGConnectionQuality::Excellent;
    }
    else if (Latency <= NetworkConfig.QualityThresholds.GoodLatency &&
             PacketLoss <= NetworkConfig.QualityThresholds.GoodPacketLoss &&
             Jitter <= NetworkConfig.QualityThresholds.GoodJitter)
    {
        return EMGConnectionQuality::Good;
    }
    else if (Latency <= NetworkConfig.QualityThresholds.FairLatency &&
             PacketLoss <= NetworkConfig.QualityThresholds.FairPacketLoss &&
             Jitter <= NetworkConfig.QualityThresholds.FairJitter)
    {
        return EMGConnectionQuality::Fair;
    }
    else if (Latency <= NetworkConfig.QualityThresholds.PoorLatency &&
             PacketLoss <= NetworkConfig.QualityThresholds.PoorPacketLoss &&
             Jitter <= NetworkConfig.QualityThresholds.PoorJitter)
    {
        return EMGConnectionQuality::Poor;
    }
    else
    {
        return EMGConnectionQuality::Critical;
    }
}
