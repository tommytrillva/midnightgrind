// Copyright Midnight Grind. All Rights Reserved.

#include "QuickChat/MGQuickChatSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGQuickChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Config.MessageCooldown = 1.0f;
	Config.PingCooldown = 2.0f;
	Config.MaxPingsPerPlayer = 3;
	Config.PingDefaultDuration = 5.0f;
	Config.NearbyRange = 5000.0f;
	Config.bPlayVoiceLines = true;
	Config.VoiceLineVolume = 1.0f;
	Config.bShowChatBubbles = true;
	Config.ChatBubbleDuration = 3.0f;

	InitializeDefaultMessages();
	InitializeDefaultWheel();
	LoadWheelConfiguration();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			QuickChatTickHandle,
			this,
			&UMGQuickChatSubsystem::OnQuickChatTick,
			0.1f,
			true
		);
	}
}

void UMGQuickChatSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QuickChatTickHandle);
	}

	SaveWheelConfiguration();
	Super::Deinitialize();
}

bool UMGQuickChatSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

bool UMGQuickChatSubsystem::SendQuickChat(FName MessageID, EMGQuickChatVisibility Visibility)
{
	if (!CanSendMessage())
	{
		return false;
	}

	FMGQuickChatMessage* Message = MessageLibrary.Find(MessageID);
	if (!Message || !Message->bIsUnlocked)
	{
		return false;
	}

	FMGChatEvent ChatEvent;
	ChatEvent.SenderID = LocalPlayerID;
	ChatEvent.SenderName = LocalPlayerName;
	ChatEvent.Message = *Message;
	ChatEvent.Timestamp = FDateTime::UtcNow();
	ChatEvent.Visibility = Visibility;
	ChatEvent.SenderLocation = LocalPlayerLocation;
	ChatEvent.TeamID = LocalTeamID;

	// Add to local history
	ChatHistory.Add(ChatEvent);
	if (ChatHistory.Num() > MaxChatHistory)
	{
		ChatHistory.RemoveAt(0);
	}

	// Play voice line locally
	PlayVoiceLine(*Message);

	// Broadcast to network (handled by game networking)
	OnQuickChatReceived.Broadcast(ChatEvent);

	// Start cooldown
	MessageCooldownRemaining = Config.MessageCooldown;
	OnChatCooldownStarted.Broadcast(Config.MessageCooldown);

	return true;
}

bool UMGQuickChatSubsystem::SendQuickChatFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ActiveWheel.Messages.Num())
	{
		return false;
	}

	return SendQuickChat(ActiveWheel.Messages[SlotIndex].MessageID);
}

bool UMGQuickChatSubsystem::SendCustomMessage(const FText& Text, EMGQuickChatVisibility Visibility)
{
	if (!CanSendMessage())
	{
		return false;
	}

	FMGQuickChatMessage CustomMessage;
	CustomMessage.MessageID = FName(*FGuid::NewGuid().ToString());
	CustomMessage.Category = EMGQuickChatCategory::Custom;
	CustomMessage.DisplayText = Text;
	CustomMessage.LocalizedText = Text;

	FMGChatEvent ChatEvent;
	ChatEvent.SenderID = LocalPlayerID;
	ChatEvent.SenderName = LocalPlayerName;
	ChatEvent.Message = CustomMessage;
	ChatEvent.Timestamp = FDateTime::UtcNow();
	ChatEvent.Visibility = Visibility;
	ChatEvent.SenderLocation = LocalPlayerLocation;
	ChatEvent.TeamID = LocalTeamID;

	ChatHistory.Add(ChatEvent);
	if (ChatHistory.Num() > MaxChatHistory)
	{
		ChatHistory.RemoveAt(0);
	}

	OnQuickChatReceived.Broadcast(ChatEvent);

	MessageCooldownRemaining = Config.MessageCooldown;
	OnChatCooldownStarted.Broadcast(Config.MessageCooldown);

	return true;
}

