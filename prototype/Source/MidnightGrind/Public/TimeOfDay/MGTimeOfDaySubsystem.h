// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTimeOfDaySubsystem.generated.h"

/**
 * Time of Day System - Dynamic Lighting
 * - Seamless day/night transitions
 * - Time-synced with server for multiplayer
 * - Affects visibility, traffic, and atmosphere
 * - Special events at certain times (midnight races)
 * - Neon lighting intensity based on time
 */

UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
	Dawn,          // 5:00 - 7:00
	Morning,       // 7:00 - 11:00
	Midday,        // 11:00 - 14:00
	Afternoon,     // 14:00 - 17:00
	Dusk,          // 17:00 - 19:00
	Evening,       // 19:00 - 22:00
	Night,         // 22:00 - 2:00
	LateNight      // 2:00 - 5:00
};

USTRUCT(BlueprintType)
struct FMGTimeOfDaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoonIntensity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkyBrightness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AmbientColor = FLinearColor(0.3f, 0.35f, 0.4f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FogColor = FLinearColor(0.5f, 0.55f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NeonIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreetLightIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrafficDensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PedestrianDensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StarVisibility = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGTimePeriodEvents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDay TimePeriod = EMGTimeOfDay::Night;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableRaceTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMidnightRacesAvailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCopActivityIncreased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashMultiplier = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTimeOfDayChanged, EMGTimeOfDay, OldTime, EMGTimeOfDay, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMidnightReached, int32, GameDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHourChanged, int32, NewHour);

UCLASS()
class MIDNIGHTGRIND_API UMGTimeOfDaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Time Queries
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	float GetCurrentTime() const { return CurrentTimeHours; }

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetCurrentHour() const { return FMath::FloorToInt(CurrentTimeHours); }

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetCurrentMinute() const { return FMath::FloorToInt(FMath::Frac(CurrentTimeHours) * 60.0f); }

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	EMGTimeOfDay GetTimePeriod() const { return CurrentTimePeriod; }

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetTimeString() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	bool IsNightTime() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	bool IsMidnightHour() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetGameDay() const { return GameDay; }

	// Time Control
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTime(float TimeHours);

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTimePeriod(EMGTimeOfDay Period);

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	float GetTimeScale() const { return TimeScale; }

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void PauseTime(bool bPause);

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SyncWithServer(float ServerTimeHours, int32 ServerGameDay);

	// Lighting Settings
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	FMGTimeOfDaySettings GetCurrentSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	float GetSunAngle() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	FRotator GetSunRotation() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	float GetNormalizedTime() const { return CurrentTimeHours / 24.0f; }

	// Period Events
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	FMGTimePeriodEvents GetPeriodEvents() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	float GetCurrentReputationMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	float GetCurrentCashMultiplier() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnMidnightReached OnMidnightReached;

	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnHourChanged OnHourChanged;

protected:
	void UpdateTime(float DeltaTime);
	void UpdateLightingSettings();
	EMGTimeOfDay CalculateTimePeriod(float TimeHours) const;
	FMGTimeOfDaySettings InterpolateSettings(float TimeHours) const;
	void InitializePeriodSettings();

private:
	float CurrentTimeHours = 22.0f; // Start at 10 PM - prime racing time
	float TimeScale = 60.0f; // 1 real second = 1 game minute
	int32 GameDay = 1;
	int32 LastHour = -1;
	EMGTimeOfDay CurrentTimePeriod = EMGTimeOfDay::Night;
	FMGTimeOfDaySettings CurrentSettings;
	TMap<EMGTimeOfDay, FMGTimeOfDaySettings> PeriodSettings;
	TMap<EMGTimeOfDay, FMGTimePeriodEvents> PeriodEvents;
	FTimerHandle TimeUpdateHandle;
	bool bTimePaused = false;
};
