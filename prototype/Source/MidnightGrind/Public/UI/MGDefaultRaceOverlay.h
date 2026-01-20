// Copyright Midnight Grind. All Rights Reserved.

#pragma once

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
	void UpdateCountdownAnimation(float DeltaTime);

	/** Update notification animations */
	void UpdateNotificationAnimations(float DeltaTime);

	/** Update wrong way flash animation */
	void UpdateWrongWayAnimation(float DeltaTime);

	/** Update finish overlay animation */
	void UpdateFinishAnimation(float DeltaTime);

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
