// Copyright Midnight Grind. All Rights Reserved.

#include "Spectator/MGSpectatorWidgets.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ==========================================
// UMGSpectatorHUDWidget
// ==========================================

void UMGSpectatorHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();

		if (SpectatorSubsystem)
		{
			SpectatorSubsystem->OnTargetChanged.AddDynamic(this, &UMGSpectatorHUDWidget::OnTargetChanged);
			SpectatorSubsystem->OnCameraModeChanged.AddDynamic(this, &UMGSpectatorHUDWidget::OnCameraModeChanged);
		}
	}

	UpdateDisplay();
}

void UMGSpectatorHUDWidget::NativeDestruct()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->OnTargetChanged.RemoveDynamic(this, &UMGSpectatorHUDWidget::OnTargetChanged);
		SpectatorSubsystem->OnCameraModeChanged.RemoveDynamic(this, &UMGSpectatorHUDWidget::OnCameraModeChanged);
	}

	Super::NativeDestruct();
}

void UMGSpectatorHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update dynamic displays
	UpdateTargetInfoDisplay();
}

void UMGSpectatorHUDWidget::UpdateDisplay()
{
	UpdateTargetInfoDisplay();
	UpdateStandingsDisplay();
	UpdateCameraInfoDisplay();
}

void UMGSpectatorHUDWidget::SetOverlayVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMGSpectatorHUDWidget::OnTargetChanged(const FMGSpectatorTarget& NewTarget)
{
	UpdateTargetInfoDisplay();
}

void UMGSpectatorHUDWidget::OnCameraModeChanged(EMGSpectatorCameraMode NewMode)
{
	UpdateCameraInfoDisplay();
}

void UMGSpectatorHUDWidget::UpdateTargetInfoDisplay_Implementation()
{
	// Implementation in Blueprint
}

void UMGSpectatorHUDWidget::UpdateStandingsDisplay_Implementation()
{
	// Implementation in Blueprint
}

void UMGSpectatorHUDWidget::UpdateCameraInfoDisplay_Implementation()
{
	if (CameraInfoText && SpectatorSubsystem)
	{
		FText ModeName = UMGSpectatorSubsystem::GetCameraModeDisplayName(SpectatorSubsystem->GetCameraMode());
		CameraInfoText->SetText(ModeName);
	}
}

// ==========================================
// UMGSpectatorTargetInfoWidget
// ==========================================

void UMGSpectatorTargetInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();
	}
}

void UMGSpectatorTargetInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update target data from subsystem
	if (SpectatorSubsystem)
	{
		SetTargetData(SpectatorSubsystem->GetCurrentTarget());
	}
}

void UMGSpectatorTargetInfoWidget::SetTargetData(const FMGSpectatorTarget& Target)
{
	TargetData = Target;
	UpdateDisplay();
}

void UMGSpectatorTargetInfoWidget::UpdateDisplay_Implementation()
{
	if (DriverNameText)
	{
		DriverNameText->SetText(TargetData.TargetName);
	}

	if (PositionText)
	{
		FText PositionFormatted = FText::Format(NSLOCTEXT("Spectator", "Position", "P{0}"), FText::AsNumber(TargetData.RacePosition));
		PositionText->SetText(PositionFormatted);
	}

	if (SpeedText)
	{
		FText SpeedFormatted = FText::Format(NSLOCTEXT("Spectator", "Speed", "{0} km/h"), FText::AsNumber(FMath::RoundToInt(TargetData.CurrentSpeed)));
		SpeedText->SetText(SpeedFormatted);
	}

	if (LapText)
	{
		FText LapFormatted = FText::Format(NSLOCTEXT("Spectator", "Lap", "Lap {0}"), FText::AsNumber(TargetData.CurrentLap));
		LapText->SetText(LapFormatted);
	}

	if (BestLapText && TargetData.BestLapTime > 0.0f)
	{
		BestLapText->SetText(UMGSpectatorLapTimerWidget::FormatLapTime(TargetData.BestLapTime));
		BestLapText->SetVisibility(ESlateVisibility::Visible);
	}
	else if (BestLapText)
	{
		BestLapText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TeamColorImage)
	{
		TeamColorImage->SetColorAndOpacity(TargetData.TeamColor);
	}

	if (AIIndicator)
	{
		AIIndicator->SetVisibility(TargetData.bIsAI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// ==========================================
// UMGSpectatorStandingsWidget
// ==========================================

void UMGSpectatorStandingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();

		if (SpectatorSubsystem)
		{
			SpectatorSubsystem->OnTargetChanged.AddDynamic(this, &UMGSpectatorStandingsWidget::OnTargetChanged);
		}
	}

	RefreshStandings();
}

void UMGSpectatorStandingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Periodic refresh
	static float RefreshTimer = 0.0f;
	RefreshTimer += InDeltaTime;
	if (RefreshTimer >= 1.0f)
	{
		RefreshTimer = 0.0f;
		RefreshStandings();
	}
}

