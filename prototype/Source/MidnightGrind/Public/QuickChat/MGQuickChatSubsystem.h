// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGQuickChatSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGQuickChatCategory : uint8
{
	Greetings,
	Racing,
	TeamTactics,
	Reactions,
	Compliments,
	Taunts,
	Callouts,
	Custom
};

UENUM(BlueprintType)
enum class EMGQuickChatVisibility : uint8
{
	All,
	TeamOnly,
	NearbyOnly,
	Private
};

UENUM(BlueprintType)
enum class EMGPingType : uint8
{
	Location,
	Warning,
	Shortcut,
	Police,
	Obstacle,
	Opponent,
	Help,
	Custom
};

USTRUCT(BlueprintType)
struct FMGQuickChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MessageID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatCategory Category = EMGQuickChatCategory::Greetings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LocalizedText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> VoiceLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AudioEventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockCost = 0;
};

USTRUCT(BlueprintType)
struct FMGChatEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SenderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGQuickChatMessage Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SenderLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamID = -1;
};

USTRUCT(BlueprintType)
struct FMGWorldPing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OwnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPingType PingType = EMGPingType::Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PingColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PingLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::TeamOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FMGQuickChatWheel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WheelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WheelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGQuickChatMessage> Messages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 8;
};

USTRUCT(BlueprintType)
struct FMGQuickChatConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MessageCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingsPerPlayer = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingDefaultDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearbyRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayVoiceLines = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoiceLineVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowChatBubbles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChatBubbleDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteOpponents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> MutedPlayers;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickChatReceived, const FMGChatEvent&, ChatEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPingCreated, const FMGWorldPing&, Ping);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPingExpired, const FMGWorldPing&, Ping);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatCooldownStarted, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatCooldownEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickChatUnlocked, FName, MessageID, EMGQuickChatCategory, Category);

UCLASS()
class MIDNIGHTGRIND_API UMGQuickChatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Send Messages
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendQuickChat(FName MessageID, EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendQuickChatFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendCustomMessage(const FText& Text, EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All);

	UFUNCTION(BlueprintPure, Category = "QuickChat|Send")
	bool CanSendMessage() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Send")
	float GetMessageCooldownRemaining() const;

	// Pings
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	FGuid CreatePing(FVector Location, EMGPingType PingType = EMGPingType::Location);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	FGuid CreateDirectionalPing(FVector Location, FVector Direction, EMGPingType PingType);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	void RemovePing(FGuid PingID);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	void RemoveAllMyPings();

	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	bool CanCreatePing() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	TArray<FMGWorldPing> GetActivePings() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	TArray<FMGWorldPing> GetMyPings() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	float GetPingCooldownRemaining() const;

	// Message Library
	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetMessagesByCategory(EMGQuickChatCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetAllMessages() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetUnlockedMessages() const;

	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	FMGQuickChatMessage GetMessage(FName MessageID) const;

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Library")
	bool UnlockMessage(FName MessageID);

	// Chat Wheel
	UFUNCTION(BlueprintPure, Category = "QuickChat|Wheel")
	FMGQuickChatWheel GetActiveWheel() const { return ActiveWheel; }

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void SetActiveWheel(FName WheelID);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void AssignMessageToSlot(FName MessageID, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void ClearSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "QuickChat|Wheel")
	FMGQuickChatMessage GetMessageAtSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void SaveWheelConfiguration();

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void LoadWheelConfiguration();

	// Chat History
	UFUNCTION(BlueprintPure, Category = "QuickChat|History")
	TArray<FMGChatEvent> GetChatHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintCallable, Category = "QuickChat|History")
	void ClearChatHistory();

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void SetConfig(const FMGQuickChatConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "QuickChat|Config")
	FMGQuickChatConfig GetConfig() const { return Config; }

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void MutePlayer(FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void UnmutePlayer(FName PlayerID);

	UFUNCTION(BlueprintPure, Category = "QuickChat|Config")
	bool IsPlayerMuted(FName PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void SetVoiceLinesEnabled(bool bEnabled);

	// Player Info
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Player")
	void SetLocalPlayerInfo(FName PlayerID, const FString& PlayerName, int32 TeamID);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Player")
	void SetLocalPlayerLocation(FVector Location);

	// Network Receive
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Network")
	void ReceiveQuickChat(const FMGChatEvent& ChatEvent);

	UFUNCTION(BlueprintCallable, Category = "QuickChat|Network")
	void ReceivePing(const FMGWorldPing& Ping);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnQuickChatReceived OnQuickChatReceived;

	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnPingCreated OnPingCreated;

	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnPingExpired OnPingExpired;

	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnChatCooldownStarted OnChatCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnChatCooldownEnded OnChatCooldownEnded;

	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnQuickChatUnlocked OnQuickChatUnlocked;

protected:
	void OnQuickChatTick();
	void UpdatePings(float DeltaTime);
	void UpdateCooldowns(float DeltaTime);
	void InitializeDefaultMessages();
	void InitializeDefaultWheel();
	void PlayVoiceLine(const FMGQuickChatMessage& Message);
	bool ShouldReceiveMessage(const FMGChatEvent& ChatEvent) const;
	FLinearColor GetPingColor(EMGPingType PingType) const;

	UPROPERTY()
	TMap<FName, FMGQuickChatMessage> MessageLibrary;

	UPROPERTY()
	TMap<FName, FMGQuickChatWheel> Wheels;

	UPROPERTY()
	FMGQuickChatWheel ActiveWheel;

	UPROPERTY()
	TArray<FMGWorldPing> ActivePings;

	UPROPERTY()
	TArray<FMGChatEvent> ChatHistory;

	UPROPERTY()
	FMGQuickChatConfig Config;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;

	UPROPERTY()
	int32 LocalTeamID = -1;

	UPROPERTY()
	FVector LocalPlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	float MessageCooldownRemaining = 0.0f;

	UPROPERTY()
	float PingCooldownRemaining = 0.0f;

	UPROPERTY()
	int32 MaxChatHistory = 100;

	FTimerHandle QuickChatTickHandle;
};