bool UMGQuickChatSubsystem::CanSendMessage() const
{
	return MessageCooldownRemaining <= 0.0f;
}

float UMGQuickChatSubsystem::GetMessageCooldownRemaining() const
{
	return FMath::Max(0.0f, MessageCooldownRemaining);
}

FGuid UMGQuickChatSubsystem::CreatePing(FVector Location, EMGPingType PingType)
{
	return CreateDirectionalPing(Location, FVector::ForwardVector, PingType);
}

FGuid UMGQuickChatSubsystem::CreateDirectionalPing(FVector Location, FVector Direction, EMGPingType PingType)
{
	if (!CanCreatePing())
	{
		return FGuid();
	}

	// Check if we need to remove oldest ping
	TArray<FMGWorldPing> MyPings = GetMyPings();
	if (MyPings.Num() >= Config.MaxPingsPerPlayer)
	{
		RemovePing(MyPings[0].PingID);
	}

	FMGWorldPing Ping;
	Ping.PingID = FGuid::NewGuid();
	Ping.OwnerID = LocalPlayerID;
	Ping.OwnerName = LocalPlayerName;
	Ping.PingType = PingType;
	Ping.WorldLocation = Location;
	Ping.WorldDirection = Direction.GetSafeNormal();
	Ping.CreatedAt = FDateTime::UtcNow();
	Ping.Duration = Config.PingDefaultDuration;
	Ping.TimeRemaining = Config.PingDefaultDuration;
	Ping.PingColor = GetPingColor(PingType);
	Ping.Visibility = EMGQuickChatVisibility::TeamOnly;
	Ping.TeamID = LocalTeamID;
	Ping.bIsActive = true;

	// Set ping label based on type
	switch (PingType)
	{
	case EMGPingType::Location:
		Ping.PingLabel = FText::FromString(TEXT("Here"));
		break;
	case EMGPingType::Warning:
		Ping.PingLabel = FText::FromString(TEXT("Warning!"));
		break;
	case EMGPingType::Shortcut:
		Ping.PingLabel = FText::FromString(TEXT("Shortcut"));
		break;
	case EMGPingType::Police:
		Ping.PingLabel = FText::FromString(TEXT("Police!"));
		break;
	case EMGPingType::Obstacle:
		Ping.PingLabel = FText::FromString(TEXT("Obstacle"));
		break;
	case EMGPingType::Opponent:
		Ping.PingLabel = FText::FromString(TEXT("Enemy"));
		break;
	case EMGPingType::Help:
		Ping.PingLabel = FText::FromString(TEXT("Help!"));
		break;
	default:
		Ping.PingLabel = FText::GetEmpty();
		break;
	}

	ActivePings.Add(Ping);
	OnPingCreated.Broadcast(Ping);

	PingCooldownRemaining = Config.PingCooldown;

	return Ping.PingID;
}

void UMGQuickChatSubsystem::RemovePing(FGuid PingID)
{
	for (int32 i = ActivePings.Num() - 1; i >= 0; --i)
	{
		if (ActivePings[i].PingID == PingID)
		{
			FMGWorldPing ExpiredPing = ActivePings[i];
			ActivePings.RemoveAt(i);
			OnPingExpired.Broadcast(ExpiredPing);
			break;
		}
	}
}

void UMGQuickChatSubsystem::RemoveAllMyPings()
{
	for (int32 i = ActivePings.Num() - 1; i >= 0; --i)
	{
		if (ActivePings[i].OwnerID == LocalPlayerID)
		{
			FMGWorldPing ExpiredPing = ActivePings[i];
			ActivePings.RemoveAt(i);
			OnPingExpired.Broadcast(ExpiredPing);
		}
	}
}

bool UMGQuickChatSubsystem::CanCreatePing() const
{
	return PingCooldownRemaining <= 0.0f;
}

