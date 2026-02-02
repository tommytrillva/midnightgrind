// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGDefaultRaceOverlay.h - Race Overlay System (Countdown, Notifications, Finish)
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the overlay layer that appears ON TOP of the regular HUD
 * for important race events. While the HUD shows constant information (speed,
 * position), the overlay handles temporary, attention-grabbing displays:
 *
 * - Race countdown ("3... 2... 1... GO!")
 * - Notifications ("POSITION GAINED!", "NEW BEST LAP!")
 * - Wrong way warning (big flashing "WRONG WAY" text)
 * - Race finish screen (final position, total time, new record indicator)
 *
 * Think of it as the "pop-up" layer that grabs the player's attention for
 * important moments, then fades away.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * Overlay vs HUD:
 *   - HUD: Always visible, shows current state (speed, position, lap)
 *   - Overlay: Appears temporarily for events, then hides
 *
 *   Visual Layer Stack:
 *   +-------------------+
 *   |  Race Overlay     |  <-- Top (countdown, notifications)
 *   +-------------------+
 *   |  Race HUD         |  <-- Middle (speedometer, position)
 *   +-------------------+
 *   |  Game World       |  <-- Bottom (3D rendered scene)
 *   +-------------------+
 *
 * USTRUCT (FMGNotificationDisplayEntry):
 *   A struct that groups related data together. This one holds all the UI
 *   elements needed for a single notification (panel, text, icon). Using
 *   structs keeps related widgets organized and makes cleanup easier.
 *
 * Animation in UI:
 *   The overlay uses manual animation in NativeTick() rather than UMG animations.
 *   Each frame:
 *   1. Update animation time: AnimTime += DeltaTime
 *   2. Calculate current value: Scale = 1.0 + sin(AnimTime) * 0.1
 *   3. Apply to widget: Widget->SetRenderScale(Scale)
 *
 *   This gives precise control but requires more code than Timeline animations.
 *
 * Input Handling (NativeOnKeyDown):
 *   The overlay can intercept keyboard input. During finish screen, it might
 *   wait for a key press before transitioning. Return FReply::Handled() to
 *   consume the input, or FReply::Unhandled() to pass it through.
 *
 * Notification Queue:
 *   Multiple notifications can be active simultaneously. The TArray holds
 *   all active entries, and LayoutNotificationEntries() stacks them vertically.
 *   Each notification has its own timer and is removed when Duration expires.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Race Manager] ---> [Overlay Widget] ---> [Screen Display]
 *        |                    |
 *        v                    v
 *   "Start Race!"      CreateCountdownDisplay()
 *   "Lap Complete!"    DisplayNotification()
 *   "Wrong Way!"       UpdateWrongWayDisplay()
 *   "Race Over!"       DisplayRaceFinish()
 *
 * Game systems call the overlay's public methods when events happen.
 * The overlay creates/animates UI and handles its own cleanup.
 *
 * STYLE CONSTANTS:
 * ----------------
 * The overlay uses a Y2K neon aesthetic matching the main HUD:
 * - CyanNeon: Primary text color (0, 255, 230)
 * - PinkNeon: Accent/emphasis (255, 0, 153)
 * - YellowNeon: Warnings (255, 255, 0)
 * - GoldColor: 1st place finish
 * - SilverColor: 2nd place finish
 * - BronzeColor: 3rd place finish
 *
 * Font sizes are defined as static constexpr for consistency and easy adjustment.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "MGRaceOverlayWidget.h"
#include "MGDefaultRaceOverlay.generated.h"

class UTextBlock;
class UImage;
class UCanvasPanel;
class UVerticalBox;
class UOverlay;
class UBorder;

/**
 * Notification display entry
 */
USTRUCT()
struct FMGNotificationDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY()
	int32 NotificationID = -1;

	UPROPERTY()
	UCanvasPanel* Panel = nullptr;

	UPROPERTY()
	UTextBlock* MainText = nullptr;

	UPROPERTY()
	UTextBlock* SubText = nullptr;

	UPROPERTY()
	UImage* Icon = nullptr;

	float SpawnTime = 0.0f;
	float Duration = 2.0f;
};

