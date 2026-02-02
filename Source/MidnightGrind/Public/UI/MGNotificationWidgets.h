// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGNotificationWidgets.h
 * @brief Widget classes for displaying various notification styles
 *
 * =============================================================================
 * @section Overview
 * This file defines the widget classes that visually display notifications.
 * Each widget style corresponds to a display mode:
 *
 * - **Toast**: Small corner popup for quick, non-intrusive messages
 * - **Banner**: Horizontal strip at screen top/bottom for moderate importance
 * - **Popup**: Center-screen popup for rewards and confirmations
 * - **FullScreen**: Dramatic announcements (level up, race results)
 * - **Minimal**: Ultra-simple text-only display for subtle feedback
 * - **Achievement**: Special format for achievement unlocks with rarity
 *
 * Also includes the NotificationContainerWidget which manages positioning
 * and lifecycle of all notification widgets on screen, plus the
 * NotificationHistoryWidget for viewing past notifications.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Widget Binding**: Uses meta = (BindWidgetOptional) so Blueprint designers
 *   can create visual layouts while C++ handles the logic. Missing widgets
 *   are simply skipped rather than causing crashes.
 *
 * - **Show/Hide Animations**: Each widget can have ShowAnimation and HideAnimation
 *   bound from Blueprint using meta = (BindWidgetAnimOptional). The C++ code
 *   plays these automatically on Show()/Hide().
 *
 * - **Notification Container**: A parent widget that holds containers for each
 *   notification position (corner for toasts, center for popups, etc.) and
 *   routes notifications to the appropriate container based on style.
 *
 * - **Action Buttons**: Banner and Popup types support action buttons. When
 *   clicked, they call OnActionClicked() which notifies the subsystem.
 *
 * - **Reward Display**: Popup and FullScreen types can display lists of rewards
 *   using the FMGRewardDisplayData struct and UMGRewardItemWidget.
 *
 * - **Time Progress Bar**: Toast notifications can show a progress bar that
 *   drains as the notification timer counts down.
 *
 * =============================================================================
 * @section WidgetClasses Widget Class Hierarchy
 *
 *   UMGNotificationWidgetBase (Abstract Base)
 *        |
 *        +-- UMGToastNotificationWidget
 *        |       Small corner popup with icon, title, message, time progress
 *        |
 *        +-- UMGBannerNotificationWidget
 *        |       Horizontal strip with action buttons
 *        |
 *        +-- UMGPopupNotificationWidget
 *        |       Center popup with rewards list and close button
 *        |
 *        +-- UMGFullScreenNotificationWidget
 *        |       Full screen with stats, rewards, continue button
 *        |
 *        +-- UMGMinimalNotificationWidget
 *        |       Simple text-only display
 *        |
 *        +-- UMGAchievementNotificationWidget
 *                Special achievement format with rarity display
 *
 *   UMGNotificationContainerWidget
 *        Manages positioning and routing to appropriate containers
 *
 *   UMGNotificationHistoryWidget
 *        Displays notification history with filtering
 *
 *   UMGRewardItemWidget
 *        Individual reward item display (icon, name, quantity, rarity)
 *
 * =============================================================================
 * @section Architecture
 *
 *   [MGNotificationSubsystem]
 *          |
 *          | OnNotificationShown event
 *          v
 *   [UMGNotificationContainerWidget]
 *          |
 *          +-- OnNotificationShown() handler
 *          |       |
 *          |       +-- GetContainerForStyle(Style) --> Returns UPanelWidget*
 *          |       |       Toast     --> ToastContainer (VerticalBox)
 *          |       |       Banner    --> TopBannerContainer or BottomBannerContainer
 *          |       |       Popup     --> CenterContainer (Overlay)
 *          |       |       FullScreen --> FullScreenContainer (Overlay)
 *          |       |
 *          |       +-- CreateNotificationWidget(Data) --> Spawns widget
 *          |               |
 *          |               +-- Based on Style, creates appropriate widget type
 *          |               +-- Adds to ActiveWidgets array
 *          |               +-- Calls SetNotificationData() and Show()
 *          |
 *          +-- OnNotificationDismissed() handler
 *                  |
 *                  +-- RemoveNotificationWidget() --> Cleans up
 *
 * =============================================================================
 * @section Usage
 * @code
 * // === SETTING UP THE CONTAINER ===
 * // The container widget is typically added to the main HUD
 * UMGNotificationContainerWidget* Container = CreateWidget<UMGNotificationContainerWidget>(
 *     GetWorld(), ContainerWidgetClass);
 * Container->AddToViewport(50); // High Z-order for notifications
 *
 * // Widget types are configured in the container widget Blueprint defaults:
 * // - ToastWidgetClass
 * // - BannerWidgetClass
 * // - PopupWidgetClass
 * // - FullScreenWidgetClass
 * // - MinimalWidgetClass
 *
 * // === CREATING CUSTOM TOAST WIDGET ===
 * // For custom notification widgets, inherit from the appropriate base:
 * UCLASS()
 * class UMyCustomToast : public UMGToastNotificationWidget
 * {
 *     GENERATED_BODY()
 *
 * protected:
 *     // Custom widgets bound from Blueprint
 *     UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
 *     UImage* CustomBorderImage;
 *
 *     virtual void UpdateDisplay_Implementation() override
 *     {
 *         Super::UpdateDisplay_Implementation();
 *
 *         // Add custom display logic
 *         if (CustomBorderImage)
 *         {
 *             FLinearColor TypeColor = GetNotificationTypeColor(NotificationData.Type);
 *             CustomBorderImage->SetColorAndOpacity(TypeColor);
 *         }
 *     }
 * };
 *
 * // === ACCESSING NOTIFICATION DATA ===
 * // In Blueprint or C++, access the notification data:
 * void UMyWidget::UpdateDisplay_Implementation()
 * {
 *     FMGNotificationData Data = GetNotificationData();
 *
 *     if (TitleText)
 *         TitleText->SetText(Data.Title);
 *
 *     if (MessageText)
 *         MessageText->SetText(Data.Message);
 *
 *     if (IconImage && Data.Icon)
 *         IconImage->SetBrushFromTexture(Data.Icon);
 *
 *     // Display rewards if present
 *     for (const FMGRewardDisplayData& Reward : Data.Rewards)
 *     {
 *         UMGRewardItemWidget* RewardWidget = CreateWidget<UMGRewardItemWidget>(
 *             this, RewardItemWidgetClass);
 *         RewardWidget->SetRewardData(Reward);
 *         RewardsContainer->AddChild(RewardWidget);
 *     }
 * }
 *
 * // === ACHIEVEMENT NOTIFICATION ===
 * // Achievement widgets have special handling:
 * void UMyAchievementWidget::SetAchievementData(const FMGAchievementNotification& Achievement)
 * {
 *     AchievementData = Achievement;
 *
 *     if (AchievementNameText)
 *         AchievementNameText->SetText(Achievement.Name);
 *
 *     if (PointsText)
 *         PointsText->SetText(FText::AsNumber(Achievement.Points));
 *
 *     if (RarityText)
 *     {
 *         FText Rarity = FText::Format(
 *             NSLOCTEXT("UI", "RarityFormat", "{0}% of players have this"),
 *             FText::AsNumber(Achievement.RarityPercent));
 *         RarityText->SetText(Rarity);
 *     }
 *
 *     PlayUnlockAnimation();
 * }
 *
 * // === NOTIFICATION HISTORY ===
 * // The history widget displays past notifications:
 * void UMyHistoryScreen::SetupHistory()
 * {
 *     UMGNotificationHistoryWidget* History = CreateWidget<UMGNotificationHistoryWidget>(
 *         this, HistoryWidgetClass);
 *
 *     // Refresh to show current history
 *     History->RefreshHistory();
 *
 *     // Filter by type
 *     History->FilterByType(EMGNotificationType::Achievement);
 *
 *     // Clear filter to show all
 *     History->ClearFilter();
 * }
 * @endcode
 *
 * =============================================================================
 * @section BlueprintSetup Blueprint Setup Guide
 *
 * To create a notification widget in Blueprint:
 *
 * 1. **Create Blueprint Class**
 *    - Create new Widget Blueprint
 *    - Set parent class to appropriate base (e.g., MGToastNotificationWidget)
 *
 * 2. **Add Required Widgets** (BindWidgetOptional - use exact names)
 *    For Toast:
 *    - IconImage (Image)
 *    - TitleText (TextBlock)
 *    - MessageText (TextBlock)
 *    - TimeProgressBar (ProgressBar) - optional countdown
 *    - AccentImage (Image) - for type color accent
 *
 *    For Banner:
 *    - IconImage, TitleText, MessageText (same as Toast)
 *    - ActionButtonsBox (HorizontalBox) - for action buttons
 *    - BackgroundImage (Image)
 *
 *    For Popup:
 *    - TitleText, MessageText, IconImage
 *    - RewardsContainer (VerticalBox) - for reward items
 *    - CloseButton (Button)
 *
 *    For FullScreen:
 *    - TitleText, SubtitleText
 *    - ContentOverlay (Overlay)
 *    - RewardsContainer (VerticalBox)
 *    - StatsContainer (VerticalBox)
 *    - ContinueButton (Button)
 *    - PositionText (TextBlock) - for race position
 *    - NewRecordIndicator (Image)
 *
 * 3. **Add Animations** (BindWidgetAnimOptional - use exact names)
 *    - ShowAnimation: Plays when notification appears
 *    - HideAnimation: Plays when notification dismisses
 *
 * 4. **Configure Container Widget**
 *    In your NotificationContainerWidget Blueprint:
 *    - Set ToastWidgetClass, BannerWidgetClass, etc. in Details panel
 *    - Add container widgets: ToastContainer, CenterContainer, etc.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGNotificationSubsystem.h"