TArray<FMGWorldPing> UMGQuickChatSubsystem::GetActivePings() const
{
	TArray<FMGWorldPing> Result;
	for (const FMGWorldPing& Ping : ActivePings)
	{
		if (Ping.bIsActive)
		{
			Result.Add(Ping);
		}
	}
	return Result;
}

TArray<FMGWorldPing> UMGQuickChatSubsystem::GetMyPings() const
{
	TArray<FMGWorldPing> Result;
	for (const FMGWorldPing& Ping : ActivePings)
	{
		if (Ping.OwnerID == LocalPlayerID && Ping.bIsActive)
		{
			Result.Add(Ping);
		}
	}
	return Result;
}

float UMGQuickChatSubsystem::GetPingCooldownRemaining() const
{
	return FMath::Max(0.0f, PingCooldownRemaining);
}

TArray<FMGQuickChatMessage> UMGQuickChatSubsystem::GetMessagesByCategory(EMGQuickChatCategory Category) const
{
	TArray<FMGQuickChatMessage> Result;
	for (const auto& Pair : MessageLibrary)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGQuickChatMessage> UMGQuickChatSubsystem::GetAllMessages() const
{
	TArray<FMGQuickChatMessage> Result;
	MessageLibrary.GenerateValueArray(Result);
	return Result;
}

TArray<FMGQuickChatMessage> UMGQuickChatSubsystem::GetUnlockedMessages() const
{
	TArray<FMGQuickChatMessage> Result;
	for (const auto& Pair : MessageLibrary)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGQuickChatMessage UMGQuickChatSubsystem::GetMessage(FName MessageID) const
{
	if (const FMGQuickChatMessage* Message = MessageLibrary.Find(MessageID))
	{
		return *Message;
	}
	return FMGQuickChatMessage();
}

bool UMGQuickChatSubsystem::UnlockMessage(FName MessageID)
{
	if (FMGQuickChatMessage* Message = MessageLibrary.Find(MessageID))
	{
		if (!Message->bIsUnlocked)
		{
			Message->bIsUnlocked = true;
			OnQuickChatUnlocked.Broadcast(MessageID, Message->Category);
			return true;
		}
	}
	return false;
}

void UMGQuickChatSubsystem::SetActiveWheel(FName WheelID)
{
	if (FMGQuickChatWheel* Wheel = Wheels.Find(WheelID))
	{
		ActiveWheel = *Wheel;
	}
}

void UMGQuickChatSubsystem::AssignMessageToSlot(FName MessageID, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ActiveWheel.MaxSlots)
	{
		return;
	}

	FMGQuickChatMessage* Message = MessageLibrary.Find(MessageID);
	if (!Message)
	{
		return;
	}

	while (ActiveWheel.Messages.Num() <= SlotIndex)
	{
		ActiveWheel.Messages.Add(FMGQuickChatMessage());
	}

	Message->SlotIndex = SlotIndex;
	ActiveWheel.Messages[SlotIndex] = *Message;
}

void UMGQuickChatSubsystem::ClearSlot(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < ActiveWheel.Messages.Num())
	{
		ActiveWheel.Messages[SlotIndex] = FMGQuickChatMessage();
	}
}

FMGQuickChatMessage UMGQuickChatSubsystem::GetMessageAtSlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < ActiveWheel.Messages.Num())
	{
		return ActiveWheel.Messages[SlotIndex];
	}
	return FMGQuickChatMessage();
}

void UMGQuickChatSubsystem::SaveWheelConfiguration()
{
	// Save wheel configuration to player save
}

void UMGQuickChatSubsystem::LoadWheelConfiguration()
{
	// Load wheel configuration from player save
}

