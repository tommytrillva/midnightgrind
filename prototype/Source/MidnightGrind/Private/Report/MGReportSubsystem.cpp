// Copyright Midnight Grind. All Rights Reserved.

#include "Report/MGReportSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGReportSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize base filtered words (placeholder - would be loaded from config)
	BaseFilteredWords.Add(TEXT("cheat"));
	BaseFilteredWords.Add(TEXT("hack"));

	// Default filter config
	FilterConfig.bFilterProfanity = true;
	FilterConfig.bFilterSlurs = true;
	FilterConfig.bFilterSpam = true;
	FilterConfig.FilterLevel = 2;

	LoadReportData();

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReportTickHandle,
			this,
			&UMGReportSubsystem::OnReportTick,
			60.0f, // Check every minute
			true
		);
	}
}

void UMGReportSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReportTickHandle);
	}

	SaveReportData();
	Super::Deinitialize();
}

bool UMGReportSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Reporting
// ============================================================================

FGuid UMGReportSubsystem::SubmitReport(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, FName MatchID)
{
	return SubmitReportWithEvidence(ReportedPlayerID, Category, Description, TArray<FString>(), MatchID);
}

FGuid UMGReportSubsystem::SubmitReportWithEvidence(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, const TArray<FString>& EvidenceURLs, FName MatchID)
{
	if (!CanReportPlayer(ReportedPlayerID))
	{
		return FGuid();
	}

	FMGPlayerReport NewReport;
	NewReport.ReportID = FGuid::NewGuid();
	NewReport.ReportedPlayerID = ReportedPlayerID;
	NewReport.Category = Category;
	NewReport.Description = Description;
	NewReport.Status = EMGReportStatus::Pending;
	NewReport.ReportedAt = FDateTime::UtcNow();
	NewReport.MatchID = MatchID;
	NewReport.EvidenceURLs = EvidenceURLs;

	// Set severity based on category
	switch (Category)
	{
	case EMGReportCategory::Cheating:
	case EMGReportCategory::Hacking:
	case EMGReportCategory::HateSpeech:
		NewReport.Severity = EMGReportSeverity::High;
		break;
	case EMGReportCategory::Harassment:
	case EMGReportCategory::Exploiting:
	case EMGReportCategory::RealMoneyTrading:
		NewReport.Severity = EMGReportSeverity::Medium;
		break;
	default:
		NewReport.Severity = EMGReportSeverity::Low;
		break;
	}

	SubmittedReports.Add(NewReport);
	ReportsSubmittedToday++;

	// Track reports per player
	int32& PlayerReportCount = ReportsPerPlayer.FindOrAdd(ReportedPlayerID);
	PlayerReportCount++;

	Stats.TotalReportsSubmitted++;

	OnReportSubmitted.Broadcast(NewReport.ReportID);
	SaveReportData();

	return NewReport.ReportID;
}

bool UMGReportSubsystem::CanReportPlayer(FName PlayerID) const
{
	if (PlayerID.IsNone())
	{
		return false;
	}

	// Check daily limit
	if (ReportsSubmittedToday >= DailyReportLimit)
	{
		return false;
	}

	// Check if we've already reported this player recently
	for (const FMGPlayerReport& Report : SubmittedReports)
	{
		if (Report.ReportedPlayerID == PlayerID && Report.Status == EMGReportStatus::Pending)
		{
			// Already have a pending report for this player
			return false;
		}
	}

	return true;
}

int32 UMGReportSubsystem::GetRemainingReportsToday() const
{
	return FMath::Max(0, DailyReportLimit - ReportsSubmittedToday);
}

TArray<FMGPlayerReport> UMGReportSubsystem::GetSubmittedReports() const
{
	return SubmittedReports;
}

FMGPlayerReport UMGReportSubsystem::GetReport(FGuid ReportID) const
{
	for (const FMGPlayerReport& Report : SubmittedReports)
	{
		if (Report.ReportID == ReportID)
		{
			return Report;
		}
	}

	return FMGPlayerReport();
}

