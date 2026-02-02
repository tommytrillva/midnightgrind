
// Generated Performance Profiling Code for Midnight Grind Subsystems
// Include this in your subsystem base class or game instance

#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemProfiler, Log, All);
DEFINE_LOG_CATEGORY(LogSubsystemProfiler);

class MIDNIGHTGRIND_API FSubsystemProfiler
{
public:
    struct FSubsystemTimingData
    {
        FString SubsystemName;
        double InitializationTime;
        double PostInitializationTime;
        int32 DependencyCount;
        FDateTime Timestamp;
    };
    
    static TArray<FSubsystemTimingData> TimingData;
    
    static void BeginSubsystemTiming(const FString& SubsystemName)
    {
        FSubsystemTimingData& Data = TimingData.AddDefaulted_GetRef();
        Data.SubsystemName = SubsystemName;
        Data.Timestamp = FDateTime::Now();
        
        UE_LOG(LogSubsystemProfiler, Log, TEXT("Starting initialization: %s"), *SubsystemName);
    }
    
    static void EndSubsystemTiming(const FString& SubsystemName, double InitTime, double PostInitTime = 0.0)
    {
        for (auto& Data : TimingData)
        {
            if (Data.SubsystemName == SubsystemName)
            {
                Data.InitializationTime = InitTime;
                Data.PostInitializationTime = PostInitTime;
                
                UE_LOG(LogSubsystemProfiler, Log, TEXT("Completed initialization: %s (%.2fms init, %.2fms post)"), 
                       *SubsystemName, InitTime, PostInitTime);
                break;
            }
        }
    }
    
    static void DumpTimingReport()
    {
        UE_LOG(LogSubsystemProfiler, Warning, TEXT("=== SUBSYSTEM TIMING REPORT ==="));
        
        double TotalTime = 0.0;
        for (const auto& Data : TimingData)
        {
            double SubsystemTotal = Data.InitializationTime + Data.PostInitializationTime;
            TotalTime += SubsystemTotal;
            
            UE_LOG(LogSubsystemProfiler, Warning, TEXT("%s: %.2fms (init: %.2fms, post: %.2fms)"),
                   *Data.SubsystemName, SubsystemTotal, Data.InitializationTime, Data.PostInitializationTime);
        }
        
        UE_LOG(LogSubsystemProfiler, Warning, TEXT("Total subsystem initialization time: %.2fms"), TotalTime);
        UE_LOG(LogSubsystemProfiler, Warning, TEXT("================================"));
    }
};

TArray<FSubsystemProfiler::FSubsystemTimingData> FSubsystemProfiler::TimingData;

// Macro for easy profiling
#define PROFILE_SUBSYSTEM_INIT(SubsystemName) \
    FSubsystemProfiler::BeginSubsystemTiming(TEXT(#SubsystemName)); \
    double StartTime = FPlatformTime::Seconds();

#define PROFILE_SUBSYSTEM_END(SubsystemName) \
    double EndTime = FPlatformTime::Seconds(); \
    FSubsystemProfiler::EndSubsystemTiming(TEXT(#SubsystemName), (EndTime - StartTime) * 1000.0);

// Usage example in subsystem:
// void UMySubsystem::Initialize(FSubsystemCollectionBase& Collection)
// {
//     PROFILE_SUBSYSTEM_INIT(MySubsystem);
//     
//     // Your initialization code here
//     Super::Initialize(Collection);
//     
//     PROFILE_SUBSYSTEM_END(MySubsystem);
// }