TArray<FMGChatEvent> UMGQuickChatSubsystem::GetChatHistory(int32 MaxEntries) const
{
	if (MaxEntries >= ChatHistory.Num())
	{
		return ChatHistory;
	}

	TArray<FMGChatEvent> Result;
	int32 StartIndex = ChatHistory.Num() - MaxEntries;
	for (int32 i = StartIndex; i < ChatHistory.Num(); ++i)
	{
		Result.Add(ChatHistory[i]);
	}
	return Result;
}

void UMGQuickChatSubsystem::ClearChatHistory()
{
	ChatHistory.Empty();
}

void UMGQuickChatSubsystem::SetConfig(const FMGQuickChatConfig& NewConfig)
{
	Config = NewConfig;
}

void UMGQuickChatSubsystem::MutePlayer(FName PlayerID)
{
	Config.MutedPlayers.AddUnique(PlayerID);
}

void UMGQuickChatSubsystem::UnmutePlayer(FName PlayerID)
{
	Config.MutedPlayers.Remove(PlayerID);
}

bool UMGQuickChatSubsystem::IsPlayerMuted(FName PlayerID) const
{
	return Config.MutedPlayers.Contains(PlayerID);
}

void UMGQuickChatSubsystem::SetVoiceLinesEnabled(bool bEnabled)
{
	Config.bPlayVoiceLines = bEnabled;
}

void UMGQuickChatSubsystem::SetLocalPlayerInfo(FName PlayerID, const FString& PlayerName, int32 TeamID)
{
	LocalPlayerID = PlayerID;
	LocalPlayerName = PlayerName;
	LocalTeamID = TeamID;
}

void UMGQuickChatSubsystem::SetLocalPlayerLocation(FVector Location)
{
	LocalPlayerLocation = Location;
}

void UMGQuickChatSubsystem::ReceiveQuickChat(const FMGChatEvent& ChatEvent)
{
	if (!ShouldReceiveMessage(ChatEvent))
	{
		return;
	}

	ChatHistory.Add(ChatEvent);
	if (ChatHistory.Num() > MaxChatHistory)
	{
		ChatHistory.RemoveAt(0);
	}

	if (Config.bPlayVoiceLines)
	{
		PlayVoiceLine(ChatEvent.Message);
	}

	OnQuickChatReceived.Broadcast(ChatEvent);
}

void UMGQuickChatSubsystem::ReceivePing(const FMGWorldPing& Ping)
{
	ActivePings.Add(Ping);
	OnPingCreated.Broadcast(Ping);
}

void UMGQuickChatSubsystem::OnQuickChatTick()
{
	float DeltaTime = 0.1f;
	UpdatePings(DeltaTime);
	UpdateCooldowns(DeltaTime);
}

void UMGQuickChatSubsystem::UpdatePings(float DeltaTime)
{
	for (int32 i = ActivePings.Num() - 1; i >= 0; --i)
	{
		ActivePings[i].TimeRemaining -= DeltaTime;

		if (ActivePings[i].TimeRemaining <= 0.0f)
		{
			FMGWorldPing ExpiredPing = ActivePings[i];
			ActivePings.RemoveAt(i);
			OnPingExpired.Broadcast(ExpiredPing);
		}
	}
}

void UMGQuickChatSubsystem::UpdateCooldowns(float DeltaTime)
{
	if (MessageCooldownRemaining > 0.0f)
	{
		MessageCooldownRemaining -= DeltaTime;
		if (MessageCooldownRemaining <= 0.0f)
		{
			MessageCooldownRemaining = 0.0f;
			OnChatCooldownEnded.Broadcast();
		}
	}

	if (PingCooldownRemaining > 0.0f)
	{
		PingCooldownRemaining -= DeltaTime;
		if (PingCooldownRemaining < 0.0f)
		{
			PingCooldownRemaining = 0.0f;
		}
	}
}

