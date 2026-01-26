// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGLoadingScreenWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;

/**
 * Loading screen context - what we're loading into
 */
UENUM(BlueprintType)
enum class EMGLoadingContext : uint8
{
	/** Initial game boot */
	GameBoot,
	/** Loading main menu */
	MainMenu,
	/** Loading race track */
	Race,
	/** Loading garage */
	Garage,
	/** Loading replay */
	Replay,
	/** Generic loading */
	Generic
};

/**
 * Loading screen data for race context
 */
USTRUCT(BlueprintType)
struct FMGRaceLoadingData
{
	GENERATED_BODY()

	/** Track name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrackName;

	/** Track location/region */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrackLocation;

	/** Track length in km */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackLengthKm = 0.0f;

	/** Number of laps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	/** Number of racers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacerCount = 8;

	/** Weather condition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Weather;

	/** Time of day */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TimeOfDay;

	/** Player vehicle name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerVehicle;

	/** Track preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* TrackPreviewImage = nullptr;

	/** Vehicle preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* VehiclePreviewImage = nullptr;

	/** Race mode name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RaceMode;

	/** Is ranked race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;
};

/**
 * Loading tip entry
 */
USTRUCT(BlueprintType)
struct FMGLoadingTip
{
	GENERATED_BODY()

	/** Tip text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TipText;

	/** Category for filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Category;

	/** Optional icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Minimum player level to show this tip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayerLevel = 0;

	/** Maximum player level (0 = no max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayerLevel = 0;
};

/**
 * Loading Screen Widget
 * Displays during level transitions with progress, tips, and context info
 *
 * Features:
 * - Animated progress bar
 * - Rotating loading tips
 * - Track/vehicle previews for races
 * - Race info summary
 * - Y2K neon aesthetic
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMGLoadingScreenWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set loading context */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetContext(EMGLoadingContext Context);

	/** Set race loading data */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetRaceData(const FMGRaceLoadingData& Data);

	/** Set loading tips */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetLoadingTips(const TArray<FMGLoadingTip>& Tips);

	/** Set tip rotation interval */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetTipInterval(float Seconds);

	// ==========================================
	// PROGRESS
	// ==========================================

	/** Set progress (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetProgress(float Progress);

	/** Set progress text */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetProgressText(const FText& Text);

	/** Set loading complete (triggers fade out) */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetLoadingComplete();

	/** Get current progress */
	UFUNCTION(BlueprintPure, Category = "Loading Screen")
	float GetProgress() const { return CurrentProgress; }

	/** Is loading complete */
	UFUNCTION(BlueprintPure, Category = "Loading Screen")
	bool IsLoadingComplete() const { return bLoadingComplete; }

	// ==========================================
	// TIPS
	// ==========================================

	/** Show next tip */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen|Tips")
	void ShowNextTip();

	/** Show specific tip by index */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen|Tips")
	void ShowTip(int32 Index);

	/** Get current tip index */
	UFUNCTION(BlueprintPure, Category = "Loading Screen|Tips")
	int32 GetCurrentTipIndex() const { return CurrentTipIndex; }

protected:
	// ==========================================
	// UI CREATION
	// ==========================================

	/** Create all UI elements */
	virtual void CreateUIElements();

	/** Create progress bar section */
	void CreateProgressSection();

	/** Create tip display section */
	void CreateTipSection();

	/** Create race info section */
	void CreateRaceInfoSection();

	/** Create animated background */
	void CreateAnimatedBackground();

	// ==========================================
	// UPDATE
	// ==========================================

	/** Update progress bar animation */
	void UpdateProgressAnimation(float DeltaTime);

	/** Update tip rotation */
	void UpdateTipRotation(float DeltaTime);

	/** Update background animation */
	void UpdateBackgroundAnimation(float DeltaTime);

	/** Update race info display */
	void UpdateRaceInfoDisplay();

	// ==========================================
	// ANIMATION
	// ==========================================

	/** Animate tip transition */
	void AnimateTipTransition();

	/** Fade in effect */
	void PlayFadeIn();

	/** Fade out effect */
	void PlayFadeOut();

private:
	// ==========================================
	// STATE
	// ==========================================

	EMGLoadingContext LoadingContext = EMGLoadingContext::Generic;
	FMGRaceLoadingData RaceData;
	TArray<FMGLoadingTip> LoadingTips;

	float CurrentProgress = 0.0f;
	float DisplayedProgress = 0.0f;
	float TargetProgress = 0.0f;

	int32 CurrentTipIndex = 0;
	float TipTimer = 0.0f;
	float TipInterval = 5.0f;
	bool bTipTransitioning = false;
	float TipTransitionProgress = 0.0f;

	bool bLoadingComplete = false;
	float FadeProgress = 0.0f;

	float BackgroundAnimTime = 0.0f;
	float ProgressGlowTime = 0.0f;

	// ==========================================
	// UI ELEMENTS
	// ==========================================

	UPROPERTY()
	UCanvasPanel* RootCanvas;

	// Progress
	UPROPERTY()
	UProgressBar* ProgressBar;

	UPROPERTY()
	UTextBlock* ProgressText;

	UPROPERTY()
	UTextBlock* ProgressPercentText;

	// Tips
	UPROPERTY()
	UCanvasPanel* TipPanel;

	UPROPERTY()
	UTextBlock* TipText;

	UPROPERTY()
	UTextBlock* TipLabelText;

	UPROPERTY()
	UImage* TipIcon;

	// Race info
	UPROPERTY()
	UCanvasPanel* RaceInfoPanel;

	UPROPERTY()
	UTextBlock* TrackNameText;

	UPROPERTY()
	UTextBlock* TrackLocationText;

	UPROPERTY()
	UTextBlock* RaceModeText;

	UPROPERTY()
	UTextBlock* LapCountText;

	UPROPERTY()
	UTextBlock* WeatherText;

	UPROPERTY()
	UTextBlock* VehicleText;

	UPROPERTY()
	UImage* TrackPreviewImage;

	UPROPERTY()
	UImage* VehiclePreviewImage;

	// Background
	UPROPERTY()
	UImage* BackgroundImage;

	UPROPERTY()
	UCanvasPanel* GridOverlay;

	// ==========================================
	// STYLE
	// ==========================================

	FLinearColor CyanNeon = FLinearColor(0.0f, 1.0f, 0.9f, 1.0f);
	FLinearColor PinkNeon = FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);
	FLinearColor BackgroundColor = FLinearColor(0.02f, 0.02f, 0.05f, 1.0f);

	static constexpr float ProgressBarSmoothSpeed = 3.0f;
	static constexpr float TipFadeDuration = 0.5f;
};
