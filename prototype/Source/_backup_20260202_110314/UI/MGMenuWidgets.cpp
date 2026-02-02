// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGMenuWidgets.h"
#include "UI/MGMenuSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGMainMenuWidget
// ==========================================

void UMGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		MenuSubsystem = GI->GetSubsystem<UMGMenuSubsystem>();
	}

	PlayIntroAnimation();
}

void UMGMainMenuWidget::OnQuickPlayClicked()
{
	OnMenuItemSelected(0);

	if (MenuSubsystem)
	{
		MenuSubsystem->StartGame();
	}
}

void UMGMainMenuWidget::OnGarageClicked()
{
	OnMenuItemSelected(1);
	// Would transition to garage screen
}

void UMGMainMenuWidget::OnMultiplayerClicked()
{
	OnMenuItemSelected(2);
	// Would show multiplayer menu
}

void UMGMainMenuWidget::OnSettingsClicked()
{
	OnMenuItemSelected(3);

	if (MenuSubsystem)
	{
		MenuSubsystem->ShowSettings();
	}
}

void UMGMainMenuWidget::OnQuitClicked()
{
	OnMenuItemSelected(4);
	PlayExitAnimation();

	if (MenuSubsystem)
	{
		MenuSubsystem->QuitGame();
	}
}

// ==========================================
// UMGPauseMenuWidget
// ==========================================

void UMGPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		MenuSubsystem = GI->GetSubsystem<UMGMenuSubsystem>();
	}
}

void UMGPauseMenuWidget::OnResumeClicked()
{
	if (MenuSubsystem)
	{
		MenuSubsystem->ResumeGame();
	}
}

void UMGPauseMenuWidget::OnRestartClicked()
{
	// Would restart current race
	if (MenuSubsystem)
	{
		MenuSubsystem->HidePauseMenu();
	}

	// UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void UMGPauseMenuWidget::OnSettingsClicked()
{
	if (MenuSubsystem)
	{
		MenuSubsystem->ShowSettings();
	}
}

void UMGPauseMenuWidget::OnMainMenuClicked()
{
	ShowExitConfirmation();
}

// ==========================================
// UMGSettingsWidget
// ==========================================

void UMGSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		MenuSubsystem = GI->GetSubsystem<UMGMenuSubsystem>();
	}

	if (MenuSubsystem)
	{
		PendingSettings = MenuSubsystem->GetSettings();
		OriginalSettings = PendingSettings;
	}
}

void UMGSettingsWidget::SetInitialCategory(EMGSettingsCategory Category)
{
	CurrentCategory = Category;
	OnCategoryChanged(Category);
}

void UMGSettingsWidget::ShowGraphicsSettings()
{
	CurrentCategory = EMGSettingsCategory::Graphics;
	OnCategoryChanged(CurrentCategory);
}

void UMGSettingsWidget::ShowAudioSettings()
{
	CurrentCategory = EMGSettingsCategory::Audio;
	OnCategoryChanged(CurrentCategory);
}

void UMGSettingsWidget::ShowControlsSettings()
{
	CurrentCategory = EMGSettingsCategory::Controls;
	OnCategoryChanged(CurrentCategory);
}

void UMGSettingsWidget::ShowGameplaySettings()
{
	CurrentCategory = EMGSettingsCategory::Gameplay;
	OnCategoryChanged(CurrentCategory);
}

void UMGSettingsWidget::ShowAccessibilitySettings()
{
	CurrentCategory = EMGSettingsCategory::Accessibility;
	OnCategoryChanged(CurrentCategory);
}

void UMGSettingsWidget::ApplySettings()
{
	if (MenuSubsystem)
	{
		MenuSubsystem->ApplyAndSaveSettings(PendingSettings);
		OriginalSettings = PendingSettings;
	}
}

void UMGSettingsWidget::RevertSettings()
{
	PendingSettings = OriginalSettings;
	OnSettingsModified();
}

void UMGSettingsWidget::ResetToDefaults()
{
	PendingSettings = FMGGameSettings();
	OnSettingsModified();
}

void UMGSettingsWidget::CloseSettings()
{
	// Check for unsaved changes
	bool bHasChanges = false;

	// Simple comparison - in practice would check each field
	bHasChanges = (PendingSettings.MasterVolume != OriginalSettings.MasterVolume ||
		PendingSettings.GraphicsQuality != OriginalSettings.GraphicsQuality);

	if (bHasChanges)
	{
		ShowUnsavedChangesDialog();
	}
	else
	{
		if (MenuSubsystem)
		{
			MenuSubsystem->HideSettings();
		}
	}
}

void UMGSettingsWidget::UpdatePendingSettings(const FMGGameSettings& NewSettings)
{
	PendingSettings = NewSettings;
	OnSettingsModified();
}

// ==========================================
// UMGLoadingScreenWidget
// ==========================================

void UMGLoadingScreenWidget::SetLoadingText(const FText& Text)
{
	LoadingText = Text;
	OnTextChanged(Text);
}

void UMGLoadingScreenWidget::SetProgress(float Progress)
{
	CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	OnProgressUpdated(CurrentProgress);
}

void UMGLoadingScreenWidget::SetLoadingTip(const FText& Tip)
{
	CurrentTip = Tip;
}

// ==========================================
// UMGResultsWidget
// ==========================================

void UMGResultsWidget::ShowResults(int32 Position, float TotalTime, float BestLapTime, int32 CashEarned, int32 ReputationEarned)
{
	FinalPosition = Position;
	FinalTime = TotalTime;
	BestLap = BestLapTime;
	Cash = CashEarned;
	Reputation = ReputationEarned;

	PlayResultsAnimation();
}

void UMGResultsWidget::OnContinueClicked()
{
	// Return to lobby or main menu
}

void UMGResultsWidget::OnReplayClicked()
{
	// Open replay viewer
}

void UMGResultsWidget::OnRematchClicked()
{
	// Request rematch
}

// ==========================================
// UMGNotificationWidget
// ==========================================

void UMGNotificationWidget::ShowNotification(const FText& Title, const FText& Message, UTexture2D* Icon)
{
	NotificationTitle = Title;
	NotificationMessage = Message;
	NotificationIcon = Icon;

	PlayShowAnimation();
}

void UMGNotificationWidget::HideNotification()
{
	PlayHideAnimation();
}
