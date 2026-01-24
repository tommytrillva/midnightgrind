// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGWagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGWagerType : uint8
{
	Currency,
	Vehicle,
	Part,
	Cosmetic,
	Experience,
	Mixed
};

UENUM(BlueprintType)
enum class EMGWagerState : uint8
{
	Proposed,
	Accepted,
	Active,
	Completed,
	Cancelled,
	Disputed,
	Expired
};

UENUM(BlueprintType)
enum class EMGWagerOutcome : uint8
{
	Pending,
	WonByInitiator,
	WonByOpponent,
	Draw,
	Voided
};

USTRUCT(BlueprintType)
struct FMGWagerStake
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWagerType StakeType = EMGWagerType::Currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PartName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CosmeticID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CosmeticName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 EstimatedValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> StakeIcon;
};

USTRUCT(BlueprintType)
struct FMGWagerParticipant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerStake Stake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAccepted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStakeLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGWagerConditions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCatchUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCollisions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNightRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeatherCondition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRestrictVehicleClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClassRestriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPerformanceRating = 0;
};

USTRUCT(BlueprintType)
struct FMGWager
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid WagerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWagerState State = EMGWagerState::Proposed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWagerOutcome Outcome = EMGWagerOutcome::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerParticipant Initiator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerParticipant Opponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerConditions Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPinkSlip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WagerTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WagerDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WinnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceSessionID;
};

USTRUCT(BlueprintType)
struct FMGWagerHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid WagerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OpponentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OpponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWagerOutcome Outcome = EMGWagerOutcome::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerStake StakeWon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWagerStake StakeLost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasPinkSlip = false;
};

USTRUCT(BlueprintType)
struct FMGWagerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MinCurrencyWager = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MaxCurrencyWager = 1000000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WagerExpirationHours = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HouseFeePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPinkSlips = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLevelForPinkSlips = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowVehicleWagers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveWagers = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireStakeMatch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StakeMatchTolerance = 0.2f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWagerProposed, const FMGWager&, Wager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWagerAccepted, const FMGWager&, Wager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWagerDeclined, FGuid, WagerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWagerStarted, const FMGWager&, Wager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWagerCompleted, const FMGWager&, Wager, bool, bWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWagerCancelled, FGuid, WagerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStakeTransferred, const FMGWagerStake&, Stake, bool, bReceived);