void UMGSpectatorStandingsWidget::RefreshStandings()
{
	if (!SpectatorSubsystem)
	{
		return;
	}

	TArray<FMGSpectatorTarget> Targets = SpectatorSubsystem->GetAllTargets();

	// Sort by position
	Targets.Sort([](const FMGSpectatorTarget& A, const FMGSpectatorTarget& B)
	{
		return A.RacePosition < B.RacePosition;
	});

	UpdateDisplay(Targets);
}

void UMGSpectatorStandingsWidget::OnTargetChanged(const FMGSpectatorTarget& NewTarget)
{
	// Highlight the new target in standings
	for (int32 i = 0; i < StandingsEntryWidgets.Num(); i++)
	{
		UMGSpectatorStandingsEntryWidget* Entry = Cast<UMGSpectatorStandingsEntryWidget>(StandingsEntryWidgets[i]);
		if (Entry)
		{
			Entry->SetHighlighted(Entry->GetClass() && NewTarget.Target != nullptr);
		}
	}
}

void UMGSpectatorStandingsWidget::UpdateDisplay_Implementation(const TArray<FMGSpectatorTarget>& Targets)
{
	if (!StandingsContainer || !StandingsEntryWidgetClass)
	{
		return;
	}

	// Create/update entry widgets
	while (StandingsEntryWidgets.Num() < Targets.Num())
	{
		UMGSpectatorStandingsEntryWidget* NewEntry = CreateWidget<UMGSpectatorStandingsEntryWidget>(this, StandingsEntryWidgetClass);
		if (NewEntry)
		{
			StandingsContainer->AddChild(NewEntry);
			StandingsEntryWidgets.Add(NewEntry);
		}
	}

	// Hide extra widgets
	for (int32 i = Targets.Num(); i < StandingsEntryWidgets.Num(); i++)
	{
		StandingsEntryWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update visible widgets
	FMGSpectatorTarget CurrentTarget;
	if (SpectatorSubsystem)
	{
		CurrentTarget = SpectatorSubsystem->GetCurrentTarget();
	}

	for (int32 i = 0; i < Targets.Num(); i++)
	{
		UMGSpectatorStandingsEntryWidget* Entry = Cast<UMGSpectatorStandingsEntryWidget>(StandingsEntryWidgets[i]);
		if (Entry)
		{
			Entry->SetTargetData(Targets[i]);
			Entry->SetHighlighted(Targets[i].Target == CurrentTarget.Target);
			Entry->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

// ==========================================
// UMGSpectatorStandingsEntryWidget
// ==========================================

void UMGSpectatorStandingsEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ClickButton)
	{
		ClickButton->OnClicked.AddDynamic(this, &UMGSpectatorStandingsEntryWidget::HandleClick);
	}
}

void UMGSpectatorStandingsEntryWidget::SetTargetData(const FMGSpectatorTarget& Target)
{
	TargetData = Target;
	UpdateDisplay();
}

void UMGSpectatorStandingsEntryWidget::SetHighlighted(bool bHighlight)
{
	bIsHighlighted = bHighlight;

	if (BackgroundImage)
	{
		FLinearColor BgColor = bHighlight ? FLinearColor(0.2f, 0.4f, 0.8f, 0.8f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.6f);
		BackgroundImage->SetColorAndOpacity(BgColor);
	}
}

void UMGSpectatorStandingsEntryWidget::UpdateDisplay_Implementation()
{
	if (PositionText)
	{
		PositionText->SetText(FText::AsNumber(TargetData.RacePosition));
	}

	if (NameText)
	{
		NameText->SetText(TargetData.TargetName);
	}

	if (TeamColorBar)
	{
		TeamColorBar->SetColorAndOpacity(TargetData.TeamColor);
	}
}

void UMGSpectatorStandingsEntryWidget::HandleClick()
{
	OnClicked.Broadcast(TargetData);
}

// ==========================================
// UMGSpectatorControlsWidget
// ==========================================

void UMGSpectatorControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();

		if (SpectatorSubsystem)
		{
			SpectatorSubsystem->OnCameraModeChanged.AddDynamic(this, &UMGSpectatorControlsWidget::OnCameraModeChanged);
		}
	}

	// Bind button clicks
	if (PrevTargetButton)
	{
		PrevTargetButton->OnClicked.AddDynamic(this, &UMGSpectatorControlsWidget::OnPrevTargetClicked);
	}
	if (NextTargetButton)
	{
		NextTargetButton->OnClicked.AddDynamic(this, &UMGSpectatorControlsWidget::OnNextTargetClicked);
	}
	if (AutoDirectorButton)
	{
		AutoDirectorButton->OnClicked.AddDynamic(this, &UMGSpectatorControlsWidget::OnAutoDirectorClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMGSpectatorControlsWidget::OnExitClicked);
	}

	UpdateDisplay();
}