#include "MGNotificationWidgets.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UProgressBar;
class UVerticalBox;
class UHorizontalBox;
class UOverlay;
class UWidgetAnimation;

// =============================================================================
// Base Notification Widget
// =============================================================================

/**
 * Base notification widget
 * All notification widget types inherit from this class
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGNotificationWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set notification data */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	virtual void SetNotificationData(const FMGNotificationData& Data);

	/** Get notification data */
	UFUNCTION(BlueprintPure, Category = "Notification")
	FMGNotificationData GetNotificationData() const { return NotificationData; }

	/** Show notification */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	virtual void Show();

	/** Hide notification */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	virtual void Hide();

	/** Dismiss notification */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void Dismiss();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGNotificationData NotificationData;

	UPROPERTY()
	UMGNotificationSubsystem* NotificationSubsystem;

	/** Show animation */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* ShowAnimation;

	/** Hide animation */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* HideAnimation;

	virtual void NativeConstruct() override;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** On animation finished */
	UFUNCTION()
	void OnHideAnimationFinished();
};

/**
 * Toast notification widget (corner popup)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGToastNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	virtual void SetNotificationData(const FMGNotificationData& Data) override;

protected:
	/** Icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* IconImage;

	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* TitleText;

	/** Message text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* MessageText;

	/** Progress bar (for timed notifications) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UProgressBar* TimeProgressBar;

	/** Color accent */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* AccentImage;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void UpdateDisplay_Implementation() override;

	/** Update time progress */
	void UpdateTimeProgress();

	/** Timer */
	float ElapsedTime = 0.0f;
};

