// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGRaceHUDWidget.h - Abstract Base Class for Race HUD Widgets
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the INTERFACE (contract) that all race HUD implementations
 * must follow. It's marked as "Abstract," meaning you can't use it directly -
 * you must create a child class that implements the required methods.
 *
 * Think of it as a blueprint (in the design sense, not Unreal Blueprint) that
 * says "any racing HUD must be able to show speed, position, lap count, etc."
 * The actual visual design can vary wildly between implementations.
 *
 * WHY USE AN ABSTRACT BASE CLASS?
 * --------------------------------
 * 1. CONSISTENCY: All HUD variations support the same features
 * 2. FLEXIBILITY: Easy to swap between different HUD styles
 * 3. POLYMORPHISM: Game code works with any HUD without knowing the type
 * 4. BLUEPRINT SUPPORT: Designers can create custom HUDs without C++ changes
 *
 * Example: The game might ship with a "minimal" HUD and a "detailed" HUD.
 * Both inherit from this class, so the race manager can use either one
 * interchangeably - it just calls UpdateSpeedDisplay() and the correct
 * implementation handles the visual update.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * Abstract Classes (UCLASS(Abstract)):
 *   - Cannot be instantiated directly (no NewObject<UMGRaceHUDWidget>())
 *   - Child classes must be created to use this functionality
 *   - Defines the interface that all children must implement
 *   - Can still contain shared implementation code
 *
 * BlueprintNativeEvent:
 *   A special function type that:
 *   - Can be implemented in C++ (via _Implementation suffix)
 *   - Can be overridden in Blueprint
 *   - Blueprint override calls parent with Super::FunctionName()
 *
 *   This is the bridge between C++ programmers and Blueprint designers.
 *   C++ defines what the function does by default, Blueprint can customize.
 *
 * EMGTachStyle Enum:
 *   Defines different visual styles for the tachometer (RPM display):
 *   - Arc: Curved sweep gauge (like car dashboards)
 *   - Bar: Linear horizontal/vertical bar
 *   - Digital: Numeric RPM display only
 *   - Needle: Classic analog needle gauge
 *
 *   Enums make code self-documenting and prevent invalid values.
 *
 * FMGHUDAnimationState Struct:
 *   Stores animation data for smoothly transitioning UI elements.
 *   Uses current/target pairs for interpolation (lerping).
 *
 *   Example: When position changes from 3rd to 2nd:
 *   - TargetScale = 1.5 (temporary enlargement)
 *   - CurrentScale lerps toward Target each frame
 *   - Creates smooth "pop" animation
 *
 * Smoothed Values (DisplayedSpeed, DisplayedRPM):
 *   Raw vehicle data can be jittery. Smoothed values use interpolation
 *   to gradually transition: DisplayedSpeed = FMath::FInterpTo(Current, Target, DeltaTime, Rate)
 *   This makes the HUD feel polished and professional.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *                    [UMGRaceHUDWidget]  (Abstract - this file)
 *                           ^
 *                           |
 *        +------------------+------------------+
 *        |                  |                  |
 *   [MGDefaultRaceHUD] [MGDefaultGameplayHUD] [BlueprintHUD]
 *   (C++ minimal)      (C++ full-featured)    (Designer-created)
 *
 * The game code only knows about UMGRaceHUDWidget. It calls methods like
 * UpdateSpeedDisplay(), and the correct child class handles it.
 *
 * INTERFACE CONTRACT:
 * -------------------
 * Any class inheriting from this must provide implementations for:
 *
 * DISPLAY UPDATES:
 *   - UpdateSpeedDisplay(): Show current speed (KPH or MPH)
 *   - UpdateTachometer(): Show RPM, gear, and shift indicators
 *   - UpdateNOSGauge(): Show nitrous oxide amount and activation
 *   - UpdatePositionDisplay(): Show race standing (1st, 2nd, etc.)
 *   - UpdateLapDisplay(): Show current lap and total laps
 *   - UpdateTimeDisplay(): Show lap times (current, best, total)
 *   - UpdateGapDisplay(): Show time gap to other racers
 *   - UpdateDriftDisplay(): Show drift score and combo chain
 *
 * ANIMATIONS:
 *   - PlayPositionChangeAnimation(): Celebrate gaining/losing position
 *   - PlayShiftIndicator(): Flash when approaching optimal shift point
 *   - PlayRedlineWarning(): Warn when RPM is too high
 *   - PlayNOSActivationEffect(): Visual feedback for NOS usage
 *   - PlayFinalLapEffect(): Indicate last lap of the race
 *   - PlayBestLapEffect(): Celebrate setting a new personal best
 *
 * CONFIGURATION OPTIONS:
 * ----------------------
 * - TachometerStyle: Visual style for RPM display
 * - ShiftIndicatorThreshold: When to flash shift light (% of max RPM)
 * - RedlineThreshold: When to show redline warning
 * - SpeedInterpRate: How quickly displayed speed follows actual speed
 * - RPMInterpRate: How quickly displayed RPM follows actual RPM
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "MGRaceHUDWidget.generated.h"

class UMGRaceHUDSubsystem;
class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;

/**
 * Tachometer display style
 */