void UMGSpectatorControlsWidget::NativeDestruct()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->OnCameraModeChanged.RemoveDynamic(this, &UMGSpectatorControlsWidget::OnCameraModeChanged);
	}

	Super::NativeDestruct();
}

void UMGSpectatorControlsWidget::OnPrevTargetClicked()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->CyclePreviousTarget();
	}
}

void UMGSpectatorControlsWidget::OnNextTargetClicked()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->CycleNextTarget();
	}
}

void UMGSpectatorControlsWidget::OnAutoDirectorClicked()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->EnableAutoDirector(!SpectatorSubsystem->IsAutoDirectorEnabled());
	}
}

void UMGSpectatorControlsWidget::OnExitClicked()
{
	if (SpectatorSubsystem)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			SpectatorSubsystem->ExitSpectatorMode(PC);
		}
	}
}

void UMGSpectatorControlsWidget::OnCameraModeChanged(EMGSpectatorCameraMode NewMode)
{
	UpdateDisplay();
}

void UMGSpectatorControlsWidget::UpdateDisplay_Implementation()
{
	if (CameraModeText && SpectatorSubsystem)
	{
		FText ModeName = UMGSpectatorSubsystem::GetCameraModeDisplayName(SpectatorSubsystem->GetCameraMode());
		CameraModeText->SetText(ModeName);
	}
}

// ==========================================
// UMGSpectatorSpeedometerWidget
// ==========================================

void UMGSpectatorSpeedometerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Smooth speed display
	DisplaySpeed = FMath::FInterpTo(DisplaySpeed, CurrentSpeed, InDeltaTime, 10.0f);
	UpdateDisplay();
}

void UMGSpectatorSpeedometerWidget::SetSpeed(float SpeedKMH)
{
	CurrentSpeed = SpeedKMH;
}

void UMGSpectatorSpeedometerWidget::UpdateDisplay_Implementation()
{
	if (SpeedText)
	{
		SpeedText->SetText(FText::AsNumber(FMath::RoundToInt(DisplaySpeed)));
	}

	if (SpeedBar)
	{
		float SpeedPercent = FMath::Clamp(DisplaySpeed / MaxDisplaySpeed, 0.0f, 1.0f);
		SpeedBar->SetPercent(SpeedPercent);
	}
}

