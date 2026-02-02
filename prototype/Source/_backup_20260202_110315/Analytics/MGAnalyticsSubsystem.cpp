// Copyright Midnight Grind. All Rights Reserved.

#include "Analytics/MGAnalyticsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/App.h"
#include "HAL/PlatformMisc.h"

void UMGAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PlayerID = FPlatformMisc::GetDeviceId();
	StartSession();

	// Start timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BatchUploadTimerHandle,
			this,
			&UMGAnalyticsSubsystem::UploadPendingEvents,
			BatchUploadInterval,
			true
		);

		World->GetTimerManager().SetTimer(
			PerformanceSampleTimerHandle,
			this,
			&UMGAnalyticsSubsystem::SamplePerformanceMetrics,
			PerformanceSampleInterval,
			true
		);
	}
}

void UMGAnalyticsSubsystem::Deinitialize()
{
	EndSession();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatchUploadTimerHandle);
		World->GetTimerManager().ClearTimer(PerformanceSampleTimerHandle);
	}

	// Final upload
	UploadPendingEvents();

	Super::Deinitialize();
}

// ==========================================
// EVENT TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackEvent(const FString& EventName, EMGAnalyticsCategory Category)
{
	FMGAnalyticsEvent Event;
	Event.EventName = EventName;
	Event.Category = Category;
	Event.Timestamp = FDateTime::Now();
	Event.SessionID = CurrentSession.SessionID;
	Event.PlayerID = PlayerID;

	QueueEvent(Event);
}

void UMGAnalyticsSubsystem::TrackEventWithProperties(const FString& EventName, EMGAnalyticsCategory Category,
	const TMap<FString, FString>& Properties)
{
	FMGAnalyticsEvent Event;
	Event.EventName = EventName;
	Event.Category = Category;
	Event.Properties = Properties;
	Event.Timestamp = FDateTime::Now();
	Event.SessionID = CurrentSession.SessionID;
	Event.PlayerID = PlayerID;

	QueueEvent(Event);
}

void UMGAnalyticsSubsystem::TrackEventWithMetrics(const FString& EventName, EMGAnalyticsCategory Category,
	const TMap<FString, float>& Metrics)
{
	FMGAnalyticsEvent Event;
	Event.EventName = EventName;
	Event.Category = Category;
	Event.Metrics = Metrics;
	Event.Timestamp = FDateTime::Now();
	Event.SessionID = CurrentSession.SessionID;
	Event.PlayerID = PlayerID;

	QueueEvent(Event);
}

void UMGAnalyticsSubsystem::TrackFullEvent(const FMGAnalyticsEvent& Event)
{
	FMGAnalyticsEvent ModifiedEvent = Event;
	ModifiedEvent.Timestamp = FDateTime::Now();
	ModifiedEvent.SessionID = CurrentSession.SessionID;
	ModifiedEvent.PlayerID = PlayerID;

	QueueEvent(ModifiedEvent);
}

// ==========================================
// GAMEPLAY TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackRaceStart(FName TrackID, FName VehicleID, bool bIsOnline, int32 RacerCount)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("TrackID"), TrackID.ToString());
	Properties.Add(TEXT("VehicleID"), VehicleID.ToString());
	Properties.Add(TEXT("IsOnline"), bIsOnline ? TEXT("true") : TEXT("false"));

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("RacerCount"), (float)RacerCount);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("RaceStart");
	Event.Category = EMGAnalyticsCategory::Gameplay;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);
}

void UMGAnalyticsSubsystem::TrackRaceEnd(const FMGRaceAnalytics& RaceData)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("TrackID"), RaceData.TrackID.ToString());
	Properties.Add(TEXT("VehicleID"), RaceData.VehicleID.ToString());
	Properties.Add(TEXT("IsOnline"), RaceData.bIsOnline ? TEXT("true") : TEXT("false"));
	Properties.Add(TEXT("RageQuit"), RaceData.bRageQuit ? TEXT("true") : TEXT("false"));

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("FinalPosition"), (float)RaceData.FinalPosition);
	Metrics.Add(TEXT("RaceTime"), RaceData.RaceTime);
	Metrics.Add(TEXT("BestLapTime"), RaceData.BestLapTime);
	Metrics.Add(TEXT("Collisions"), (float)RaceData.Collisions);
	Metrics.Add(TEXT("DriftDistance"), RaceData.DriftDistance);
	Metrics.Add(TEXT("NitroUses"), (float)RaceData.NitroUses);
	Metrics.Add(TEXT("Overtakes"), (float)RaceData.Overtakes);
	Metrics.Add(TEXT("RacerCount"), (float)RaceData.RacerCount);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("RaceEnd");
	Event.Category = EMGAnalyticsCategory::Gameplay;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);

	// Update session data
	CurrentSession.RacesCompleted++;
	if (RaceData.FinalPosition == 1)
	{
		CurrentSession.RacesWon++;
	}

	// Update vehicle balance stats
	UpdateVehicleBalanceStats(RaceData.VehicleID, RaceData.FinalPosition, RaceData.BestLapTime);
}

