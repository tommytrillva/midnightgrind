// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGDefaultGameplayHUD.h - Full-Featured Racing HUD Implementation
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the complete, production-ready racing HUD with all the
 * visual elements a player sees during a race. Unlike the minimal Debug HUD,
 * this includes a full digital speedometer, tachometer, nitrous gauge, drift
 * scoring display, minimap integration, and animated visual feedback.
 *
 * This is what players actually see when playing the game - the polished,
 * feature-complete dashboard interface.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * Class Inheritance:
 *   This class inherits from UMGRaceHUDWidget (see MGRaceHUDWidget.h).
 *   The base class defines the interface (what methods must exist), and this
 *   class provides the actual implementation (how those methods work).
 *
 *   UMGRaceHUDWidget (Abstract Base)
 *         |
 *         v
 *   UMGDefaultGameplayHUD (Concrete Implementation)
 *
 * _Implementation Suffix:
 *   Methods ending in "_Implementation" are the C++ implementations of
 *   BlueprintNativeEvent functions. This is Unreal's pattern for allowing
 *   both C++ and Blueprint to override the same function.
 *
 * Widget Binding Types:
 *   - BindWidget: REQUIRED - the Blueprint MUST have this widget or it crashes
 *   - BindWidgetOptional: OPTIONAL - the Blueprint may or may not have this widget
 *   Use Optional for features that might not appear in all HUD variations.
 *
 * UI Element Types Used:
 *   - UTextBlock: Displays text (speed numbers, lap count, times)
 *   - UProgressBar: Shows a value as a filled bar (RPM, nitrous amount)
 *   - UImage: Displays textures/icons (shift light, nitrous glow)
 *   - UBorder: Container with background color for visual grouping
 *   - UCanvasPanel: Allows precise positioning of child widgets
 *
 * Color Configuration:
 *   The UPROPERTY(EditDefaultsOnly) properties let designers adjust colors
 *   in the editor without changing code. This is essential for iteration -
 *   artists can tweak the look without programmer involvement.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Vehicle Pawn] -----> [HUD Data Provider] -----> [DefaultGameplayHUD]
 *        |                       |                           |
 *        v                       v                           v
 *   (Speed, RPM,           (Aggregates &              (Renders all
 *    Gear, NOS)            Formats Data)              UI Elements)
 *
 * The HUD receives data from the HUD Data Provider subsystem, which
 * aggregates information from the vehicle, race systems, and scoring systems.
 *
 * FEATURE BREAKDOWN:
 * ------------------
 * 1. SPEEDOMETER CLUSTER: Digital speed display with unit toggle (MPH/KPH)
 * 2. TACHOMETER: Arc-style RPM display with redline warning animation
 * 3. GEAR INDICATOR: Current gear with shift light when approaching redline
 * 4. NITROUS GAUGE: Shows NOS remaining with activation glow effect
 * 5. POSITION/LAP: Race standing (1st/8) and lap progress (2/3)
 * 6. TIMING: Current lap time, best lap time, and gap to other racers
 * 7. DRIFT DISPLAY: Score, multiplier, and combo chain during drifts
 * 8. MINIMAP: Integrated track map showing racer positions
 *
 * VISUAL STYLE:
 * -------------
 * The HUD follows a "Y2K aesthetic" with neon colors (cyan, pink, yellow)
 * reminiscent of early 2000s racing games. Color constants are configurable
 * through the editor properties.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "UI/MGRaceHUDWidget.h"
#include "MGDefaultGameplayHUD.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;
class UMGMinimapWidget;
class UOverlay;
class UBorder;

/**
 * Default Gameplay HUD - Concrete implementation with all core elements
 *
 * Features:
 * - Digital speedometer with unit toggle (MPH/KPH)
 * - Arc-style tachometer with redline indicator
 * - Gear indicator with shift light
 * - Nitrous gauge with activation effects
 * - Race position display (1st, 2nd, etc.)
 * - Lap counter with final lap indicator
 * - Current/Best lap times
 * - Gap to leader/next position
 * - Drift score display
 * - Integrated minimap
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDefaultGameplayHUD : public UMGRaceHUDWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// ==========================================
	// SPEEDOMETER ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeedText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeedUnitText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SpeedGlowEffect;

	// ==========================================
	// TACHOMETER ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> TachometerBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> TachometerRedline;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> GearText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ShiftLightImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RedlinePulse;

	// ==========================================
	// NITROUS ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> NitrousBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> NitrousActiveGlow;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NitrousLabel;

	// ==========================================
	// POSITION/LAP ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> PositionText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> PositionSuffixText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalRacersText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> LapText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FinalLapIndicator;

	// ==========================================
	// TIME ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentLapTimeText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> BestLapTimeText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TotalTimeText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GapText;

	// ==========================================
	// DRIFT ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> DriftScorePanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DriftScoreText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DriftMultiplierText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DriftChainBar;

	// ==========================================
	// MINIMAP
	// ==========================================

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UMGMinimapWidget> MinimapWidget;

	// ==========================================
	// UPDATE IMPLEMENTATIONS
	// ==========================================

	virtual void UpdateSpeedDisplay_Implementation(float SpeedKPH, float SpeedMPH, bool bUseMPH) override;
	virtual void UpdateTachometer_Implementation(float RPM, float MaxRPM, int32 Gear, int32 TotalGears) override;
	virtual void UpdateNOSGauge_Implementation(float NOSAmount, bool bNOSActive) override;
	virtual void UpdatePositionDisplay_Implementation(int32 Position, int32 TotalRacers) override;
	virtual void UpdateLapDisplay_Implementation(int32 CurrentLap, int32 TotalLaps, bool bFinalLap) override;
	virtual void UpdateTimeDisplay_Implementation(float CurrentLapTime, float BestLapTime, float TotalTime) override;
	virtual void UpdateGapDisplay_Implementation(float GapToLeader, float GapToNext) override;
	virtual void UpdateDriftDisplay_Implementation(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining) override;

	// ==========================================
	// ANIMATION IMPLEMENTATIONS
	// ==========================================

	virtual void PlayPositionChangeAnimation_Implementation(int32 OldPosition, int32 NewPosition) override;
	virtual void PlayShiftIndicator_Implementation() override;
	virtual void PlayRedlineWarning_Implementation() override;
	virtual void PlayNOSActivationEffect_Implementation() override;
	virtual void PlayFinalLapEffect_Implementation() override;
	virtual void PlayBestLapEffect_Implementation() override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Speed text color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor SpeedTextColor = FLinearColor::White;

	/** High speed text color (above threshold) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor HighSpeedTextColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

	/** High speed threshold (MPH) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Config")
	float HighSpeedThreshold = 150.0f;

	/** Tachometer bar color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor TachBarColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);

	/** Tachometer redline color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor TachRedlineColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Nitrous bar color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor NitrousBarColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Nitrous active color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor NitrousActiveColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	/** Drift score text color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD|Colors")
	FLinearColor DriftScoreColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

private:
	/** Get position suffix (st, nd, rd, th) */
	FString GetPositionSuffix(int32 Position) const;

	/** Animate element pulse */
	void AnimatePulse(UWidget* Widget, float Duration = 0.5f);

	/** Show/hide drift display based on activity */
	void UpdateDriftVisibility(bool bDrifting);

	bool bIsDrifting = false;
	bool bWasNOSActive = false;
	int32 LastPosition = 0;
};