/**
 * Banner notification widget (top/bottom strip)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGBannerNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	virtual void SetNotificationData(const FMGNotificationData& Data) override;

protected:
	/** Icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* IconImage;

	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* TitleText;

	/** Message text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* MessageText;

	/** Action buttons container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UHorizontalBox* ActionButtonsBox;

	/** Background image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* BackgroundImage;

	virtual void UpdateDisplay_Implementation() override;

	/** Create action buttons */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void CreateActionButtons();

	/** Handle action clicked */
	UFUNCTION()
	void OnActionClicked(FName ActionID);
};

/**
 * Popup notification widget (center of screen)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPopupNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	virtual void SetNotificationData(const FMGNotificationData& Data) override;

protected:
	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* TitleText;

	/** Message text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* MessageText;

	/** Icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* IconImage;

	/** Rewards container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* RewardsContainer;

	/** Close button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* CloseButton;

	/** Reward item widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> RewardItemWidgetClass;

	virtual void NativeConstruct() override;

	virtual void UpdateDisplay_Implementation() override;

	/** Populate rewards */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void PopulateRewards();

	/** Handle close clicked */
	UFUNCTION()
	void OnCloseClicked();
};

/**
 * Reward item widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGRewardItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set reward data */
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void SetRewardData(const FMGRewardDisplayData& Reward);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGRewardDisplayData RewardData;

	/** Icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* RewardIcon;

	/** Name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* RewardNameText;

	/** Quantity text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* QuantityText;

	/** Rarity border */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* RarityBorder;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();
};