// ==========================================
// UMGSpectatorLapTimerWidget
// ==========================================

void UMGSpectatorLapTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateDisplay();
}

void UMGSpectatorLapTimerWidget::SetLapData(int32 Lap, int32 TotalLapsValue, float CurrentTime, float BestTime)
{
	CurrentLap = Lap;
	TotalLaps = TotalLapsValue;
	CurrentLapTime = CurrentTime;
	BestLapTime = BestTime;
}

void UMGSpectatorLapTimerWidget::UpdateDisplay_Implementation()
{
	if (CurrentLapText)
	{
		FText LapText = FText::Format(NSLOCTEXT("Spectator", "LapProgress", "LAP {0}/{1}"), FText::AsNumber(CurrentLap), FText::AsNumber(TotalLaps));
		CurrentLapText->SetText(LapText);
	}

	if (CurrentTimeText)
	{
		CurrentTimeText->SetText(FormatLapTime(CurrentLapTime));
	}

	if (BestTimeText && BestLapTime > 0.0f)
	{
		BestTimeText->SetText(FormatLapTime(BestLapTime));
		BestTimeText->SetVisibility(ESlateVisibility::Visible);
	}
	else if (BestTimeText)
	{
		BestTimeText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (DeltaTimeText && BestLapTime > 0.0f)
	{
		float Delta = CurrentLapTime - BestLapTime;
		FString DeltaString = Delta >= 0.0f ?
			FString::Printf(TEXT("+%.3f"), Delta) :
			FString::Printf(TEXT("%.3f"), Delta);
		DeltaTimeText->SetText(FText::FromString(DeltaString));

		// Color based on delta
		FSlateColor DeltaColor = Delta < 0.0f ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red);
		DeltaTimeText->SetColorAndOpacity(DeltaColor);
	}
}

FText UMGSpectatorLapTimerWidget::FormatLapTime(float TimeSeconds)
{
	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);

	FString TimeString = FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
	return FText::FromString(TimeString);
}

// ==========================================
// UMGCameraCutIndicatorWidget
// ==========================================

void UMGCameraCutIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();

		if (SpectatorSubsystem)
		{
			SpectatorSubsystem->OnCameraCut.AddDynamic(this, &UMGCameraCutIndicatorWidget::OnCameraCut);
		}
	}

	// Start hidden
	SetVisibility(ESlateVisibility::Collapsed);
}

void UMGCameraCutIndicatorWidget::NativeDestruct()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->OnCameraCut.RemoveDynamic(this, &UMGCameraCutIndicatorWidget::OnCameraCut);
	}

	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	Super::NativeDestruct();
}

void UMGCameraCutIndicatorWidget::ShowCutIndicator(const FMGCameraCut& CutInfo)
{
	if (CutInfoText)
	{
		FText FromName = UMGSpectatorSubsystem::GetCameraModeDisplayName(CutInfo.FromMode);
		FText ToName = UMGSpectatorSubsystem::GetCameraModeDisplayName(CutInfo.ToMode);
		CutInfoText->SetText(ToName);
	}

	SetVisibility(ESlateVisibility::Visible);
	PlayShowAnimation();

	// Schedule hide
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&UMGCameraCutIndicatorWidget::HideIndicator,
			DisplayDuration,
			false
		);
	}
}

void UMGCameraCutIndicatorWidget::OnCameraCut(const FMGCameraCut& CutInfo)
{
	ShowCutIndicator(CutInfo);
}

void UMGCameraCutIndicatorWidget::HideIndicator()
{
	PlayHideAnimation();
}

void UMGCameraCutIndicatorWidget::PlayShowAnimation_Implementation()
{
	// Override in Blueprint for animation
}

void UMGCameraCutIndicatorWidget::PlayHideAnimation_Implementation()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
