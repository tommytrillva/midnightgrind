// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGLoadingScreenWidget.h
 * @brief Loading screen UI with progress bar, tips, and race info display
 *
 * =============================================================================
 * @section Overview
 * This file defines the loading screen widget displayed during level transitions.
 * The loading screen provides visual feedback during potentially long load times
 * and uses that time to inform/engage players with:
 *
 * - Animated progress bar showing load completion percentage
 * - Rotating gameplay tips appropriate to player skill level
 * - Track and vehicle previews when loading into races
 * - Race configuration summary (laps, weather, opponents)
 *
 * The visual design follows the Y2K neon aesthetic with animated backgrounds,
 * glowing progress bars, and bold typography.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Loading Context**: Different content based on what's loading (race track
 *   shows track preview, main menu shows game logo, etc.)
 *
 * - **Tip Rotation**: Tips cycle automatically on a timer, filtered by player
 *   level so beginners see basic tips and experts see advanced techniques.
 *
 * - **Smooth Progress**: The displayed progress interpolates smoothly rather
 *   than jumping, creating a more polished feel even with uneven load times.
 *
 * - **Fade Transitions**: Fade-in on appear, fade-out when load completes to
 *   smooth the transition to gameplay.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Level Loading System]
 *          |
 *          +-- SetProgress(0.0 to 1.0) --> [MGLoadingScreenWidget]
 *          |                                      |
 *          +-- SetContext(Race/Garage/...)        +-- Progress Bar Animation
 *          |                                      |
 *          +-- SetRaceData(track, vehicle...)     +-- Tip Rotation Timer
 *          |                                      |
 *          +-- SetLoadingComplete()               +-- Race Info Display
 *                                                 |
 *                                                 +-- Animated Background
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Create loading screen
 * UMGLoadingScreenWidget* LoadScreen = CreateWidget<UMGLoadingScreenWidget>(
 *     GetWorld(), LoadingScreenClass);
 *
 * // Configure for race loading
 * FMGRaceLoadingData RaceData;
 * RaceData.TrackName = FText::FromString("Downtown Circuit");
 * RaceData.LapCount = 3;
 * RaceData.RacerCount = 8;
 * LoadScreen->SetContext(EMGLoadingContext::Race);
 * LoadScreen->SetRaceData(RaceData);
 *
 * // Add loading tips
 * TArray<FMGLoadingTip> Tips;
 * FMGLoadingTip Tip;
 * Tip.TipText = FText::FromString("Draft behind opponents for a speed boost!");
 * Tips.Add(Tip);
 * LoadScreen->SetLoadingTips(Tips);
 *
 * // Display and update progress
 * LoadScreen->AddToViewport(100); // High Z-order to be on top
 * LoadScreen->SetProgress(0.5f);  // 50% loaded
 * LoadScreen->SetLoadingComplete(); // Triggers fade-out
 * @endcode
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGLoadingScreenWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;

// =============================================================================
// Enums and Structs
// =============================================================================

/**
 * Loading screen context - what we're loading into
 * Determines which UI sections to display and what content to show
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
	void UpdateProgressAnimation(float MGDeltaTime);

	/** Update tip rotation */
	void UpdateTipRotation(float MGDeltaTime);

	/** Update background animation */
	void UpdateBackgroundAnimation(float MGDeltaTime);

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
