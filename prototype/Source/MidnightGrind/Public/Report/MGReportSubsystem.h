// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReportSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGReportCategory : uint8
{
	None,
	Cheating,
	Hacking,
	Griefing,
	Harassment,
	HateSpeech,
	InappropriateName,
	InappropriateContent,
	Boosting,
	AFKAbuse,
	Exploiting,
	RealMoneyTrading,
	Impersonation,
	Spam,
	Other
};

UENUM(BlueprintType)
enum class EMGReportStatus : uint8
{
	Pending,
	UnderReview,
	ActionTaken,
	Dismissed,
	Duplicate,
	InsufficientEvidence
};

UENUM(BlueprintType)
enum class EMGReportSeverity : uint8
{
	Low,
	Medium,
	High,
	Critical
};

UENUM(BlueprintType)
enum class EMGPunishmentType : uint8
{
	None,
	Warning,
	TempMute,
	PermMute,
	TempBan,
	PermBan,
	CompetitiveBan,
	ResetProgress,
	RemoveContent,
	ShadowBan
};

UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
	None,
	Manual,
	Reported,
	AutoDetected,
	ContentFilter
};

USTRUCT(BlueprintType)
struct FMGPlayerReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReportID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReporterPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReporterPlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReportedPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReportedPlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportCategory Category = EMGReportCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportSeverity Severity = EMGReportSeverity::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportStatus Status = EMGReportStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReportedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReviewedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MatchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EvidenceURLs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ChatLogs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Metadata;
};

USTRUCT(BlueprintType)
struct FMGPunishment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PunishmentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPunishmentType Type = EMGPunishmentType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportCategory RelatedCategory = EMGReportCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Reason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime IssuedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAppealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RelatedReportID;
};

USTRUCT(BlueprintType)
struct FMGMutedPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMuteReason Reason = EMGMuteReason::Manual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime MutedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteVoice = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteText = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideEmotes = false;
};

USTRUCT(BlueprintType)
struct FMGBlockedPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BlockedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPreventMatching = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideContent = true;
};

USTRUCT(BlueprintType)
struct FMGReportFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReportID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActionTaken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FeedbackMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReceivedAt;
};

USTRUCT(BlueprintType)
struct FMGReportingStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReportsSubmitted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReportsResultingInAction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReportsReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PunishmentsReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReportAccuracyScore = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayersBlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayersMuted = 0;
};

USTRUCT(BlueprintType)
struct FMGContentFilterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterProfanity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterSlurs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterSpam = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterLinks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FilterLevel = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CustomBlockedWords;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedWords;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportSubmitted, FGuid, ReportID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportFeedbackReceived, const FMGReportFeedback&, Feedback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPunishmentReceived, const FMGPunishment&, Punishment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPunishmentExpired, const FMGPunishment&, Punishment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMuted, const FMGMutedPlayer&, MutedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUnmuted, FName, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBlocked, const FMGBlockedPlayer&, BlockedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUnblocked, FName, PlayerID);