/**
 * Concrete implementation of race overlay with C++ UI
 * Provides functional overlays without requiring Blueprint setup
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDefaultRaceOverlay : public UMGRaceOverlayWidget
{
	GENERATED_BODY()

public:
	UMGDefaultRaceOverlay(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	//~ End UUserWidget Interface

protected:
	//~ Begin UMGRaceOverlayWidget Interface
	virtual void OnCountdownValueChanged_Implementation(int32 NewValue) override;
	virtual void OnCountdownGo_Implementation() override;
	virtual void DisplayNotification_Implementation(const FMGNotificationData& Data) override;
	virtual void RemoveNotification_Implementation(int32 NotificationID) override;
	virtual void UpdateWrongWayDisplay_Implementation(bool bShow) override;
	virtual void DisplayRaceFinish_Implementation(int32 Position, float Time, bool bNewRecord) override;
	//~ End UMGRaceOverlayWidget Interface

private:
	// ==========================================
	// UI CREATION
	// ==========================================

	/** Create all UI elements */
	void CreateUIElements();

	/** Create countdown display */
	void CreateCountdownDisplay();

	/** Create notification area */
	void CreateNotificationArea();

	/** Create wrong way warning display */
	void CreateWrongWayDisplay();

	/** Create finish overlay */
	void CreateFinishDisplay();

	/** Create a notification widget entry */
	FMGNotificationDisplayEntry CreateNotificationEntry(const FMGNotificationData& Data);

	// ==========================================
	// ANIMATION
	// ==========================================

	/** Update countdown animation */
	void UpdateCountdownAnimation(float MGDeltaTime);

	/** Update notification animations */
	void UpdateNotificationAnimations(float MGDeltaTime);

	/** Update wrong way flash animation */
	void UpdateWrongWayAnimation(float MGDeltaTime);

	/** Update finish overlay animation */
	void UpdateFinishAnimation(float MGDeltaTime);

	/** Get pulse scale multiplier */
	float GetPulseScale(float Time, float Frequency = 2.0f, float Amplitude = 0.15f) const;

	// ==========================================
	// HELPERS
	// ==========================================

	/** Set text with scale animation */
	void SetCountdownTextWithAnimation(const FText& Text, FLinearColor Color);

	/** Position notification entries */
	void LayoutNotificationEntries();

	/** Get position color (gold, silver, bronze, etc) */
	FSlateColor GetPositionDisplayColor(int32 Position) const;

	/** Format position with ordinal (1st, 2nd, etc) */
	FText FormatPosition(int32 Position) const;

	// ==========================================
	// ROOT UI
	// ==========================================

	UPROPERTY()
	UCanvasPanel* RootCanvas;

	// ==========================================
	// COUNTDOWN UI
	// ==========================================

	UPROPERTY()
	UCanvasPanel* CountdownPanel;

	UPROPERTY()
	UTextBlock* CountdownText;

	UPROPERTY()
	UTextBlock* CountdownSubText;

	float CountdownAnimTime = 0.0f;
	bool bCountdownAnimating = false;
	float CountdownScaleTarget = 1.0f;
	float CountdownCurrentScale = 1.0f;

	// ==========================================
	// NOTIFICATION UI
	// ==========================================

	UPROPERTY()
	UVerticalBox* NotificationContainer;

	UPROPERTY()
	TArray<FMGNotificationDisplayEntry> NotificationEntries;

	// ==========================================
	// WRONG WAY UI
	// ==========================================

	UPROPERTY()
	UCanvasPanel* WrongWayPanel;

	UPROPERTY()
	UTextBlock* WrongWayText;

	UPROPERTY()
	UBorder* WrongWayBorder;

	float WrongWayAnimTime = 0.0f;
	bool bWrongWayVisible = false;

	// ==========================================
	// FINISH UI
	// ==========================================

	UPROPERTY()
	UCanvasPanel* FinishPanel;

	UPROPERTY()
	UTextBlock* FinishMainText;

	UPROPERTY()
	UTextBlock* FinishPositionText;

	UPROPERTY()
	UTextBlock* FinishTimeText;

	UPROPERTY()
	UTextBlock* FinishRecordText;

	UPROPERTY()
	UBorder* FinishBackground;

	float FinishAnimTime = 0.0f;
	bool bFinishVisible = false;
	int32 FinishPosition = 0;
	bool bFinishNewRecord = false;

	// ==========================================
	// STYLE CONSTANTS
	// ==========================================

	// Y2K aesthetic colors
	FLinearColor CyanNeon = FLinearColor(0.0f, 1.0f, 0.9f, 1.0f);
	FLinearColor PinkNeon = FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);
	FLinearColor YellowNeon = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	FLinearColor GreenNeon = FLinearColor(0.0f, 1.0f, 0.4f, 1.0f);
	FLinearColor RedNeon = FLinearColor(1.0f, 0.0f, 0.2f, 1.0f);
	FLinearColor GoldColor = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);
	FLinearColor SilverColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
	FLinearColor BronzeColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);

	// Font sizes
	static constexpr float CountdownFontSize = 200.0f;
	static constexpr float CountdownSubFontSize = 48.0f;
	static constexpr float NotificationFontSize = 36.0f;
	static constexpr float NotificationSubFontSize = 24.0f;
	static constexpr float WrongWayFontSize = 72.0f;
	static constexpr float FinishMainFontSize = 80.0f;
	static constexpr float FinishPositionFontSize = 120.0f;
	static constexpr float FinishTimeFontSize = 48.0f;
};
