// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Stage 54: Debug Race HUD - MVP Minimal Display

/**
 * =============================================================================
 * MGDebugRaceHUD.h - Debug/Fallback Racing HUD
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines a minimal, developer-friendly HUD (Heads-Up Display) that
 * shows essential racing information during gameplay. Think of it as a simple
 * dashboard overlay that displays speed, position, lap count, and timing.
 *
 * WHY IT EXISTS:
 * --------------
 * In game development, you often need a working UI before artists create the
 * final polished version. This "debug" HUD serves two purposes:
 * 1. Provides immediate visual feedback during development and testing
 * 2. Acts as a fallback when no Blueprint-based HUD is configured
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * UUserWidget:
 *   This class inherits from UUserWidget, which is Unreal's base class for
 *   creating UI elements. UUserWidget handles rendering, input, and lifecycle
 *   for on-screen interfaces.
 *
 * BindWidget Meta Tag:
 *   The `meta = (BindWidget)` property specifier tells Unreal that this C++
 *   variable should be automatically connected to a widget with the same name
 *   in the Blueprint/UMG Designer. This is how code talks to visual elements.
 *
 * TObjectPtr and TWeakObjectPtr:
 *   - TObjectPtr<T>: A smart pointer that holds a reference to an Unreal object
 *   - TWeakObjectPtr<T>: A "weak" reference that won't prevent garbage collection
 *   Use weak pointers for references to objects you don't "own"
 *
 * NativeConstruct / NativeTick:
 *   - NativeConstruct(): Called once when the widget is created (like a constructor)
 *   - NativeTick(): Called every frame to update the display (like Update() in Unity)
 *
 * UFUNCTION Macros:
 *   - BlueprintCallable: Can be called from Blueprint visual scripting
 *   - Category: Groups functions in the Blueprint editor for organization
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Game Mode] --> [Race HUD Subsystem] --> [MGDebugRaceHUD]
 *                          |                        |
 *                          v                        v
 *                   (Provides Data)          (Displays Data)
 *
 * The HUD Subsystem gathers race data (speed, position, lap times) and sends
 * it to this widget for display. This separation keeps data logic separate
 * from presentation logic (a pattern called MVC or Model-View-Controller).
 *
 * TYPICAL USAGE:
 * --------------
 * This HUD is automatically created when racing starts if no custom HUD is set.
 * Game systems call methods like SetSpeed() and SetPosition() each frame to
 * update what the player sees.
 *
 * =============================================================================
 */

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
