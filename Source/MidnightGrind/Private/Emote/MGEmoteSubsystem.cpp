// Copyright Midnight Grind. All Rights Reserved.

#include "Emote/MGEmoteSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGEmoteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadEmoteData();
	InitializeDefaultLoadout();

	// Start emote tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EmoteTickHandle,
			this,
			&UMGEmoteSubsystem::OnEmoteTick,
			0.05f, // 20Hz tick for smooth emote updates
			true
		);
	}
}

void UMGEmoteSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EmoteTickHandle);
	}

	SaveEmoteData();
	Super::Deinitialize();
}

bool UMGEmoteSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Emote Playback
// ============================================================================

bool UMGEmoteSubsystem::PlayEmote(FName EmoteID, EMGEmoteContext Context)
{
	// Check if emote exists and is unlocked
	const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID);
	if (!EmoteDef)
	{
		return false;
	}

	if (!IsEmoteUnlocked(EmoteID))
	{
		return false;
	}

	// Check cooldown
	if (IsEmoteOnCooldown(EmoteID))
	{
		return false;
	}

	// Check context validity
	if (!CanPlayEmoteInContext(*EmoteDef, Context))
	{
		return false;
	}

	// Stop current emote if playing
	if (ActiveEmote.bIsPlaying)
	{
		StopCurrentEmote();
	}

	// Start new emote
	ActiveEmote = FMGActiveEmote();
	ActiveEmote.EmoteID = EmoteID;
	ActiveEmote.PlayerID = LocalPlayerID;
	ActiveEmote.Context = Context;
	ActiveEmote.Duration = EmoteDef->Duration;
	ActiveEmote.bIsPlaying = true;

	if (const UWorld* World = GetWorld())
	{
		ActiveEmote.StartTime = World->GetTimeSeconds();
	}

	// Set cooldown
	EmoteCooldowns.Add(EmoteID, EmoteDef->Cooldown);

	// Track usage
	TrackEmoteUsage(EmoteID, Context);

	OnEmoteStarted.Broadcast(LocalPlayerID, *EmoteDef);

	return true;
}

bool UMGEmoteSubsystem::PlayEmoteAtIndex(int32 WheelSlotIndex)
{
	TArray<FMGEmoteWheelSlot> Slots = GetWheelSlots();

	for (const FMGEmoteWheelSlot& Slot : Slots)
	{
		if (Slot.SlotIndex == WheelSlotIndex && !Slot.EmoteID.IsNone())
		{
			return PlayEmote(Slot.EmoteID, CurrentContext);
		}
	}

	return false;
}

bool UMGEmoteSubsystem::PlayRandomEmote(EMGEmoteType TypeFilter, EMGEmoteContext ContextFilter)
{
	TArray<FMGEmoteDefinition> AvailableEmotes;

	for (const FName& EmoteID : Collection.UnlockedEmotes)
	{
		if (const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID))
		{
			// Check type filter
			if (TypeFilter != EMGEmoteType::None && EmoteDef->EmoteType != TypeFilter)
			{
				continue;
			}

			// Check context filter
			if (ContextFilter != EMGEmoteContext::Any && !CanPlayEmoteInContext(*EmoteDef, ContextFilter))
			{
				continue;
			}

			// Check cooldown
			if (!IsEmoteOnCooldown(EmoteID))
			{
				AvailableEmotes.Add(*EmoteDef);
			}
		}
	}

	if (AvailableEmotes.Num() == 0)
	{
		return false;
	}

	int32 RandomIndex = FMath::RandRange(0, AvailableEmotes.Num() - 1);
	return PlayEmote(AvailableEmotes[RandomIndex].EmoteID, ContextFilter);
}

void UMGEmoteSubsystem::StopCurrentEmote()
{
	if (!ActiveEmote.bIsPlaying)
	{
		return;
	}

	FName EmoteID = ActiveEmote.EmoteID;
	ActiveEmote = FMGActiveEmote();

	OnEmoteEnded.Broadcast(LocalPlayerID, EmoteID);
}