/**
 * Full screen notification widget (level up, race result)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGFullScreenNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	virtual void SetNotificationData(const FMGNotificationData& Data) override;

protected:
	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* TitleText;

	/** Subtitle text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* SubtitleText;

	/** Main content overlay */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UOverlay* ContentOverlay;

	/** Rewards container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* RewardsContainer;

	/** Continue button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* ContinueButton;

	/** Stats container (for race results) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* StatsContainer;

	/** Position display text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* PositionText;

	/** New record indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* NewRecordIndicator;

	/** Reward item widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> RewardItemWidgetClass;

	virtual void NativeConstruct() override;

	virtual void UpdateDisplay_Implementation() override;

	/** Populate rewards */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void PopulateRewards();

	/** Populate stats */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void PopulateStats();

	/** Handle continue clicked */
	UFUNCTION()
	void OnContinueClicked();

	/** Play celebration effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Effects")
	void PlayCelebration();
};

/**
 * Minimal notification widget (just text)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGMinimalNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	virtual void SetNotificationData(const FMGNotificationData& Data) override;

protected:
	/** Message text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* MessageText;

	/** Small icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* SmallIcon;

	virtual void UpdateDisplay_Implementation() override;
};

/**
 * Achievement notification widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGAchievementNotificationWidget : public UMGNotificationWidgetBase
{
	GENERATED_BODY()

public:
	/** Set achievement data */
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void SetAchievementData(const FMGAchievementNotification& Achievement);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGAchievementNotification AchievementData;

	/** Achievement icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* AchievementIcon;

	/** Achievement name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* AchievementNameText;

	/** Achievement description text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* AchievementDescText;

	/** Points text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* PointsText;

	/** Rarity text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* RarityText;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateAchievementDisplay();

	/** Play unlock animation */
	UFUNCTION(BlueprintNativeEvent, Category = "Effects")
	void PlayUnlockAnimation();
};

/**
 * Notification container widget
 * Manages displaying notifications in correct positions
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGNotificationContainerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** Toast notification widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGToastNotificationWidget> ToastWidgetClass;

	/** Banner notification widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGBannerNotificationWidget> BannerWidgetClass;

	/** Popup notification widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGPopupNotificationWidget> PopupWidgetClass;

	/** Full screen notification widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGFullScreenNotificationWidget> FullScreenWidgetClass;

	/** Minimal notification widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGMinimalNotificationWidget> MinimalWidgetClass;

	/** Toast container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* ToastContainer;

	/** Banner container (top) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UOverlay* TopBannerContainer;

	/** Banner container (bottom) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UOverlay* BottomBannerContainer;

	/** Center container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UOverlay* CenterContainer;

	/** Full screen container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UOverlay* FullScreenContainer;

	/** Active notification widgets */
	UPROPERTY()
	TArray<UMGNotificationWidgetBase*> ActiveWidgets;

	UPROPERTY()
	UMGNotificationSubsystem* NotificationSubsystem;

	/** Handle notification shown */
	UFUNCTION()
	void OnNotificationShown(const FMGNotificationData& Notification);

	/** Handle notification dismissed */
	UFUNCTION()
	void OnNotificationDismissed(const FMGNotificationData& Notification);

	/** Create widget for notification */
	UMGNotificationWidgetBase* CreateNotificationWidget(const FMGNotificationData& Notification);

	/** Get container for style */
	UPanelWidget* GetContainerForStyle(EMGNotificationStyle Style) const;

	/** Remove widget */
	void RemoveNotificationWidget(UMGNotificationWidgetBase* Widget);
};

/**
 * Notification history widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGNotificationHistoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Refresh history display */
	UFUNCTION(BlueprintCallable, Category = "History")
	void RefreshHistory();

	/** Filter by type */
	UFUNCTION(BlueprintCallable, Category = "History")
	void FilterByType(EMGNotificationType Type);

	/** Clear filter */
	UFUNCTION(BlueprintCallable, Category = "History")
	void ClearFilter();

	/** Clear all history */
	UFUNCTION(BlueprintCallable, Category = "History")
	void ClearAllHistory();

protected:
	/** History item widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> HistoryItemWidgetClass;

	/** History list container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* HistoryListContainer;

	/** Empty history text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* EmptyHistoryText;

	/** Unread count text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* UnreadCountText;

	UPROPERTY()
	UMGNotificationSubsystem* NotificationSubsystem;

	/** Current type filter */
	UPROPERTY(BlueprintReadOnly, Category = "Filter")
	EMGNotificationType CurrentTypeFilter;

	/** Is filtering by type */
	UPROPERTY(BlueprintReadOnly, Category = "Filter")
	bool bIsFiltering = false;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay(const TArray<FMGNotificationHistoryEntry>& Entries);
};