void UMGQuickChatSubsystem::InitializeDefaultMessages()
{
	// Greetings
	auto AddMessage = [this](FName ID, EMGQuickChatCategory Cat, const FString& Text)
	{
		FMGQuickChatMessage Msg;
		Msg.MessageID = ID;
		Msg.Category = Cat;
		Msg.DisplayText = FText::FromString(Text);
		Msg.LocalizedText = FText::FromString(Text);
		Msg.bIsUnlocked = true;
		MessageLibrary.Add(ID, Msg);
	};

	AddMessage(FName(TEXT("Hello")), EMGQuickChatCategory::Greetings, TEXT("Hello!"));
	AddMessage(FName(TEXT("GoodLuck")), EMGQuickChatCategory::Greetings, TEXT("Good luck!"));
	AddMessage(FName(TEXT("HaveFun")), EMGQuickChatCategory::Greetings, TEXT("Have fun!"));
	AddMessage(FName(TEXT("LetsRace")), EMGQuickChatCategory::Greetings, TEXT("Let's race!"));

	AddMessage(FName(TEXT("NiceRace")), EMGQuickChatCategory::Racing, TEXT("Nice race!"));
	AddMessage(FName(TEXT("GoodMove")), EMGQuickChatCategory::Racing, TEXT("Good move!"));
	AddMessage(FName(TEXT("WatchOut")), EMGQuickChatCategory::Racing, TEXT("Watch out!"));
	AddMessage(FName(TEXT("OnYourLeft")), EMGQuickChatCategory::Racing, TEXT("On your left!"));
	AddMessage(FName(TEXT("OnYourRight")), EMGQuickChatCategory::Racing, TEXT("On your right!"));

	AddMessage(FName(TEXT("FollowMe")), EMGQuickChatCategory::TeamTactics, TEXT("Follow me!"));
	AddMessage(FName(TEXT("GoAhead")), EMGQuickChatCategory::TeamTactics, TEXT("Go ahead!"));
	AddMessage(FName(TEXT("CoverMe")), EMGQuickChatCategory::TeamTactics, TEXT("Cover me!"));
	AddMessage(FName(TEXT("StickTogether")), EMGQuickChatCategory::TeamTactics, TEXT("Stick together!"));

	AddMessage(FName(TEXT("Wow")), EMGQuickChatCategory::Reactions, TEXT("Wow!"));
	AddMessage(FName(TEXT("Nice")), EMGQuickChatCategory::Reactions, TEXT("Nice!"));
	AddMessage(FName(TEXT("Oops")), EMGQuickChatCategory::Reactions, TEXT("Oops!"));
	AddMessage(FName(TEXT("MyBad")), EMGQuickChatCategory::Reactions, TEXT("My bad!"));
	AddMessage(FName(TEXT("NoWay")), EMGQuickChatCategory::Reactions, TEXT("No way!"));

	AddMessage(FName(TEXT("NiceDrift")), EMGQuickChatCategory::Compliments, TEXT("Nice drift!"));
	AddMessage(FName(TEXT("GreatSave")), EMGQuickChatCategory::Compliments, TEXT("Great save!"));
	AddMessage(FName(TEXT("WellPlayed")), EMGQuickChatCategory::Compliments, TEXT("Well played!"));
	AddMessage(FName(TEXT("Impressive")), EMGQuickChatCategory::Compliments, TEXT("Impressive!"));

	AddMessage(FName(TEXT("EatMyDust")), EMGQuickChatCategory::Taunts, TEXT("Eat my dust!"));
	AddMessage(FName(TEXT("TooSlow")), EMGQuickChatCategory::Taunts, TEXT("Too slow!"));
	AddMessage(FName(TEXT("LaterLoser")), EMGQuickChatCategory::Taunts, TEXT("Later, loser!"));
	AddMessage(FName(TEXT("CantCatchMe")), EMGQuickChatCategory::Taunts, TEXT("Can't catch me!"));

	AddMessage(FName(TEXT("PoliceAhead")), EMGQuickChatCategory::Callouts, TEXT("Police ahead!"));
	AddMessage(FName(TEXT("ShortcutHere")), EMGQuickChatCategory::Callouts, TEXT("Shortcut here!"));
	AddMessage(FName(TEXT("HazardAhead")), EMGQuickChatCategory::Callouts, TEXT("Hazard ahead!"));
	AddMessage(FName(TEXT("TrafficHeavy")), EMGQuickChatCategory::Callouts, TEXT("Heavy traffic!"));
}

