// Copyright Midnight Grind. All Rights Reserved.
// Stage 54: Debug Race HUD - MVP Minimal Display

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGDebugRaceHUD.generated.h"

class UTextBlock;
class UCanvasPanel;
class UMGRaceHUDSubsystem;

/**
 * Minimal Debug Race HUD
 *
 * A simple HUD that displays essential race info without Blueprint setup.
 * Used as fallback when no Blueprint HUD is configured.
 *
 * Displays:
 * - Speed (mph)
 * - Position (e.g., "1st / 8")
 * - Lap (e.g., "Lap 2 / 3")
 * - Current lap time
 * - Countdown
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDebugRaceHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ==========================================
	// DISPLAY UPDATE
	// ==========================================

	/** Update speed display */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void SetSpeed(float SpeedMPH);

	/** Update position display */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void SetPosition(int32 Position, int32 TotalRacers);

	/** Update lap display */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void SetLap(int32 CurrentLap, int32 TotalLaps);

	/** Update time display */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void SetLapTime(float LapTimeSeconds);

	/** Show countdown */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void ShowCountdown(int32 Seconds);

	/** Show GO! */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void ShowGO();

	/** Hide countdown */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void HideCountdown();

	/** Show race finished */
	UFUNCTION(BlueprintCallable, Category = "DebugHUD")
	void ShowFinished(int32 FinalPosition);

protected:
	// ==========================================
	// UI ELEMENTS
	// ==========================================

	/** Speed text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeedText;

	/** Position text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> PositionText;

	/** Lap text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> LapText;

	/** Time text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	/** Countdown text (large center) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	// ==========================================
	// STATE
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGRaceHUDSubsystem> HUDSubsystem;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Fetch data from HUD subsystem */
	void UpdateFromSubsystem();

	/** Format time as string */
	FString FormatTime(float Seconds) const;

	/** Get position suffix (st, nd, rd, th) */
	FString GetPositionSuffix(int32 Position) const;
};