UENUM(BlueprintType)
enum class EMGTachStyle : uint8
{
	/** Arc/sweep style */
	Arc,
	/** Linear bar */
	Bar,
	/** Digital numeric */
	Digital,
	/** Classic needle */
	Needle
};

/**
 * HUD element animation state
 */
USTRUCT(BlueprintType)
struct FMGHUDAnimationState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float CurrentAlpha = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float TargetAlpha = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector2D CurrentOffset = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector2D TargetOffset = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float CurrentScale = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float TargetScale = 1.0f;
};

/**
 * Race HUD Widget
 * Main racing HUD with speedometer, tachometer, position, laps, and timers
 *
 * This is a base class - actual UI layout should be created in Blueprints
 * that derive from this class, allowing for different HUD styles.
 */
UCLASS(Abstract)
class MIDNIGHTGRIND_API UMGRaceHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ==========================================
	// UPDATE FUNCTIONS
	// ==========================================

	/** Update all displays with current data */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshDisplay();

	/** Update speed display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Speed")
	void UpdateSpeedDisplay(float SpeedKPH, float SpeedMPH, bool bUseMPH);

	/** Update tachometer display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Speed")
	void UpdateTachometer(float RPM, float MaxRPM, int32 Gear, int32 TotalGears);

	/** Update NOS gauge */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Speed")
	void UpdateNOSGauge(float NOSAmount, bool bNOSActive);

	/** Update position display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Race")
	void UpdatePositionDisplay(int32 Position, int32 TotalRacers);

	/** Update lap display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Race")
	void UpdateLapDisplay(int32 CurrentLap, int32 TotalLaps, bool bFinalLap);

	/** Update time displays */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Race")
	void UpdateTimeDisplay(float CurrentLapTime, float BestLapTime, float TotalTime);

	/** Update gap display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Race")
	void UpdateGapDisplay(float GapToLeader, float GapToNext);

	/** Update drift score display */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Drift")
	void UpdateDriftDisplay(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining);

	// ==========================================
	// ELEMENT VISIBILITY
	// ==========================================

	/** Set visibility of a HUD element by name */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Visibility")
	void SetElementVisible(FName ElementName, bool bVisible);

	/** Set overall HUD opacity */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Visibility")
	void SetHUDOpacity(float Opacity);

	/** Set overall HUD scale */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Visibility")
	void SetHUDScale(float Scale);

	// ==========================================
	// ANIMATIONS
	// ==========================================

	/** Play position change animation */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayPositionChangeAnimation(int32 OldPosition, int32 NewPosition);

	/** Play shift indicator flash */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayShiftIndicator();

	/** Play redline warning */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayRedlineWarning();

	/** Play NOS activation effect */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayNOSActivationEffect();

	/** Play final lap effect */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayFinalLapEffect();

	/** Play best lap effect */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HUD|Animation")
	void PlayBestLapEffect();

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Tachometer style */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	EMGTachStyle TachometerStyle = EMGTachStyle::Arc;

	/** Shift indicator RPM threshold (as percentage of max) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	float ShiftIndicatorThreshold = 0.9f;

	/** Redline RPM threshold (as percentage of max) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	float RedlineThreshold = 0.95f;

	/** Speed interpolation rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	float SpeedInterpRate = 15.0f;

	/** RPM interpolation rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	float RPMInterpRate = 20.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Displayed speed (smoothed) */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	float DisplayedSpeed = 0.0f;

	/** Displayed RPM (smoothed) */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	float DisplayedRPM = 0.0f;

	/** Is shift indicator showing */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	bool bShiftIndicatorActive = false;

	/** Is redline warning showing */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	bool bRedlineActive = false;

	/** Current telemetry */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	FMGVehicleTelemetry CurrentTelemetry;

	/** Current race status */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	FMGRaceStatus CurrentRaceStatus;

	/** Current drift data */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	FMGDriftScoreData CurrentDriftData;

	/** Animation states for elements */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	TMap<FName, FMGHUDAnimationState> ElementAnimations;

	// ==========================================
	// CACHED REFERENCES
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGRaceHUDSubsystem> HUDSubsystem;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Get HUD subsystem */
	UFUNCTION(BlueprintPure, Category = "HUD")
	UMGRaceHUDSubsystem* GetHUDSubsystem() const;

	/** Format time as string (MM:SS.ms) */
	UFUNCTION(BlueprintPure, Category = "HUD|Helpers")
	FText FormatTime(float TimeInSeconds) const;

	/** Format gap time as string (+/-X.XXs) */
	UFUNCTION(BlueprintPure, Category = "HUD|Helpers")
	FText FormatGapTime(float GapInSeconds) const;

	/** Get color for position (gold, silver, bronze, white) */
	UFUNCTION(BlueprintPure, Category = "HUD|Helpers")
	FLinearColor GetPositionColor(int32 Position) const;

	/** Get color for gap (green if ahead, red if behind) */
	UFUNCTION(BlueprintPure, Category = "HUD|Helpers")
	FLinearColor GetGapColor(float Gap) const;

	/** Update smooth values */
	void UpdateSmoothValues(float DeltaTime);
};