UCLASS()
class MIDNIGHTGRIND_API UMGWagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Wager Creation
	UFUNCTION(BlueprintCallable, Category = "Wager|Create")
	FGuid ProposeWager(FName OpponentID, const FMGWagerStake& MyStake, const FMGWagerConditions& Conditions);

	UFUNCTION(BlueprintCallable, Category = "Wager|Create")
	FGuid ProposePinkSlipRace(FName OpponentID, FName MyVehicleID, const FMGWagerConditions& Conditions);

	UFUNCTION(BlueprintCallable, Category = "Wager|Create")
	FGuid ProposeCurrencyWager(FName OpponentID, int64 Amount, const FMGWagerConditions& Conditions);

	UFUNCTION(BlueprintPure, Category = "Wager|Create")
	bool CanCreateWager() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Create")
	bool CanProposePinkSlip() const;

	// Wager Response
	UFUNCTION(BlueprintCallable, Category = "Wager|Response")
	bool AcceptWager(FGuid WagerID, const FMGWagerStake& MyStake);

	UFUNCTION(BlueprintCallable, Category = "Wager|Response")
	bool DeclineWager(FGuid WagerID);

	UFUNCTION(BlueprintCallable, Category = "Wager|Response")
	bool CounterOffer(FGuid WagerID, const FMGWagerStake& CounterStake);

	UFUNCTION(BlueprintCallable, Category = "Wager|Response")
	bool CancelWager(FGuid WagerID);

	// Active Wagers
	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	TArray<FMGWager> GetPendingWagers() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	TArray<FMGWager> GetActiveWagers() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	TArray<FMGWager> GetIncomingWagers() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	TArray<FMGWager> GetOutgoingWagers() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	FMGWager GetWager(FGuid WagerID) const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	bool HasActiveWager() const;

	UFUNCTION(BlueprintPure, Category = "Wager|Active")
	FMGWager GetCurrentRaceWager() const;

	// Race Integration
	UFUNCTION(BlueprintCallable, Category = "Wager|Race")
	void StartWagerRace(FGuid WagerID);

	UFUNCTION(BlueprintCallable, Category = "Wager|Race")
	void ReportRaceResult(FGuid WagerID, FName WinnerID, int32 InitiatorPosition, int32 OpponentPosition);

	UFUNCTION(BlueprintCallable, Category = "Wager|Race")
	void DisputeResult(FGuid WagerID, const FString& Reason);

	// History
	UFUNCTION(BlueprintPure, Category = "Wager|History")
	TArray<FMGWagerHistory> GetWagerHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintPure, Category = "Wager|History")
	int32 GetTotalWagersWon() const;

	UFUNCTION(BlueprintPure, Category = "Wager|History")
	int32 GetTotalWagersLost() const;

	UFUNCTION(BlueprintPure, Category = "Wager|History")
	int64 GetTotalCurrencyWon() const;

	UFUNCTION(BlueprintPure, Category = "Wager|History")
	int64 GetTotalCurrencyLost() const;

	// Validation
	UFUNCTION(BlueprintPure, Category = "Wager|Validation")
	bool ValidateStake(const FMGWagerStake& Stake) const;

	UFUNCTION(BlueprintPure, Category = "Wager|Validation")
	bool DoStakesMatch(const FMGWagerStake& Stake1, const FMGWagerStake& Stake2) const;

	UFUNCTION(BlueprintPure, Category = "Wager|Validation")
	bool CanAffordStake(const FMGWagerStake& Stake) const;

	UFUNCTION(BlueprintPure, Category = "Wager|Validation")
	bool OwnsVehicle(FName VehicleID) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Wager|Config")
	void SetConfig(const FMGWagerConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Wager|Config")
	FMGWagerConfig GetConfig() const { return Config; }

	// Player
	UFUNCTION(BlueprintCallable, Category = "Wager|Player")
	void SetLocalPlayer(FName PlayerID, const FString& PlayerName, int32 PlayerLevel);

	// Network
	UFUNCTION(BlueprintCallable, Category = "Wager|Network")
	void ReceiveWagerProposal(const FMGWager& Wager);

	UFUNCTION(BlueprintCallable, Category = "Wager|Network")
	void ReceiveWagerAcceptance(FGuid WagerID);

	UFUNCTION(BlueprintCallable, Category = "Wager|Network")
	void ReceiveWagerDecline(FGuid WagerID);

	UFUNCTION(BlueprintCallable, Category = "Wager|Network")
	void ReceiveWagerResult(FGuid WagerID, FName WinnerID);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerProposed OnWagerProposed;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerAccepted OnWagerAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerDeclined OnWagerDeclined;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerStarted OnWagerStarted;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerCompleted OnWagerCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnWagerCancelled OnWagerCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Wager|Events")
	FOnStakeTransferred OnStakeTransferred;

protected:
	void CheckExpiredWagers();
	void ProcessWagerCompletion(FMGWager& Wager, FName WinnerID);
	void TransferStake(const FMGWagerStake& Stake, FName FromPlayer, FName ToPlayer);
	void LockStake(const FMGWagerStake& Stake);
	void UnlockStake(const FMGWagerStake& Stake);
	void AddToHistory(const FMGWager& Wager, bool bWon);
	void SaveWagerData();
	void LoadWagerData();

	UPROPERTY()
	TMap<FGuid, FMGWager> AllWagers;

	UPROPERTY()
	TArray<FMGWagerHistory> WagerHistory;

	UPROPERTY()
	FMGWagerConfig Config;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;

	UPROPERTY()
	int32 LocalPlayerLevel = 1;

	UPROPERTY()
	int32 TotalWon = 0;

	UPROPERTY()
	int32 TotalLost = 0;

	UPROPERTY()
	int64 CurrencyWon = 0;

	UPROPERTY()
	int64 CurrencyLost = 0;

	UPROPERTY()
	FGuid CurrentRaceWagerID;

	FTimerHandle ExpirationCheckHandle;
};