void UMGAnalyticsSubsystem::TrackCrash(FVector Location, FName TrackID, float Speed)
{
	AddHeatMapPoint(TrackID, FName(TEXT("Crash")), Location, Speed / 100.0f);

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("Speed"), Speed);
	Metrics.Add(TEXT("LocationX"), Location.X);
	Metrics.Add(TEXT("LocationY"), Location.Y);
	Metrics.Add(TEXT("LocationZ"), Location.Z);

	TrackEventWithMetrics(TEXT("Crash"), EMGAnalyticsCategory::Gameplay, Metrics);
}

void UMGAnalyticsSubsystem::TrackOvertake(FVector Location, FName TrackID)
{
	AddHeatMapPoint(TrackID, FName(TEXT("Overtake")), Location, 1.0f);
	TrackEvent(TEXT("Overtake"), EMGAnalyticsCategory::Gameplay);
}

void UMGAnalyticsSubsystem::TrackDrift(FVector Location, FName TrackID, float DriftScore)
{
	AddHeatMapPoint(TrackID, FName(TEXT("Drift")), Location, DriftScore / 1000.0f);

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("DriftScore"), DriftScore);

	TrackEventWithMetrics(TEXT("Drift"), EMGAnalyticsCategory::Gameplay, Metrics);
}

void UMGAnalyticsSubsystem::TrackNitroUse(FVector Location, FName TrackID)
{
	AddHeatMapPoint(TrackID, FName(TEXT("Nitro")), Location, 1.0f);
	TrackEvent(TEXT("NitroUse"), EMGAnalyticsCategory::Gameplay);
}

// ==========================================
// ECONOMY TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackCurrencyEarned(const FString& CurrencyType, int32 Amount, const FString& Source)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("CurrencyType"), CurrencyType);
	Properties.Add(TEXT("Source"), Source);

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("Amount"), (float)Amount);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("CurrencyEarned");
	Event.Category = EMGAnalyticsCategory::Economy;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);

	if (CurrencyType == TEXT("Cash"))
	{
		CurrentSession.CashEarned += Amount;
	}
	else if (CurrencyType == TEXT("XP"))
	{
		CurrentSession.XPEarned += Amount;
	}
}

void UMGAnalyticsSubsystem::TrackCurrencySpent(const FString& CurrencyType, int32 Amount, const FString& ItemType, FName ItemID)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("CurrencyType"), CurrencyType);
	Properties.Add(TEXT("ItemType"), ItemType);
	Properties.Add(TEXT("ItemID"), ItemID.ToString());

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("Amount"), (float)Amount);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("CurrencySpent");
	Event.Category = EMGAnalyticsCategory::Economy;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);

	if (CurrencyType == TEXT("Cash"))
	{
		CurrentSession.CashSpent += Amount;
	}
}

void UMGAnalyticsSubsystem::TrackPurchase(FName ItemID, const FString& ItemType, int32 Price, const FString& CurrencyType)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("ItemID"), ItemID.ToString());
	Properties.Add(TEXT("ItemType"), ItemType);
	Properties.Add(TEXT("CurrencyType"), CurrencyType);

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("Price"), (float)Price);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("Purchase");
	Event.Category = EMGAnalyticsCategory::Economy;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);

	if (ItemType == TEXT("Vehicle"))
	{
		CurrentSession.VehiclesPurchased++;
	}
}

// ==========================================
// PROGRESSION TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackLevelUp(int32 NewLevel, float TotalPlayTime)
{
	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("NewLevel"), (float)NewLevel);
	Metrics.Add(TEXT("TotalPlayTime"), TotalPlayTime);

	TrackEventWithMetrics(TEXT("LevelUp"), EMGAnalyticsCategory::Progression, Metrics);

	CurrentSession.LevelsGained++;
}

