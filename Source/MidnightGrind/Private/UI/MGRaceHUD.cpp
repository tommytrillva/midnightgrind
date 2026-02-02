// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGRaceHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMGRaceHUD::AMGRaceHUD()
{
	PrimaryActorTick.bCanEverTick = true;

	// Y2K Neon colors
	NeonCyan = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);     // Cyan
	NeonMagenta = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
	NeonYellow = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
	NeonGreen = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);   // Green

	// Default VFX state
	bSpeedLinesEnabled = true;
	SpeedLinesIntensity = 0.0f;
	ScreenFlashColor = FLinearColor::Transparent;
	ScreenFlashRemaining = 0.0f;
	ScreenFlashIntensity = 0.0f;
}

void AMGRaceHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create widget instances
	if (SpeedometerWidgetClass)
	{
		SpeedometerWidget = CreateWidget<UUserWidget>(GetWorld(), SpeedometerWidgetClass);
		if (SpeedometerWidget)
		{
			SpeedometerWidget->AddToViewport(10);
		}
	}

	if (PositionWidgetClass)
	{
		PositionWidget = CreateWidget<UUserWidget>(GetWorld(), PositionWidgetClass);
		if (PositionWidget)
		{
			PositionWidget->AddToViewport(10);
		}
	}

	if (LapTimerWidgetClass)
	{
		LapTimerWidget = CreateWidget<UUserWidget>(GetWorld(), LapTimerWidgetClass);
		if (LapTimerWidget)
		{
			LapTimerWidget->AddToViewport(10);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[RaceHUD] HUD initialized - Y2K mode activated!"));
}

void AMGRaceHUD::DrawHUD()
{
	Super::DrawHUD();

	// Update screen flash
	if (ScreenFlashRemaining > 0.0f)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		ScreenFlashRemaining -= DeltaTime;
		DrawScreenFlash();
	}

	// Draw speed lines
	if (bSpeedLinesEnabled && SpeedLinesIntensity > 0.01f)
	{
		DrawSpeedLines();
	}
}

// ============================================
// UI WIDGETS
// ============================================

void AMGRaceHUD::ShowCountdown(float Duration)
{
	if (!CountdownWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RaceHUD] No countdown widget class set"));
		return;
	}

	if (!CountdownWidget)
	{
		CountdownWidget = CreateWidget<UUserWidget>(GetWorld(), CountdownWidgetClass);
	}

	if (CountdownWidget && !CountdownWidget->IsInViewport())
	{
		CountdownWidget->AddToViewport(100); // High Z-order for countdown
		UE_LOG(LogTemp, Log, TEXT("[RaceHUD] Countdown started!"));
	}
}

void AMGRaceHUD::HideCountdown()
{
	if (CountdownWidget && CountdownWidget->IsInViewport())
	{
		CountdownWidget->RemoveFromParent();
		UE_LOG(LogTemp, Log, TEXT("[RaceHUD] Countdown hidden"));
	}
}

void AMGRaceHUD::UpdatePosition(int32 Position, int32 TotalRacers)
{
	// TODO: Call Blueprint function on PositionWidget to update display
	UE_LOG(LogTemp, Verbose, TEXT("[RaceHUD] Position: %d/%d"), Position, TotalRacers);
}

void AMGRaceHUD::UpdateLap(int32 CurrentLap, int32 TotalLaps)
{
	// TODO: Call Blueprint function on LapTimerWidget to update display
	UE_LOG(LogTemp, Verbose, TEXT("[RaceHUD] Lap: %d/%d"), CurrentLap, TotalLaps);
}

void AMGRaceHUD::UpdateRaceTime(float TimeInSeconds)
{
	// TODO: Call Blueprint function on LapTimerWidget to update time
}