UCLASS()
class MIDNIGHTGRIND_API UMGReportSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Reporting
	UFUNCTION(BlueprintCallable, Category = "Report|Submit")
	FGuid SubmitReport(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, FName MatchID = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Report|Submit")
	FGuid SubmitReportWithEvidence(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, const TArray<FString>& EvidenceURLs, FName MatchID = NAME_None);

	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	bool CanReportPlayer(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	int32 GetRemainingReportsToday() const;

	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	TArray<FMGPlayerReport> GetSubmittedReports() const;

	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	FMGPlayerReport GetReport(FGuid ReportID) const;

	// Report Categories
	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	TArray<EMGReportCategory> GetAvailableCategories() const;

	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	FText GetCategoryDisplayName(EMGReportCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	FText GetCategoryDescription(EMGReportCategory Category) const;

	// Muting
	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void MutePlayer(FName PlayerID, bool bMuteVoice = true, bool bMuteText = true);

	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void UnmutePlayer(FName PlayerID);

	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	bool IsPlayerMuted(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	FMGMutedPlayer GetMutedPlayerInfo(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	TArray<FMGMutedPlayer> GetMutedPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void MuteAll(bool bMuteVoice = true, bool bMuteText = true);

	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void UnmuteAll();

	// Blocking
	UFUNCTION(BlueprintCallable, Category = "Report|Block")
	void BlockPlayer(FName PlayerID, bool bPreventMatching = true);

	UFUNCTION(BlueprintCallable, Category = "Report|Block")
	void UnblockPlayer(FName PlayerID);

	UFUNCTION(BlueprintPure, Category = "Report|Block")
	bool IsPlayerBlocked(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Report|Block")
	TArray<FMGBlockedPlayer> GetBlockedPlayers() const;

	UFUNCTION(BlueprintPure, Category = "Report|Block")
	bool ShouldPreventMatching(FName PlayerID) const;

	// Punishments
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool HasActivePunishment() const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	TArray<FMGPunishment> GetActivePunishments() const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	FMGPunishment GetPunishment(FGuid PunishmentID) const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool IsBanned() const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool IsMuted() const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	FDateTime GetBanExpirationTime() const;

	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	float GetBanTimeRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Report|Punishment")
	bool AppealPunishment(FGuid PunishmentID, const FText& AppealReason);

	// Content Filtering
	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	FString FilterText(const FString& InputText) const;

	UFUNCTION(BlueprintPure, Category = "Report|Filter")
	bool ContainsFilteredContent(const FString& Text) const;

	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void SetContentFilterConfig(const FMGContentFilterConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Report|Filter")
	FMGContentFilterConfig GetContentFilterConfig() const { return FilterConfig; }

	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void AddCustomBlockedWord(const FString& Word);

	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void RemoveCustomBlockedWord(const FString& Word);

	// Feedback
	UFUNCTION(BlueprintCallable, Category = "Report|Feedback")
	void ReceiveReportFeedback(const FMGReportFeedback& Feedback);

	UFUNCTION(BlueprintPure, Category = "Report|Feedback")
	TArray<FMGReportFeedback> GetRecentFeedback(int32 MaxEntries = 10) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Report|Stats")
	FMGReportingStats GetReportingStats() const { return Stats; }

	// Receive from server
	UFUNCTION(BlueprintCallable, Category = "Report|Network")
	void ReceivePunishment(const FMGPunishment& Punishment);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnReportSubmitted OnReportSubmitted;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnReportFeedbackReceived OnReportFeedbackReceived;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPunishmentReceived OnPunishmentReceived;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPunishmentExpired OnPunishmentExpired;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerMuted OnPlayerMuted;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerUnmuted OnPlayerUnmuted;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerBlocked OnPlayerBlocked;

	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerUnblocked OnPlayerUnblocked;

protected:
	void OnReportTick();
	void CheckPunishmentExpiration();
	bool IsWordFiltered(const FString& Word) const;
	void SaveReportData();
	void LoadReportData();

	UPROPERTY()
	TArray<FMGPlayerReport> SubmittedReports;

	UPROPERTY()
	TArray<FMGPunishment> ActivePunishments;

	UPROPERTY()
	TArray<FMGPunishment> PunishmentHistory;

	UPROPERTY()
	TMap<FName, FMGMutedPlayer> MutedPlayers;

	UPROPERTY()
	TMap<FName, FMGBlockedPlayer> BlockedPlayers;

	UPROPERTY()
	TArray<FMGReportFeedback> ReportFeedback;

	UPROPERTY()
	FMGReportingStats Stats;

	UPROPERTY()
	FMGContentFilterConfig FilterConfig;

	UPROPERTY()
	TArray<FString> BaseFilteredWords;

	UPROPERTY()
	TMap<FName, int32> ReportsPerPlayer;

	UPROPERTY()
	int32 DailyReportLimit = 10;

	UPROPERTY()
	int32 ReportsSubmittedToday = 0;

	UPROPERTY()
	FDateTime LastReportResetDate;

	UPROPERTY()
	bool bMuteAllVoice = false;

	UPROPERTY()
	bool bMuteAllText = false;

	FTimerHandle ReportTickHandle;
};