void UMGAnalyticsSubsystem::TrackAchievementUnlocked(FName AchievementID, float TotalPlayTime)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("AchievementID"), AchievementID.ToString());

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("TotalPlayTime"), TotalPlayTime);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("AchievementUnlocked");
	Event.Category = EMGAnalyticsCategory::Progression;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);

	CurrentSession.AchievementsUnlocked++;
}

void UMGAnalyticsSubsystem::TrackTutorialStep(const FString& StepName, bool bCompleted, float TimeSpent)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("StepName"), StepName);
	Properties.Add(TEXT("Completed"), bCompleted ? TEXT("true") : TEXT("false"));

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("TimeSpent"), TimeSpent);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("TutorialStep");
	Event.Category = EMGAnalyticsCategory::Progression;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);
}

void UMGAnalyticsSubsystem::TrackFunnelStep(const FString& FunnelName, const FString& StepName, bool bCompleted)
{
	TArray<FMGFunnelStep>& Steps = FunnelData.FindOrAdd(FunnelName);

	FMGFunnelStep* ExistingStep = Steps.FindByPredicate([&StepName](const FMGFunnelStep& Step) {
		return Step.StepName == StepName;
	});

	if (ExistingStep)
	{
		ExistingStep->UsersReached++;
		if (bCompleted)
		{
			ExistingStep->UsersCompleted++;
		}
		ExistingStep->ConversionRate = (float)ExistingStep->UsersCompleted / (float)ExistingStep->UsersReached;
	}
	else
	{
		FMGFunnelStep NewStep;
		NewStep.StepName = StepName;
		NewStep.UsersReached = 1;
		NewStep.UsersCompleted = bCompleted ? 1 : 0;
		NewStep.ConversionRate = bCompleted ? 1.0f : 0.0f;
		Steps.Add(NewStep);
	}

	TMap<FString, FString> Properties;
	Properties.Add(TEXT("FunnelName"), FunnelName);
	Properties.Add(TEXT("StepName"), StepName);
	Properties.Add(TEXT("Completed"), bCompleted ? TEXT("true") : TEXT("false"));

	TrackEventWithProperties(TEXT("FunnelStep"), EMGAnalyticsCategory::Progression, Properties);
}

// ==========================================
// SOCIAL TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackSocialAction(const FString& ActionType, const FString& Context)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("ActionType"), ActionType);
	Properties.Add(TEXT("Context"), Context);

	TrackEventWithProperties(TEXT("SocialAction"), EMGAnalyticsCategory::Social, Properties);
}

void UMGAnalyticsSubsystem::TrackCrewAction(const FString& ActionType, FName CrewID)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("ActionType"), ActionType);
	Properties.Add(TEXT("CrewID"), CrewID.ToString());

	TrackEventWithProperties(TEXT("CrewAction"), EMGAnalyticsCategory::Social, Properties);
}

// ==========================================
// TECHNICAL TRACKING
// ==========================================

void UMGAnalyticsSubsystem::TrackError(const FString& ErrorType, const FString& ErrorMessage, const FString& StackTrace)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("ErrorType"), ErrorType);
	Properties.Add(TEXT("ErrorMessage"), ErrorMessage);
	Properties.Add(TEXT("StackTrace"), StackTrace.Left(1000)); // Truncate long stack traces

	TrackEventWithProperties(TEXT("Error"), EMGAnalyticsCategory::Error, Properties);

	CurrentSession.CrashCount++;
}

void UMGAnalyticsSubsystem::TrackPerformanceSnapshot(const FMGPerformanceMetrics& Metrics)
{
	TMap<FString, float> MetricMap;
	MetricMap.Add(TEXT("AverageFPS"), Metrics.AverageFPS);
	MetricMap.Add(TEXT("MinFPS"), Metrics.MinFPS);
	MetricMap.Add(TEXT("MaxFPS"), Metrics.MaxFPS);
	MetricMap.Add(TEXT("FrameTime"), Metrics.AverageFrameTime);
	MetricMap.Add(TEXT("GPU_Time"), Metrics.GPU_Time);
	MetricMap.Add(TEXT("CPU_Time"), Metrics.CPU_Time);
	MetricMap.Add(TEXT("MemoryMB"), (float)Metrics.MemoryUsedMB);
	MetricMap.Add(TEXT("DrawCalls"), (float)Metrics.DrawCalls);
	MetricMap.Add(TEXT("HitchCount"), (float)Metrics.HitchCount);

	TrackEventWithMetrics(TEXT("PerformanceSnapshot"), EMGAnalyticsCategory::Technical, MetricMap);

	// Update session average FPS
	if (CurrentSession.AverageFPS == 0.0f)
	{
		CurrentSession.AverageFPS = Metrics.AverageFPS;
	}
	else
	{
		CurrentSession.AverageFPS = (CurrentSession.AverageFPS + Metrics.AverageFPS) / 2.0f;
	}
}