void AMGRaceHUD::ShowRaceResults(int32 FinalPosition, float FinalTime, int32 CashEarned)
{
	if (!RaceResultsWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RaceHUD] No race results widget class set"));
		return;
	}

	if (!RaceResultsWidget)
	{
		RaceResultsWidget = CreateWidget<UUserWidget>(GetWorld(), RaceResultsWidgetClass);
	}

	if (RaceResultsWidget)
	{
		RaceResultsWidget->AddToViewport(200); // Highest Z-order for results screen
		UE_LOG(LogTemp, Warning, TEXT("[RaceHUD] Race Results: Position %d, Time %.2fs, Cash $%d"), 
			FinalPosition, FinalTime, CashEarned);

		// TODO: Pass results data to widget via Blueprint function
	}

	// Flash screen with appropriate color
	FLinearColor FlashColor = NeonGreen; // Default to green (success)
	if (FinalPosition == 1)
	{
		FlashColor = NeonYellow; // Gold for 1st place
	}
	else if (FinalPosition > 3)
	{
		FlashColor = NeonCyan; // Cyan for participation
	}

	FlashScreen(FlashColor, 0.5f, 0.3f);
}

void AMGRaceHUD::FlashBoostIndicator()
{
	// Flash screen cyan for boost activation
	FlashScreen(NeonCyan, 0.2f, 0.5f);
	UE_LOG(LogTemp, Verbose, TEXT("[RaceHUD] BOOST!"));
}

// ============================================
// Y2K VISUAL EFFECTS
// ============================================

void AMGRaceHUD::SetSpeedLinesEnabled(bool bEnabled)
{
	bSpeedLinesEnabled = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("[RaceHUD] Speed lines %s"), bEnabled ? TEXT("ENABLED") : TEXT("disabled"));
}

void AMGRaceHUD::SetSpeedLinesIntensity(float Intensity)
{
	SpeedLinesIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void AMGRaceHUD::FlashScreen(FLinearColor Color, float Duration, float Intensity)
{
	ScreenFlashColor = Color;
	ScreenFlashRemaining = Duration;
	ScreenFlashIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void AMGRaceHUD::DrawSpeedLines()
{
	if (!Canvas)
	{
		return;
	}

	// Get screen dimensions
	float ScreenWidth = Canvas->SizeX;
	float ScreenHeight = Canvas->SizeY;
	float CenterX = ScreenWidth * 0.5f;
	float CenterY = ScreenHeight * 0.5f;

	// Calculate vanishing point (slightly below center for perspective)
	float VanishX = CenterX;
	float VanishY = CenterY + (ScreenHeight * 0.1f);

	// Draw radial speed lines
	int32 NumLines = FMath::RoundToInt(20.0f * SpeedLinesIntensity);
	for (int32 i = 0; i < NumLines; ++i)
	{
		// Random angle
		float Angle = FMath::FRandRange(0.0f, 360.0f);
		float AngleRad = FMath::DegreesToRadians(Angle);

		// Start point (near vanishing point)
		float StartDist = FMath::FRandRange(100.0f, 200.0f);
		FVector2D Start(
			VanishX + FMath::Cos(AngleRad) * StartDist,
			VanishY + FMath::Sin(AngleRad) * StartDist
		);

		// End point (at screen edge)
		float EndDist = FMath::Max(ScreenWidth, ScreenHeight);
		FVector2D End(
			VanishX + FMath::Cos(AngleRad) * EndDist,
			VanishY + FMath::Sin(AngleRad) * EndDist
		);

		// Line color (cyan with alpha based on intensity)
		FLinearColor LineColor = NeonCyan;
		LineColor.A = SpeedLinesIntensity * 0.3f * FMath::FRandRange(0.5f, 1.0f);

		// Draw line
		DrawLine(
			Start.X, Start.Y,
			End.X, End.Y,
			LineColor,
			FMath::FRandRange(1.0f, 3.0f) * SpeedLinesIntensity
		);
	}
}

void AMGRaceHUD::DrawScreenFlash()
{
	if (!Canvas || ScreenFlashRemaining <= 0.0f)
	{
		return;
	}

	// Calculate flash alpha (fade out over time)
	float FlashAlpha = ScreenFlashIntensity * (ScreenFlashRemaining / 0.5f);
	FLinearColor FlashColor = ScreenFlashColor;
	FlashColor.A = FlashAlpha;

	// Draw full-screen rect
	Canvas->K2_DrawBox(
		FVector2D(0.0f, 0.0f),
		FVector2D(Canvas->SizeX, Canvas->SizeY),
		2.0f,
		FlashColor
	);
}