// ============================================================================
// Report Categories
// ============================================================================

TArray<EMGReportCategory> UMGReportSubsystem::GetAvailableCategories() const
{
	return {
		EMGReportCategory::Cheating,
		EMGReportCategory::Hacking,
		EMGReportCategory::Griefing,
		EMGReportCategory::Harassment,
		EMGReportCategory::HateSpeech,
		EMGReportCategory::InappropriateName,
		EMGReportCategory::InappropriateContent,
		EMGReportCategory::Boosting,
		EMGReportCategory::AFKAbuse,
		EMGReportCategory::Exploiting,
		EMGReportCategory::Spam,
		EMGReportCategory::Other
	};
}

FText UMGReportSubsystem::GetCategoryDisplayName(EMGReportCategory Category) const
{
	switch (Category)
	{
	case EMGReportCategory::Cheating:
		return FText::FromString(TEXT("Cheating"));
	case EMGReportCategory::Hacking:
		return FText::FromString(TEXT("Hacking"));
	case EMGReportCategory::Griefing:
		return FText::FromString(TEXT("Griefing"));
	case EMGReportCategory::Harassment:
		return FText::FromString(TEXT("Harassment"));
	case EMGReportCategory::HateSpeech:
		return FText::FromString(TEXT("Hate Speech"));
	case EMGReportCategory::InappropriateName:
		return FText::FromString(TEXT("Inappropriate Name"));
	case EMGReportCategory::InappropriateContent:
		return FText::FromString(TEXT("Inappropriate Content"));
	case EMGReportCategory::Boosting:
		return FText::FromString(TEXT("Boosting/Win Trading"));
	case EMGReportCategory::AFKAbuse:
		return FText::FromString(TEXT("AFK/Idle Abuse"));
	case EMGReportCategory::Exploiting:
		return FText::FromString(TEXT("Exploiting Bugs"));
	case EMGReportCategory::RealMoneyTrading:
		return FText::FromString(TEXT("Real Money Trading"));
	case EMGReportCategory::Impersonation:
		return FText::FromString(TEXT("Impersonation"));
	case EMGReportCategory::Spam:
		return FText::FromString(TEXT("Spam"));
	case EMGReportCategory::Other:
		return FText::FromString(TEXT("Other"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGReportSubsystem::GetCategoryDescription(EMGReportCategory Category) const
{
	switch (Category)
	{
	case EMGReportCategory::Cheating:
		return FText::FromString(TEXT("Using unfair advantages like aimbots or speed hacks"));
	case EMGReportCategory::Hacking:
		return FText::FromString(TEXT("Modifying game files or using external tools"));
	case EMGReportCategory::Griefing:
		return FText::FromString(TEXT("Intentionally sabotaging teammates or races"));
	case EMGReportCategory::Harassment:
		return FText::FromString(TEXT("Targeting a player with unwanted behavior"));
	case EMGReportCategory::HateSpeech:
		return FText::FromString(TEXT("Discriminatory language or symbols"));
	case EMGReportCategory::InappropriateName:
		return FText::FromString(TEXT("Offensive player name or crew tag"));
	case EMGReportCategory::Boosting:
		return FText::FromString(TEXT("Artificially inflating stats or rank"));
	default:
		return FText::GetEmpty();
	}
}

// ============================================================================
// Muting
// ============================================================================

void UMGReportSubsystem::MutePlayer(FName PlayerID, bool bMuteVoice, bool bMuteText)
{
	if (PlayerID.IsNone())
	{
		return;
	}

	FMGMutedPlayer MutedPlayer;
	MutedPlayer.PlayerID = PlayerID;
	MutedPlayer.Reason = EMGMuteReason::Manual;
	MutedPlayer.MutedAt = FDateTime::UtcNow();
	MutedPlayer.bMuteVoice = bMuteVoice;
	MutedPlayer.bMuteText = bMuteText;

	MutedPlayers.Add(PlayerID, MutedPlayer);
	Stats.PlayersMuted++;

	OnPlayerMuted.Broadcast(MutedPlayer);
	SaveReportData();
}

void UMGReportSubsystem::UnmutePlayer(FName PlayerID)
{
	if (MutedPlayers.Contains(PlayerID))
	{
		MutedPlayers.Remove(PlayerID);
		Stats.PlayersMuted = FMath::Max(0, Stats.PlayersMuted - 1);
		OnPlayerUnmuted.Broadcast(PlayerID);
		SaveReportData();
	}
}

bool UMGReportSubsystem::IsPlayerMuted(FName PlayerID) const
{
	if (bMuteAllVoice || bMuteAllText)
	{
		return true;
	}

	return MutedPlayers.Contains(PlayerID);
}

FMGMutedPlayer UMGReportSubsystem::GetMutedPlayerInfo(FName PlayerID) const
{
	if (const FMGMutedPlayer* Muted = MutedPlayers.Find(PlayerID))
	{
		return *Muted;
	}

	return FMGMutedPlayer();
}

TArray<FMGMutedPlayer> UMGReportSubsystem::GetMutedPlayers() const
{
	TArray<FMGMutedPlayer> Result;
	MutedPlayers.GenerateValueArray(Result);
	return Result;
}

void UMGReportSubsystem::MuteAll(bool bMuteVoice, bool bMuteText)
{
	bMuteAllVoice = bMuteVoice;
	bMuteAllText = bMuteText;
	SaveReportData();
}

void UMGReportSubsystem::UnmuteAll()
{
	bMuteAllVoice = false;
	bMuteAllText = false;
	SaveReportData();
}

// ============================================================================
// Blocking
// ============================================================================

void UMGReportSubsystem::BlockPlayer(FName PlayerID, bool bPreventMatching)
{
	if (PlayerID.IsNone())
	{
		return;
	}

	FMGBlockedPlayer BlockedPlayer;
	BlockedPlayer.PlayerID = PlayerID;
	BlockedPlayer.BlockedAt = FDateTime::UtcNow();
	BlockedPlayer.bPreventMatching = bPreventMatching;
	BlockedPlayer.bHideContent = true;

	BlockedPlayers.Add(PlayerID, BlockedPlayer);
	Stats.PlayersBlocked++;

	OnPlayerBlocked.Broadcast(BlockedPlayer);
	SaveReportData();
}

void UMGReportSubsystem::UnblockPlayer(FName PlayerID)
{
	if (BlockedPlayers.Contains(PlayerID))
	{
		BlockedPlayers.Remove(PlayerID);
		Stats.PlayersBlocked = FMath::Max(0, Stats.PlayersBlocked - 1);
		OnPlayerUnblocked.Broadcast(PlayerID);
		SaveReportData();
	}
}

bool UMGReportSubsystem::IsPlayerBlocked(FName PlayerID) const
{
	return BlockedPlayers.Contains(PlayerID);
}

TArray<FMGBlockedPlayer> UMGReportSubsystem::GetBlockedPlayers() const
{
	TArray<FMGBlockedPlayer> Result;
	BlockedPlayers.GenerateValueArray(Result);
	return Result;
}

bool UMGReportSubsystem::ShouldPreventMatching(FName PlayerID) const
{
	if (const FMGBlockedPlayer* Blocked = BlockedPlayers.Find(PlayerID))
	{
		return Blocked->bPreventMatching;
	}

	return false;
}

// ============================================================================
// Punishments
// ============================================================================

bool UMGReportSubsystem::HasActivePunishment() const
{
	return ActivePunishments.Num() > 0;
}

TArray<FMGPunishment> UMGReportSubsystem::GetActivePunishments() const
{
	return ActivePunishments;
}

FMGPunishment UMGReportSubsystem::GetPunishment(FGuid PunishmentID) const
{
	for (const FMGPunishment& Punishment : ActivePunishments)
	{
		if (Punishment.PunishmentID == PunishmentID)
		{
			return Punishment;
		}
	}

	for (const FMGPunishment& Punishment : PunishmentHistory)
	{
		if (Punishment.PunishmentID == PunishmentID)
		{
			return Punishment;
		}
	}

	return FMGPunishment();
}

bool UMGReportSubsystem::IsBanned() const
{
	for (const FMGPunishment& Punishment : ActivePunishments)
	{
		if (Punishment.bIsActive &&
			(Punishment.Type == EMGPunishmentType::TempBan || Punishment.Type == EMGPunishmentType::PermBan))
		{
			return true;
		}
	}

	return false;
}

bool UMGReportSubsystem::IsMuted() const
{
	for (const FMGPunishment& Punishment : ActivePunishments)
	{
		if (Punishment.bIsActive &&
			(Punishment.Type == EMGPunishmentType::TempMute || Punishment.Type == EMGPunishmentType::PermMute))
		{
			return true;
		}
	}

	return false;
}

FDateTime UMGReportSubsystem::GetBanExpirationTime() const
{
	for (const FMGPunishment& Punishment : ActivePunishments)
	{
		if (Punishment.bIsActive && !Punishment.bIsPermanent &&
			(Punishment.Type == EMGPunishmentType::TempBan))
		{
			return Punishment.ExpiresAt;
		}
	}

	return FDateTime::MinValue();
}

float UMGReportSubsystem::GetBanTimeRemaining() const
{
	FDateTime ExpiresAt = GetBanExpirationTime();
	if (ExpiresAt == FDateTime::MinValue())
	{
		return -1.0f; // Permanent or no ban
	}

	FTimespan Remaining = ExpiresAt - FDateTime::UtcNow();
	return static_cast<float>(Remaining.GetTotalSeconds());
}

bool UMGReportSubsystem::AppealPunishment(FGuid PunishmentID, const FText& AppealReason)
{
	for (FMGPunishment& Punishment : ActivePunishments)
	{
		if (Punishment.PunishmentID == PunishmentID && !Punishment.bAppealed)
		{
			Punishment.bAppealed = true;
			// Appeal would be sent to server
			SaveReportData();
			return true;
		}
	}

	return false;
}

// ============================================================================
// Content Filtering
// ============================================================================

FString UMGReportSubsystem::FilterText(const FString& InputText) const
{
	if (!FilterConfig.bFilterProfanity && !FilterConfig.bFilterSlurs)
	{
		return InputText;
	}

	FString FilteredText = InputText;

	// Split into words
	TArray<FString> Words;
	InputText.ParseIntoArray(Words, TEXT(" "), true);

	for (const FString& Word : Words)
	{
		if (IsWordFiltered(Word))
		{
			// Replace with asterisks
			FString Replacement;
			for (int32 i = 0; i < Word.Len(); ++i)
			{
				Replacement.AppendChar(TEXT('*'));
			}
			FilteredText = FilteredText.Replace(*Word, *Replacement);
		}
	}

	return FilteredText;
}

bool UMGReportSubsystem::ContainsFilteredContent(const FString& Text) const
{
	TArray<FString> Words;
	Text.ParseIntoArray(Words, TEXT(" "), true);

	for (const FString& Word : Words)
	{
		if (IsWordFiltered(Word))
		{
			return true;
		}
	}

	return false;
}

void UMGReportSubsystem::SetContentFilterConfig(const FMGContentFilterConfig& Config)
{
	FilterConfig = Config;
	SaveReportData();
}

void UMGReportSubsystem::AddCustomBlockedWord(const FString& Word)
{
	if (!FilterConfig.CustomBlockedWords.Contains(Word))
	{
		FilterConfig.CustomBlockedWords.Add(Word);
		SaveReportData();
	}
}

void UMGReportSubsystem::RemoveCustomBlockedWord(const FString& Word)
{
	FilterConfig.CustomBlockedWords.Remove(Word);
	SaveReportData();
}

// ============================================================================
// Feedback
// ============================================================================

void UMGReportSubsystem::ReceiveReportFeedback(const FMGReportFeedback& Feedback)
{
	ReportFeedback.Add(Feedback);

	// Update report status
	for (FMGPlayerReport& Report : SubmittedReports)
	{
		if (Report.ReportID == Feedback.ReportID)
		{
			Report.Status = Feedback.bActionTaken ? EMGReportStatus::ActionTaken : EMGReportStatus::Dismissed;
			Report.ReviewedAt = FDateTime::UtcNow();

			if (Feedback.bActionTaken)
			{
				Stats.ReportsResultingInAction++;
			}
			break;
		}
	}

	// Update accuracy score
	if (Stats.TotalReportsSubmitted > 0)
	{
		Stats.ReportAccuracyScore = static_cast<float>(Stats.ReportsResultingInAction) / static_cast<float>(Stats.TotalReportsSubmitted);
	}

	OnReportFeedbackReceived.Broadcast(Feedback);
	SaveReportData();
}

TArray<FMGReportFeedback> UMGReportSubsystem::GetRecentFeedback(int32 MaxEntries) const
{
	if (MaxEntries <= 0 || MaxEntries >= ReportFeedback.Num())
	{
		return ReportFeedback;
	}

	TArray<FMGReportFeedback> Recent;
	int32 StartIndex = FMath::Max(0, ReportFeedback.Num() - MaxEntries);
	for (int32 i = StartIndex; i < ReportFeedback.Num(); ++i)
	{
		Recent.Add(ReportFeedback[i]);
	}

	return Recent;
}

// ============================================================================
// Network
// ============================================================================

void UMGReportSubsystem::ReceivePunishment(const FMGPunishment& Punishment)
{
	ActivePunishments.Add(Punishment);
	Stats.PunishmentsReceived++;
	Stats.TotalReportsReceived++;

	OnPunishmentReceived.Broadcast(Punishment);
	SaveReportData();
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGReportSubsystem::OnReportTick()
{
	// Reset daily reports at midnight
	FDateTime Today = FDateTime::UtcNow();
	if (Today.GetDate() != LastReportResetDate.GetDate())
	{
		ReportsSubmittedToday = 0;
		LastReportResetDate = Today;
	}

	// Check punishment expiration
	CheckPunishmentExpiration();
}

void UMGReportSubsystem::CheckPunishmentExpiration()
{
	FDateTime Now = FDateTime::UtcNow();

	for (int32 i = ActivePunishments.Num() - 1; i >= 0; --i)
	{
		FMGPunishment& Punishment = ActivePunishments[i];

		if (!Punishment.bIsPermanent && Punishment.ExpiresAt < Now)
		{
			Punishment.bIsActive = false;
			PunishmentHistory.Add(Punishment);
			OnPunishmentExpired.Broadcast(Punishment);
			ActivePunishments.RemoveAt(i);
		}
	}
}

bool UMGReportSubsystem::IsWordFiltered(const FString& Word) const
{
	FString LowerWord = Word.ToLower();

	// Check allowed words first (whitelist)
	if (FilterConfig.AllowedWords.Contains(LowerWord))
	{
		return false;
	}

	// Check base filtered words
	for (const FString& FilteredWord : BaseFilteredWords)
	{
		if (LowerWord.Contains(FilteredWord.ToLower()))
		{
			return true;
		}
	}

	// Check custom blocked words
	for (const FString& BlockedWord : FilterConfig.CustomBlockedWords)
	{
		if (LowerWord.Contains(BlockedWord.ToLower()))
		{
			return true;
		}
	}

	return false;
}

void UMGReportSubsystem::SaveReportData()
{
	// Persist report data
	// Implementation would use USaveGame or cloud save
}

void UMGReportSubsystem::LoadReportData()
{
	// Load persisted report data
	// Implementation would use USaveGame or cloud save
}