void UMGAnalyticsSubsystem::TrackLoadingTime(const FString& LoadType, float LoadTime)
{
	TMap<FString, FString> Properties;
	Properties.Add(TEXT("LoadType"), LoadType);

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("LoadTime"), LoadTime);

	FMGAnalyticsEvent Event;
	Event.EventName = TEXT("LoadingTime");
	Event.Category = EMGAnalyticsCategory::Technical;
	Event.Properties = Properties;
	Event.Metrics = Metrics;

	QueueEvent(Event);
}

// ==========================================
// HEAT MAPS
// ==========================================

FMGTrackHeatMap UMGAnalyticsSubsystem::GetTrackHeatMap(FName TrackID) const
{
	if (const FMGTrackHeatMap* HeatMap = TrackHeatMaps.Find(TrackID))
	{
		return *HeatMap;
	}
	return FMGTrackHeatMap();
}

void UMGAnalyticsSubsystem::AddHeatMapPoint(FName TrackID, FName EventType, FVector Location, float Intensity)
{
	FMGTrackHeatMap& HeatMap = TrackHeatMaps.FindOrAdd(TrackID);
	HeatMap.TrackID = TrackID;

	FMGHeatMapPoint Point;
	Point.Location = Location;
	Point.Intensity = Intensity;
	Point.EventType = EventType;
	Point.Timestamp = FDateTime::Now();

	if (EventType == FName(TEXT("Crash")))
	{
		HeatMap.CrashPoints.Add(Point);
	}
	else if (EventType == FName(TEXT("Overtake")))
	{
		HeatMap.OvertakePoints.Add(Point);
	}
	else if (EventType == FName(TEXT("Drift")))
	{
		HeatMap.DriftPoints.Add(Point);
	}
	else if (EventType == FName(TEXT("Nitro")))
	{
		HeatMap.NitroPoints.Add(Point);
	}
	else if (EventType == FName(TEXT("Slowdown")))
	{
		HeatMap.SlowdownPoints.Add(Point);
	}
}

void UMGAnalyticsSubsystem::ClearHeatMapData(FName TrackID)
{
	TrackHeatMaps.Remove(TrackID);
}

// ==========================================
// BALANCE DATA
// ==========================================

FMGBalanceAnalytics UMGAnalyticsSubsystem::GetVehicleBalanceData(FName VehicleID) const
{
	if (const FMGBalanceAnalytics* Data = VehicleBalanceData.Find(VehicleID))
	{
		return *Data;
	}
	return FMGBalanceAnalytics();
}

TArray<FMGBalanceAnalytics> UMGAnalyticsSubsystem::GetAllVehicleBalanceData() const
{
	TArray<FMGBalanceAnalytics> Result;
	for (const auto& Pair : VehicleBalanceData)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TMap<int32, float> UMGAnalyticsSubsystem::GetTrackStartPositionWinRates(FName TrackID) const
{
	// Would calculate from stored race data
	TMap<int32, float> WinRates;
	return WinRates;
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGAnalyticsSubsystem::SetAnalyticsEnabled(bool bEnabled)
{
	bAnalyticsEnabled = bEnabled;

	if (!bEnabled)
	{
		PendingEvents.Empty();
	}
}

void UMGAnalyticsSubsystem::SetBatchUploadInterval(float Seconds)
{
	BatchUploadInterval = Seconds;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatchUploadTimerHandle);
		World->GetTimerManager().SetTimer(
			BatchUploadTimerHandle,
			this,
			&UMGAnalyticsSubsystem::UploadPendingEvents,
			BatchUploadInterval,
			true
		);
	}
}