void UMGEmoteSubsystem::InterruptEmote(FName PlayerID)
{
	if (PlayerID == LocalPlayerID)
	{
		if (ActiveEmote.bIsPlaying)
		{
			const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(ActiveEmote.EmoteID);
			if (EmoteDef && EmoteDef->bInterruptible)
			{
				FName EmoteID = ActiveEmote.EmoteID;
				ActiveEmote = FMGActiveEmote();
				OnEmoteInterrupted.Broadcast(PlayerID, EmoteID);
			}
		}
	}
	else
	{
		// Interrupt remote player's emote
		for (int32 i = ActiveRemoteEmotes.Num() - 1; i >= 0; --i)
		{
			if (ActiveRemoteEmotes[i].PlayerID == PlayerID)
			{
				FName EmoteID = ActiveRemoteEmotes[i].EmoteID;
				ActiveRemoteEmotes.RemoveAt(i);
				OnEmoteInterrupted.Broadcast(PlayerID, EmoteID);
				break;
			}
		}
	}
}

bool UMGEmoteSubsystem::IsPlayingEmote() const
{
	return ActiveEmote.bIsPlaying;
}

bool UMGEmoteSubsystem::IsEmoteOnCooldown(FName EmoteID) const
{
	if (const float* Cooldown = EmoteCooldowns.Find(EmoteID))
	{
		return *Cooldown > 0.0f;
	}
	return false;
}

float UMGEmoteSubsystem::GetEmoteCooldownRemaining(FName EmoteID) const
{
	if (const float* Cooldown = EmoteCooldowns.Find(EmoteID))
	{
		return FMath::Max(0.0f, *Cooldown);
	}
	return 0.0f;
}

// ============================================================================
// Remote Emotes
// ============================================================================

void UMGEmoteSubsystem::PlayRemoteEmote(FName PlayerID, FName EmoteID, const FVector& Position, const FRotator& Rotation)
{
	const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID);
	if (!EmoteDef)
	{
		return;
	}

	// Check if this player already has an active emote
	for (int32 i = 0; i < ActiveRemoteEmotes.Num(); ++i)
	{
		if (ActiveRemoteEmotes[i].PlayerID == PlayerID)
		{
			ActiveRemoteEmotes.RemoveAt(i);
			break;
		}
	}

	FMGActiveEmote RemoteEmote;
	RemoteEmote.EmoteID = EmoteID;
	RemoteEmote.PlayerID = PlayerID;
	RemoteEmote.Duration = EmoteDef->Duration;
	RemoteEmote.bIsPlaying = true;
	RemoteEmote.WorldPosition = Position;
	RemoteEmote.WorldRotation = Rotation;

	if (const UWorld* World = GetWorld())
	{
		RemoteEmote.StartTime = World->GetTimeSeconds();
	}

	ActiveRemoteEmotes.Add(RemoteEmote);

	OnEmoteStarted.Broadcast(PlayerID, *EmoteDef);
}

// ============================================================================
// Contextual Emotes
// ============================================================================

void UMGEmoteSubsystem::PlayVictoryEmote()
{
	if (!ActiveLoadout.VictoryEmote.IsNone())
	{
		PlayEmote(ActiveLoadout.VictoryEmote, EMGEmoteContext::Victory);
	}
	else
	{
		PlayRandomEmote(EMGEmoteType::Celebration, EMGEmoteContext::Victory);
	}
}

void UMGEmoteSubsystem::PlayDefeatEmote()
{
	if (!ActiveLoadout.DefeatEmote.IsNone())
	{
		PlayEmote(ActiveLoadout.DefeatEmote, EMGEmoteContext::Defeat);
	}
}

void UMGEmoteSubsystem::PlayPodiumEmote(int32 Position)
{
	if (!ActiveLoadout.PodiumEmote.IsNone())
	{
		PlayEmote(ActiveLoadout.PodiumEmote, EMGEmoteContext::Podium);
	}
	else if (Position == 1)
	{
		PlayVictoryEmote();
	}
}

void UMGEmoteSubsystem::PlayGreetingEmote()
{
	if (!ActiveLoadout.GreetingEmote.IsNone())
	{
		PlayEmote(ActiveLoadout.GreetingEmote, EMGEmoteContext::Lobby);
	}
	else
	{
		PlayRandomEmote(EMGEmoteType::Greeting, EMGEmoteContext::Lobby);
	}
}

void UMGEmoteSubsystem::SetCurrentContext(EMGEmoteContext Context)
{
	CurrentContext = Context;
}

// ============================================================================
// Emote Wheel
// ============================================================================

void UMGEmoteSubsystem::OpenEmoteWheel()
{
	if (!bWheelOpen)
	{
		bWheelOpen = true;
		OnEmoteWheelOpened.Broadcast();
	}
}

