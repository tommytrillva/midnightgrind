// Copyright Midnight Grind. All Rights Reserved.

#pragma once

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

/**
 * Base notification widget
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