void UMGAnalyticsSubsystem::FlushEvents()
{
	UploadPendingEvents();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAnalyticsSubsystem::StartSession()
{
	CurrentSession = FMGSessionAnalytics();
	CurrentSession.SessionID = GenerateSessionID();
	CurrentSession.SessionStart = FDateTime::Now();
	CurrentSession.Platform = FPlatformProperties::PlatformName();
	CurrentSession.DeviceInfo = GetDeviceInfo();

	TrackEvent(TEXT("SessionStart"), EMGAnalyticsCategory::Engagement);
}

void UMGAnalyticsSubsystem::EndSession()
{
	UpdateSessionDuration();

	TMap<FString, float> Metrics;
	Metrics.Add(TEXT("SessionDuration"), CurrentSession.SessionDuration);
	Metrics.Add(TEXT("RacesCompleted"), (float)CurrentSession.RacesCompleted);
	Metrics.Add(TEXT("RacesWon"), (float)CurrentSession.RacesWon);
	Metrics.Add(TEXT("CashEarned"), (float)CurrentSession.CashEarned);
	Metrics.Add(TEXT("CashSpent"), (float)CurrentSession.CashSpent);
	Metrics.Add(TEXT("XPEarned"), (float)CurrentSession.XPEarned);
	Metrics.Add(TEXT("AverageFPS"), CurrentSession.AverageFPS);

	TrackEventWithMetrics(TEXT("SessionEnd"), EMGAnalyticsCategory::Engagement, Metrics);

	TotalPlayTime += CurrentSession.SessionDuration;
}

void UMGAnalyticsSubsystem::QueueEvent(const FMGAnalyticsEvent& Event)
{
	if (!bAnalyticsEnabled)
	{
		return;
	}

	FMGAnalyticsEvent QueuedEvent = Event;
	QueuedEvent.Timestamp = FDateTime::Now();
	QueuedEvent.SessionID = CurrentSession.SessionID;
	QueuedEvent.PlayerID = PlayerID;

	PendingEvents.Add(QueuedEvent);
	OnAnalyticsEventSent.Broadcast(QueuedEvent);

	// Auto-flush if too many events
	if (PendingEvents.Num() > 100)
	{
		UploadPendingEvents();
	}
}

void UMGAnalyticsSubsystem::UploadPendingEvents()
{
	if (PendingEvents.Num() == 0)
	{
		return;
	}

	// Would send to analytics backend
	// Firebase, GameAnalytics, custom server, etc.

	// For now, just clear the queue
	PendingEvents.Empty();
}

void UMGAnalyticsSubsystem::SamplePerformanceMetrics()
{
	FMGPerformanceMetrics Metrics;

	// Get FPS
	extern ENGINE_API float GAverageFPS;
	Metrics.AverageFPS = GAverageFPS;
	Metrics.AverageFrameTime = GAverageFPS > 0 ? 1000.0f / GAverageFPS : 0.0f;

	// Memory
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	Metrics.MemoryUsedMB = MemStats.UsedPhysical / (1024 * 1024);

	TrackPerformanceSnapshot(Metrics);
}

void UMGAnalyticsSubsystem::UpdateSessionDuration()
{
	CurrentSession.SessionEnd = FDateTime::Now();
	CurrentSession.SessionDuration = (CurrentSession.SessionEnd - CurrentSession.SessionStart).GetTotalSeconds();
}

FString UMGAnalyticsSubsystem::GenerateSessionID() const
{
	return FGuid::NewGuid().ToString();
}

FString UMGAnalyticsSubsystem::GetDeviceInfo() const
{
	return FString::Printf(TEXT("%s|%s|%d cores|%lld MB RAM"),
		*FPlatformProperties::PlatformName(),
		*FPlatformMisc::GetCPUBrand(),
		FPlatformMisc::NumberOfCores(),
		FPlatformMemory::GetStats().TotalPhysical / (1024 * 1024));
}

void UMGAnalyticsSubsystem::UpdateVehicleBalanceStats(FName VehicleID, int32 Position, float LapTime)
{
	FMGBalanceAnalytics& Data = VehicleBalanceData.FindOrAdd(VehicleID);
	Data.VehicleID = VehicleID;
	Data.TimesUsed++;

	if (Position == 1)
	{
		Data.Wins++;
	}

	Data.WinRate = (float)Data.Wins / (float)Data.TimesUsed;

	// Running average for position
	Data.AveragePosition = ((Data.AveragePosition * (Data.TimesUsed - 1)) + Position) / Data.TimesUsed;

	// Running average for lap time
	if (LapTime > 0)
	{
		if (Data.AverageLapTime == 0)
		{
			Data.AverageLapTime = LapTime;
		}
		else
		{
			Data.AverageLapTime = ((Data.AverageLapTime * (Data.TimesUsed - 1)) + LapTime) / Data.TimesUsed;
		}
	}
}