void UMGEmoteSubsystem::CloseEmoteWheel()
{
	bWheelOpen = false;
}

void UMGEmoteSubsystem::SelectWheelSlot(int32 SlotIndex)
{
	if (bWheelOpen)
	{
		OnEmoteWheelSelection.Broadcast(SlotIndex);
		PlayEmoteAtIndex(SlotIndex);
		CloseEmoteWheel();
	}
}

TArray<FMGEmoteWheelSlot> UMGEmoteSubsystem::GetWheelSlots() const
{
	return ActiveLoadout.WheelSlots;
}

void UMGEmoteSubsystem::SetWheelSlot(int32 SlotIndex, FName EmoteID)
{
	for (FMGEmoteWheelSlot& Slot : ActiveLoadout.WheelSlots)
	{
		if (Slot.SlotIndex == SlotIndex)
		{
			Slot.EmoteID = EmoteID;
			OnEmoteEquipped.Broadcast(SlotIndex, EmoteID);
			SaveEmoteData();
			return;
		}
	}

	// Add new slot if it doesn't exist
	FMGEmoteWheelSlot NewSlot;
	NewSlot.SlotIndex = SlotIndex;
	NewSlot.EmoteID = EmoteID;
	ActiveLoadout.WheelSlots.Add(NewSlot);
	OnEmoteEquipped.Broadcast(SlotIndex, EmoteID);
	SaveEmoteData();
}

// ============================================================================
// Quick Select
// ============================================================================

void UMGEmoteSubsystem::PlayQuickSelectEmote(int32 Index)
{
	if (Index >= 0 && Index < ActiveLoadout.QuickSelectEmotes.Num())
	{
		FName EmoteID = ActiveLoadout.QuickSelectEmotes[Index];
		if (!EmoteID.IsNone())
		{
			PlayEmote(EmoteID, CurrentContext);
		}
	}
}

void UMGEmoteSubsystem::SetQuickSelectEmote(int32 Index, FName EmoteID)
{
	// Expand array if needed
	while (ActiveLoadout.QuickSelectEmotes.Num() <= Index)
	{
		ActiveLoadout.QuickSelectEmotes.Add(NAME_None);
	}

	ActiveLoadout.QuickSelectEmotes[Index] = EmoteID;
	SaveEmoteData();
}

TArray<FName> UMGEmoteSubsystem::GetQuickSelectEmotes() const
{
	return ActiveLoadout.QuickSelectEmotes;
}

// ============================================================================
// Loadouts
// ============================================================================

void UMGEmoteSubsystem::SetActiveLoadout(const FMGEmoteLoadout& Loadout)
{
	ActiveLoadout = Loadout;
	SaveEmoteData();
}

void UMGEmoteSubsystem::SaveLoadout(const FMGEmoteLoadout& Loadout, int32 SlotIndex)
{
	// Expand array if needed
	while (SavedLoadouts.Num() <= SlotIndex)
	{
		SavedLoadouts.Add(FMGEmoteLoadout());
	}

	SavedLoadouts[SlotIndex] = Loadout;
	SaveEmoteData();
}

FMGEmoteLoadout UMGEmoteSubsystem::GetLoadout(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < SavedLoadouts.Num())
	{
		return SavedLoadouts[SlotIndex];
	}

	return FMGEmoteLoadout();
}

// ============================================================================
// Collection
// ============================================================================

bool UMGEmoteSubsystem::UnlockEmote(FName EmoteID)
{
	if (Collection.UnlockedEmotes.Contains(EmoteID))
	{
		return false; // Already unlocked
	}

	const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID);
	if (!EmoteDef)
	{
		return false;
	}

	Collection.UnlockedEmotes.Add(EmoteID);
	Collection.TotalEmotesUnlocked++;

	OnEmoteUnlocked.Broadcast(*EmoteDef);
	SaveEmoteData();

	return true;
}

