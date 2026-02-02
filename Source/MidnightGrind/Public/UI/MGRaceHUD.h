// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MGRaceHUD.generated.h"

class UMGRaceWidget;
class UMGSpeedometer;
class UMGPositionIndicator;
class UMGLapTimer;
class UMGCountdownWidget;

/**
 * Streamlined Racing HUD for Single-Player
 * 
 * Y2K Aesthetic Goals:
 * - Neon colors (cyan, magenta, yellow)
 * - Glowing elements
 * - Speed lines
 * - Digital/tech feel
 * - Minimal but EXCITING
 * 
 * Shows:
 * - Speed (big and bold)
 * - Current position (1st, 2nd, etc.)
 * - Lap counter
 * - Race timer
 * - Countdown (3...2...1...GO!)
 * - Boost meter
 */
UCLASS()
class MIDNIGHTGRIND_API AMGRaceHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMGRaceHUD();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	//~ End AActor Interface

	// ============================================
	// UI WIDGETS
	// ============================================

	/** Show countdown (3, 2, 1, GO!) */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowCountdown(float Duration);

	/** Hide countdown */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideCountdown();

	/** Update position display */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdatePosition(int32 Position, int32 TotalRacers);

	/** Update lap display */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateLap(int32 CurrentLap, int32 TotalLaps);

	/** Update race time */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateRaceTime(float TimeInSeconds);

	/** Show race results */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowRaceResults(int32 FinalPosition, float FinalTime, int32 CashEarned);

	/** Flash boost indicator */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void FlashBoostIndicator();

	// ============================================
	// Y2K VISUAL EFFECTS
	// ============================================

	/** Enable/disable speed lines effect */
	UFUNCTION(BlueprintCallable, Category = "HUD|VFX")
	void SetSpeedLinesEnabled(bool bEnabled);

	/** Set speed lines intensity (0-1) */
	UFUNCTION(BlueprintCallable, Category = "HUD|VFX")
	void SetSpeedLinesIntensity(float Intensity);

	/** Flash screen with color (impact, boost, etc.) */
	UFUNCTION(BlueprintCallable, Category = "HUD|VFX")
	void FlashScreen(FLinearColor Color, float Duration, float Intensity);

protected:
	// ============================================
	// WIDGET CLASSES
	// ============================================

	/** Speedometer widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> SpeedometerWidgetClass;

	/** Position indicator widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> PositionWidgetClass;

	/** Lap timer widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> LapTimerWidgetClass;

	/** Countdown widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> CountdownWidgetClass;

	/** Race results widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> RaceResultsWidgetClass;

	// ============================================
	// WIDGET INSTANCES
	// ============================================

	UPROPERTY()
	UUserWidget* SpeedometerWidget;

	UPROPERTY()
	UUserWidget* PositionWidget;

	UPROPERTY()
	UUserWidget* LapTimerWidget;

	UPROPERTY()
	UUserWidget* CountdownWidget;

	UPROPERTY()
	UUserWidget* RaceResultsWidget;

	// ============================================
	// Y2K VISUAL EFFECTS
	// ============================================

	/** Are speed lines enabled? */
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	bool bSpeedLinesEnabled;

	/** Speed lines intensity */
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	float SpeedLinesIntensity;

	/** Screen flash color */
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FLinearColor ScreenFlashColor;

	/** Screen flash remaining time */
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	float ScreenFlashRemaining;

	/** Screen flash intensity */
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	float ScreenFlashIntensity;

	// ============================================
	// INTERNAL
	// ============================================

	/** Draw speed lines effect */
	void DrawSpeedLines();

	/** Draw screen flash */
	void DrawScreenFlash();

	/** Y2K neon colors */
	FLinearColor NeonCyan;
	FLinearColor NeonMagenta;
	FLinearColor NeonYellow;
	FLinearColor NeonGreen;
};