void UMGQuickChatSubsystem::InitializeDefaultWheel()
{
	FMGQuickChatWheel DefaultWheel;
	DefaultWheel.WheelID = FName(TEXT("Default"));
	DefaultWheel.WheelName = FText::FromString(TEXT("Quick Chat"));
	DefaultWheel.MaxSlots = 8;

	TArray<FName> DefaultSlots = {
		FName(TEXT("Hello")),
		FName(TEXT("GoodLuck")),
		FName(TEXT("Nice")),
		FName(TEXT("WatchOut")),
		FName(TEXT("NiceDrift")),
		FName(TEXT("MyBad")),
		FName(TEXT("FollowMe")),
		FName(TEXT("PoliceAhead"))
	};

	for (int32 i = 0; i < DefaultSlots.Num(); ++i)
	{
		if (FMGQuickChatMessage* Msg = MessageLibrary.Find(DefaultSlots[i]))
		{
			Msg->SlotIndex = i;
			DefaultWheel.Messages.Add(*Msg);
		}
	}

	Wheels.Add(DefaultWheel.WheelID, DefaultWheel);
	ActiveWheel = DefaultWheel;
}

void UMGQuickChatSubsystem::PlayVoiceLine(const FMGQuickChatMessage& Message)
{
	if (!Config.bPlayVoiceLines)
	{
		return;
	}

	if (!Message.VoiceLine.IsNull())
	{
		if (USoundBase* Sound = Message.VoiceLine.LoadSynchronous())
		{
			UGameplayStatics::PlaySound2D(GetWorld(), Sound, Config.VoiceLineVolume);
		}
	}
}

bool UMGQuickChatSubsystem::ShouldReceiveMessage(const FMGChatEvent& ChatEvent) const
{
	// Don't receive our own messages (we already handled them)
	if (ChatEvent.SenderID == LocalPlayerID)
	{
		return false;
	}

	// Check muted players
	if (IsPlayerMuted(ChatEvent.SenderID))
	{
		return false;
	}

	// Check if opponents are muted
	if (Config.bMuteOpponents && ChatEvent.TeamID != LocalTeamID && ChatEvent.TeamID >= 0)
	{
		return false;
	}

	// Check visibility
	switch (ChatEvent.Visibility)
	{
	case EMGQuickChatVisibility::All:
		return true;

	case EMGQuickChatVisibility::TeamOnly:
		return ChatEvent.TeamID == LocalTeamID;

	case EMGQuickChatVisibility::NearbyOnly:
		return FVector::Dist(ChatEvent.SenderLocation, LocalPlayerLocation) <= Config.NearbyRange;

	case EMGQuickChatVisibility::Private:
		return false; // Private messages handled separately

	default:
		return true;
	}
}

FLinearColor UMGQuickChatSubsystem::GetPingColor(EMGPingType PingType) const
{
	switch (PingType)
	{
	case EMGPingType::Location:
		return FLinearColor::Blue;
	case EMGPingType::Warning:
		return FLinearColor::Yellow;
	case EMGPingType::Shortcut:
		return FLinearColor::Green;
	case EMGPingType::Police:
		return FLinearColor::Red;
	case EMGPingType::Obstacle:
		return FLinearColor(1.0f, 0.5f, 0.0f);
	case EMGPingType::Opponent:
		return FLinearColor::Red;
	case EMGPingType::Help:
		return FLinearColor(1.0f, 0.0f, 1.0f);
	default:
		return FLinearColor::White;
	}
}