bool UMGEmoteSubsystem::IsEmoteUnlocked(FName EmoteID) const
{
	return Collection.UnlockedEmotes.Contains(EmoteID);
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetUnlockedEmotes() const
{
	TArray<FMGEmoteDefinition> UnlockedEmotes;

	for (const FName& EmoteID : Collection.UnlockedEmotes)
	{
		if (const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID))
		{
			UnlockedEmotes.Add(*EmoteDef);
		}
	}

	return UnlockedEmotes;
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetEmotesByType(EMGEmoteType Type) const
{
	TArray<FMGEmoteDefinition> FilteredEmotes;

	for (const auto& EmotePair : EmoteDatabase)
	{
		if (EmotePair.Value.EmoteType == Type)
		{
			FilteredEmotes.Add(EmotePair.Value);
		}
	}

	return FilteredEmotes;
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetEmotesByRarity(EMGEmoteRarity Rarity) const
{
	TArray<FMGEmoteDefinition> FilteredEmotes;

	for (const auto& EmotePair : EmoteDatabase)
	{
		if (EmotePair.Value.Rarity == Rarity)
		{
			FilteredEmotes.Add(EmotePair.Value);
		}
	}

	return FilteredEmotes;
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetEmotesForContext(EMGEmoteContext Context) const
{
	TArray<FMGEmoteDefinition> ContextEmotes;

	for (const auto& EmotePair : EmoteDatabase)
	{
		if (CanPlayEmoteInContext(EmotePair.Value, Context))
		{
			ContextEmotes.Add(EmotePair.Value);
		}
	}

	return ContextEmotes;
}

void UMGEmoteSubsystem::SetEmoteFavorite(FName EmoteID, bool bFavorite)
{
	if (bFavorite)
	{
		if (!Collection.FavoriteEmotes.Contains(EmoteID))
		{
			Collection.FavoriteEmotes.Add(EmoteID);
		}
	}
	else
	{
		Collection.FavoriteEmotes.Remove(EmoteID);
	}

	FMGEmoteUsageStats& Stats = Collection.UsageStats.FindOrAdd(EmoteID);
	Stats.EmoteID = EmoteID;
	Stats.bFavorite = bFavorite;

	SaveEmoteData();
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetFavoriteEmotes() const
{
	TArray<FMGEmoteDefinition> FavoriteEmotes;

	for (const FName& EmoteID : Collection.FavoriteEmotes)
	{
		if (const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID))
		{
			FavoriteEmotes.Add(*EmoteDef);
		}
	}

	return FavoriteEmotes;
}

// ============================================================================
// Emote Database
// ============================================================================

void UMGEmoteSubsystem::RegisterEmote(const FMGEmoteDefinition& Emote)
{
	EmoteDatabase.Add(Emote.EmoteID, Emote);

	// Auto-unlock free emotes
	if (Emote.bUnlocked && !Collection.UnlockedEmotes.Contains(Emote.EmoteID))
	{
		Collection.UnlockedEmotes.Add(Emote.EmoteID);
		Collection.TotalEmotesUnlocked++;
	}
}

FMGEmoteDefinition UMGEmoteSubsystem::GetEmoteDefinition(FName EmoteID) const
{
	if (const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID))
	{
		return *EmoteDef;
	}

	return FMGEmoteDefinition();
}

TArray<FMGEmoteDefinition> UMGEmoteSubsystem::GetAllEmotes() const
{
	TArray<FMGEmoteDefinition> AllEmotes;

	for (const auto& EmotePair : EmoteDatabase)
	{
		AllEmotes.Add(EmotePair.Value);
	}

	return AllEmotes;
}

// ============================================================================
// Stats
// ============================================================================

FMGEmoteUsageStats UMGEmoteSubsystem::GetEmoteStats(FName EmoteID) const
{
	if (const FMGEmoteUsageStats* Stats = Collection.UsageStats.Find(EmoteID))
	{
		return *Stats;
	}

	return FMGEmoteUsageStats();
}

FName UMGEmoteSubsystem::GetMostUsedEmote() const
{
	return Collection.MostUsedEmote;
}

TArray<FMGEmoteUsageStats> UMGEmoteSubsystem::GetEmoteLeaderboard(int32 MaxEntries) const
{
	TArray<FMGEmoteUsageStats> Leaderboard;

	for (const auto& StatsPair : Collection.UsageStats)
	{
		Leaderboard.Add(StatsPair.Value);
	}

	// Sort by total uses descending
	Leaderboard.Sort([](const FMGEmoteUsageStats& A, const FMGEmoteUsageStats& B)
	{
		return A.TotalUses > B.TotalUses;
	});

	// Limit entries
	if (MaxEntries > 0 && Leaderboard.Num() > MaxEntries)
	{
		Leaderboard.SetNum(MaxEntries);
	}

	return Leaderboard;
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGEmoteSubsystem::OnEmoteTick()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	float MGDeltaTime = 0.05f; // Our tick rate

	// Update cooldowns
	TArray<FName> CooldownsToRemove;
	for (auto& CooldownPair : EmoteCooldowns)
	{
		CooldownPair.Value -= DeltaTime;
		if (CooldownPair.Value <= 0.0f)
		{
			CooldownsToRemove.Add(CooldownPair.Key);
		}
	}
	for (const FName& Key : CooldownsToRemove)
	{
		EmoteCooldowns.Remove(Key);
	}

	// Update active emote
	if (ActiveEmote.bIsPlaying)
	{
		ActiveEmote.ElapsedTime = CurrentTime - ActiveEmote.StartTime;

		const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(ActiveEmote.EmoteID);
		bool bShouldLoop = EmoteDef && EmoteDef->bLooping;

		if (!bShouldLoop && ActiveEmote.ElapsedTime >= ActiveEmote.Duration)
		{
			StopCurrentEmote();
		}
	}

	// Update remote emotes
	for (int32 i = ActiveRemoteEmotes.Num() - 1; i >= 0; --i)
	{
		FMGActiveEmote& RemoteEmote = ActiveRemoteEmotes[i];
		RemoteEmote.ElapsedTime = CurrentTime - RemoteEmote.StartTime;

		const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(RemoteEmote.EmoteID);
		bool bShouldLoop = EmoteDef && EmoteDef->bLooping;

		if (!bShouldLoop && RemoteEmote.ElapsedTime >= RemoteEmote.Duration)
		{
			FName EmoteID = RemoteEmote.EmoteID;
			FName PlayerID = RemoteEmote.PlayerID;
			ActiveRemoteEmotes.RemoveAt(i);
			OnEmoteEnded.Broadcast(PlayerID, EmoteID);
		}
	}
}

void UMGEmoteSubsystem::UpdateActiveEmotes(float MGDeltaTime)
{
	// Handled in OnEmoteTick
}

void UMGEmoteSubsystem::TrackEmoteUsage(FName EmoteID, EMGEmoteContext Context)
{
	FMGEmoteUsageStats& Stats = Collection.UsageStats.FindOrAdd(EmoteID);
	Stats.EmoteID = EmoteID;
	Stats.TotalUses++;
	Stats.LastUsed = FDateTime::UtcNow();

	if (Context == EMGEmoteContext::Victory || Context == EMGEmoteContext::Podium)
	{
		Stats.VictoryUses++;
	}

	if (const FMGEmoteDefinition* EmoteDef = EmoteDatabase.Find(EmoteID))
	{
		if (EmoteDef->EmoteType == EMGEmoteType::Taunt)
		{
			Stats.TauntUses++;
		}
	}

	// Update most used emote
	int32 HighestUses = 0;
	for (const auto& StatsPair : Collection.UsageStats)
	{
		if (StatsPair.Value.TotalUses > HighestUses)
		{
			HighestUses = StatsPair.Value.TotalUses;
			Collection.MostUsedEmote = StatsPair.Key;
		}
	}

	SaveEmoteData();
}

bool UMGEmoteSubsystem::CanPlayEmoteInContext(const FMGEmoteDefinition& Emote, EMGEmoteContext Context) const
{
	// If emote has no context restrictions, it can be played anywhere
	if (Emote.ValidContexts.Num() == 0)
	{
		return true;
	}

	// Check if context is explicitly allowed
	if (Emote.ValidContexts.Contains(Context))
	{
		return true;
	}

	// "Any" context is always valid
	if (Emote.ValidContexts.Contains(EMGEmoteContext::Any))
	{
		return true;
	}

	return false;
}

void UMGEmoteSubsystem::SaveEmoteData()
{
	// Persist emote data
	// Implementation would use USaveGame or cloud save
}

void UMGEmoteSubsystem::LoadEmoteData()
{
	// Load persisted emote data
	// Implementation would use USaveGame or cloud save
}

void UMGEmoteSubsystem::InitializeDefaultLoadout()
{
	// Create default wheel slots (8 slots like most games)
	for (int32 i = 0; i < 8; ++i)
	{
		FMGEmoteWheelSlot Slot;
		Slot.SlotIndex = i;
		ActiveLoadout.WheelSlots.Add(Slot);
	}

	// Create default quick select slots (4 slots for D-pad)
	for (int32 i = 0; i < 4; ++i)
	{
		ActiveLoadout.QuickSelectEmotes.Add(NAME_None);
	}
}
