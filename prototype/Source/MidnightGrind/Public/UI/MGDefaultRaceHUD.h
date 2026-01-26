// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/MGRaceHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "MGDefaultRaceHUD.generated.h"

/**
 * Default Race HUD Implementation
 * Provides a functional C++ HUD without requiring Blueprint setup
 * Creates all UI elements programmatically for immediate testing
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDefaultRaceHUD : public UMGRaceHUDWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// ==========================================
	// OVERRIDE IMPLEMENTATIONS
	// ==========================================

	virtual void UpdateSpeedDisplay_Implementation(float SpeedKPH, float SpeedMPH, bool bUseMPH) override;
	virtual void UpdateTachometer_Implementation(float RPM, float MaxRPM, int32 Gear, int32 TotalGears) override;
	virtual void UpdateNOSGauge_Implementation(float NOSAmount, bool bNOSActive) override;
	virtual void UpdatePositionDisplay_Implementation(int32 Position, int32 TotalRacers) override;
	virtual void UpdateLapDisplay_Implementation(int32 CurrentLap, int32 TotalLaps, bool bFinalLap) override;
	virtual void UpdateTimeDisplay_Implementation(float CurrentLapTime, float BestLapTime, float TotalTime) override;
	virtual void UpdateGapDisplay_Implementation(float GapToLeader, float GapToNext) override;
	virtual void UpdateDriftDisplay_Implementation(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining) override;

	virtual void PlayPositionChangeAnimation_Implementation(int32 OldPosition, int32 NewPosition) override;
	virtual void PlayShiftIndicator_Implementation() override;
	virtual void PlayRedlineWarning_Implementation() override;
	virtual void PlayNOSActivationEffect_Implementation() override;
	virtual void PlayFinalLapEffect_Implementation() override;
	virtual void PlayBestLapEffect_Implementation() override;

	// ==========================================
	// UI ELEMENTS
	// ==========================================

	/** Main canvas panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCanvasPanel* RootCanvas;

	// Speed cluster (bottom right)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* SpeedText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* SpeedUnitText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* GearText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UProgressBar* RPMBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UProgressBar* NOSBar;

	// Race info (top)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* PositionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* LapText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* CurrentLapTimeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* BestLapTimeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TotalTimeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* GapText;

	// Drift display (center)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* DriftScoreText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* DriftMultiplierText;

	// Status messages (center)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* StatusMessageText;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Y2K aesthetic font color - neon cyan */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor PrimaryColor = FSlateColor(FLinearColor(0.0f, 1.0f, 0.9f, 1.0f));

	/** Secondary accent color - neon pink */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor AccentColor = FSlateColor(FLinearColor(1.0f, 0.0f, 0.6f, 1.0f));

	/** Warning color - neon yellow */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor WarningColor = FSlateColor(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f));

private:
	/** Create UI elements programmatically */
	void CreateUIElements();

	/** Helper to create styled text block */
	UTextBlock* CreateStyledText(const FString& InitialText, int32 FontSize, FSlateColor Color, EHorizontalAlignment HAlign = EHorizontalAlignment::HAlign_Center);

	/** Helper to create progress bar */
	UProgressBar* CreateStyledProgressBar(FLinearColor FillColor, FLinearColor BackgroundColor);

	/** Position element in canvas */
	void PositionElement(UWidget* Widget, FVector2D Position, FVector2D Size, FAnchors Anchors);

	/** Status message timer */
	FTimerHandle StatusMessageTimerHandle;

	/** Clear status message */
	void ClearStatusMessage();
};
