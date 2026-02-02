// Copyright Midnight Grind. All Rights Reserved.

#include "Campaign/MGCampaignSubsystem.h"

void UMGCampaignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentChapter = 1;
}

void UMGCampaignSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// Registration
void UMGCampaignSubsystem::RegisterMission(const FMGMissionDefinition& Mission)
{
	RegisteredMissions.Add(Mission.MissionID, Mission);

	// Initialize progress if not exists
	if (!MissionProgress.Contains(Mission.MissionID))
	{
		FMGMissionProgress NewProgress;
		NewProgress.MissionID = Mission.MissionID;
		NewProgress.Status = EMGMissionStatus::Locked;
		MissionProgress.Add(Mission.MissionID, NewProgress);
	}

	CheckMissionAvailability();
}

void UMGCampaignSubsystem::RegisterChapter(const FMGChapterDefinition& Chapter)
{
	RegisteredChapters.Add(Chapter.ChapterNumber, Chapter);
}

void UMGCampaignSubsystem::RegisterCharacter(const FMGStoryCharacter& Character)
{
	RegisteredCharacters.Add(Character.CharacterID, Character);
}

// Mission Management
bool UMGCampaignSubsystem::StartMission(FName MissionID)
{
	if (bInMission)
	{
		return false;
	}

	const FMGMissionDefinition* Mission = RegisteredMissions.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	if (!IsMissionAvailable(MissionID))
	{
		return false;
	}

	bInMission = true;
	CurrentMissionID = MissionID;
	CurrentObjectives = Mission->Objectives;

	// Update progress
	FMGMissionProgress& Progress = MissionProgress.FindOrAdd(MissionID);
	Progress.Status = EMGMissionStatus::InProgress;
	Progress.AttemptCount++;
	Progress.LastAttemptTime = FDateTime::Now();
	Progress.ObjectiveProgress = CurrentObjectives;

	// Start intro dialogue
	if (Mission->IntroDialogue.Num() > 0)
	{
		StartDialogue(Mission->IntroDialogue);
	}

	OnMissionStarted.Broadcast(MissionID);
	return true;
}

void UMGCampaignSubsystem::CompleteMission(bool bPerfect)
{
	if (!bInMission)
	{
		return;
	}

	FMGMissionProgress& Progress = MissionProgress.FindOrAdd(CurrentMissionID);
	Progress.Status = EMGMissionStatus::Completed;
	Progress.CompletionCount++;
	Progress.bPerfectCompletion = Progress.bPerfectCompletion || bPerfect;

	if (Progress.CompletionCount == 1)
	{
		Progress.FirstCompletionTime = FDateTime::Now();
	}

	// Show outro dialogue
	const FMGMissionDefinition* Mission = RegisteredMissions.Find(CurrentMissionID);
	if (Mission && Mission->OutroDialogue.Num() > 0)
	{
		StartDialogue(Mission->OutroDialogue);
	}

	OnMissionCompleted.Broadcast(CurrentMissionID, bPerfect);

	bInMission = false;
	CurrentMissionID = NAME_None;
	CurrentObjectives.Empty();

	CheckMissionAvailability();
	CheckChapterCompletion();
}

void UMGCampaignSubsystem::FailMission()
{
	if (!bInMission)
	{
		return;
	}

	FMGMissionProgress& Progress = MissionProgress.FindOrAdd(CurrentMissionID);
	Progress.Status = EMGMissionStatus::Failed;

	// Show fail dialogue
	const FMGMissionDefinition* Mission = RegisteredMissions.Find(CurrentMissionID);
	if (Mission && Mission->FailDialogue.Num() > 0)
	{
		StartDialogue(Mission->FailDialogue);
	}

	OnMissionFailed.Broadcast(CurrentMissionID);

	bInMission = false;
	CurrentMissionID = NAME_None;
	CurrentObjectives.Empty();
}

void UMGCampaignSubsystem::AbandonMission()
{
	if (!bInMission)
	{
		return;
	}

	FMGMissionProgress& Progress = MissionProgress.FindOrAdd(CurrentMissionID);
	Progress.Status = EMGMissionStatus::Available;

	bInMission = false;
	CurrentMissionID = NAME_None;
	CurrentObjectives.Empty();
}

FMGMissionDefinition UMGCampaignSubsystem::GetCurrentMission() const
{
	return GetMission(CurrentMissionID);
}

FMGMissionDefinition UMGCampaignSubsystem::GetMission(FName MissionID) const
{
	const FMGMissionDefinition* Mission = RegisteredMissions.Find(MissionID);
	return Mission ? *Mission : FMGMissionDefinition();
}

FMGMissionProgress UMGCampaignSubsystem::GetMissionProgress(FName MissionID) const
{
	const FMGMissionProgress* Progress = MissionProgress.Find(MissionID);
	return Progress ? *Progress : FMGMissionProgress();
}

TArray<FMGMissionDefinition> UMGCampaignSubsystem::GetAvailableMissions() const
{
	TArray<FMGMissionDefinition> Result;

	for (const auto& Pair : RegisteredMissions)
	{
		if (IsMissionAvailable(Pair.Key))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGMissionDefinition> UMGCampaignSubsystem::GetMissionsByType(EMGMissionType Type) const
{
	TArray<FMGMissionDefinition> Result;

	for (const auto& Pair : RegisteredMissions)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

bool UMGCampaignSubsystem::IsMissionAvailable(FName MissionID) const
{
	const FMGMissionProgress* Progress = MissionProgress.Find(MissionID);
	if (!Progress)
	{
		return false;
	}

	if (Progress->Status == EMGMissionStatus::Locked)
	{
		return false;
	}

	if (Progress->Status == EMGMissionStatus::InProgress)
	{
		return false;
	}

	const FMGMissionDefinition* Mission = RegisteredMissions.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	// Check if completed and not replayable
	if (Progress->Status == EMGMissionStatus::Completed && !Mission->bIsReplayable)
	{
		return false;
	}

	return CheckPrerequisites(*Mission);
}

bool UMGCampaignSubsystem::IsMissionCompleted(FName MissionID) const
{
	const FMGMissionProgress* Progress = MissionProgress.Find(MissionID);
	return Progress && Progress->Status == EMGMissionStatus::Completed;
}

// Objectives
void UMGCampaignSubsystem::UpdateObjective(FName ObjectiveID, int32 NewValue)
{
	for (FMGMissionObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			Objective.CurrentValue = NewValue;

			OnObjectiveUpdated.Broadcast(CurrentMissionID, ObjectiveID);

			if (Objective.CurrentValue >= Objective.TargetValue && !Objective.bIsComplete)
			{
				CompleteObjective(ObjectiveID);
			}
			break;
		}
	}
}

void UMGCampaignSubsystem::IncrementObjective(FName ObjectiveID, int32 Amount)
{
	for (FMGMissionObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			UpdateObjective(ObjectiveID, Objective.CurrentValue + Amount);
			break;
		}
	}
}

void UMGCampaignSubsystem::CompleteObjective(FName ObjectiveID)
{
	for (FMGMissionObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			Objective.bIsComplete = true;
			Objective.CurrentValue = Objective.TargetValue;

			OnObjectiveCompleted.Broadcast(CurrentMissionID, ObjectiveID);

			// Check if all required objectives complete
			bool bAllRequiredComplete = true;
			bool bAllOptionalComplete = true;

			for (const FMGMissionObjective& Obj : CurrentObjectives)
			{
				if (!Obj.bIsComplete)
				{
					if (Obj.bIsOptional)
					{
						bAllOptionalComplete = false;
					}
					else
					{
						bAllRequiredComplete = false;
					}
				}
			}

			if (bAllRequiredComplete)
			{
				CompleteMission(bAllOptionalComplete);
			}
			break;
		}
	}
}

TArray<FMGMissionObjective> UMGCampaignSubsystem::GetCurrentObjectives() const
{
	return CurrentObjectives;
}

FMGMissionObjective UMGCampaignSubsystem::GetObjective(FName ObjectiveID) const
{
	for (const FMGMissionObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			return Objective;
		}
	}
	return FMGMissionObjective();
}

// Chapters
FMGChapterDefinition UMGCampaignSubsystem::GetChapter(int32 ChapterNumber) const
{
	const FMGChapterDefinition* Chapter = RegisteredChapters.Find(ChapterNumber);
	return Chapter ? *Chapter : FMGChapterDefinition();
}

bool UMGCampaignSubsystem::IsChapterUnlocked(int32 ChapterNumber) const
{
	if (ChapterNumber <= 1)
	{
		return true;
	}

	return IsChapterCompleted(ChapterNumber - 1);
}

bool UMGCampaignSubsystem::IsChapterCompleted(int32 ChapterNumber) const
{
	return CompletedChapters.Contains(ChapterNumber);
}

float UMGCampaignSubsystem::GetChapterProgress(int32 ChapterNumber) const
{
	const FMGChapterDefinition* Chapter = RegisteredChapters.Find(ChapterNumber);
	if (!Chapter || Chapter->MissionIDs.Num() == 0)
	{
		return 0.0f;
	}

	int32 Completed = 0;
	for (FName MissionID : Chapter->MissionIDs)
	{
		if (IsMissionCompleted(MissionID))
		{
			Completed++;
		}
	}

	return static_cast<float>(Completed) / Chapter->MissionIDs.Num();
}

// Characters
FMGStoryCharacter UMGCampaignSubsystem::GetCharacter(FName CharacterID) const
{
	const FMGStoryCharacter* Character = RegisteredCharacters.Find(CharacterID);
	return Character ? *Character : FMGStoryCharacter();
}

TArray<FMGStoryCharacter> UMGCampaignSubsystem::GetAllCharacters() const
{
	TArray<FMGStoryCharacter> Result;
	RegisteredCharacters.GenerateValueArray(Result);
	return Result;
}

TArray<FMGStoryCharacter> UMGCampaignSubsystem::GetCharactersByRole(EMGCharacterRole Role) const
{
	TArray<FMGStoryCharacter> Result;

	for (const auto& Pair : RegisteredCharacters)
	{
		if (Pair.Value.Role == Role)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

void UMGCampaignSubsystem::ModifyRelationship(FName CharacterID, int32 Amount)
{
	FMGStoryCharacter* Character = RegisteredCharacters.Find(CharacterID);
	if (Character)
	{
		int32 OldLevel = Character->RelationshipLevel;
		Character->RelationshipLevel = FMath::Clamp(Character->RelationshipLevel + Amount, -100, 100);

		if (OldLevel != Character->RelationshipLevel)
		{
			OnCharacterRelationshipChanged.Broadcast(CharacterID, Character->RelationshipLevel);
		}
	}
}

int32 UMGCampaignSubsystem::GetRelationship(FName CharacterID) const
{
	const FMGStoryCharacter* Character = RegisteredCharacters.Find(CharacterID);
	return Character ? Character->RelationshipLevel : 0;
}

// Dialogue
void UMGCampaignSubsystem::StartDialogue(const TArray<FMGDialogueLine>& Dialogue)
{
	if (Dialogue.Num() == 0)
	{
		return;
	}

	CurrentDialogue = Dialogue;
	CurrentDialogueIndex = 0;
	bInDialogue = true;

	OnDialogueStarted.Broadcast(Dialogue);
}

void UMGCampaignSubsystem::AdvanceDialogue()
{
	if (!bInDialogue)
	{
		return;
	}

	CurrentDialogueIndex++;

	if (CurrentDialogueIndex >= CurrentDialogue.Num())
	{
		bInDialogue = false;
		CurrentDialogue.Empty();
		CurrentDialogueIndex = 0;
		OnDialogueEnded.Broadcast();
	}
}

void UMGCampaignSubsystem::SkipDialogue()
{
	if (!bInDialogue)
	{
		return;
	}

	bInDialogue = false;
	CurrentDialogue.Empty();
	CurrentDialogueIndex = 0;
	OnDialogueEnded.Broadcast();
}

FMGDialogueLine UMGCampaignSubsystem::GetCurrentDialogueLine() const
{
	if (bInDialogue && CurrentDialogue.IsValidIndex(CurrentDialogueIndex))
	{
		return CurrentDialogue[CurrentDialogueIndex];
	}
	return FMGDialogueLine();
}

// Progress/Stats
float UMGCampaignSubsystem::GetOverallStoryProgress() const
{
	int32 TotalStoryMissions = 0;
	int32 CompletedStoryMissions = 0;

	for (const auto& Pair : RegisteredMissions)
	{
		if (Pair.Value.Type == EMGMissionType::Story)
		{
			TotalStoryMissions++;
			if (IsMissionCompleted(Pair.Key))
			{
				CompletedStoryMissions++;
			}
		}
	}

	if (TotalStoryMissions == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(CompletedStoryMissions) / TotalStoryMissions;
}

int32 UMGCampaignSubsystem::GetTotalMissionsCompleted() const
{
	int32 Count = 0;
	for (const auto& Pair : MissionProgress)
	{
		if (Pair.Value.Status == EMGMissionStatus::Completed)
		{
			Count++;
		}
	}
	return Count;
}

int32 UMGCampaignSubsystem::GetTotalPerfectCompletions() const
{
	int32 Count = 0;
	for (const auto& Pair : MissionProgress)
	{
		if (Pair.Value.bPerfectCompletion)
		{
			Count++;
		}
	}
	return Count;
}

// Save/Load
TArray<FMGMissionProgress> UMGCampaignSubsystem::GetAllMissionProgress() const
{
	TArray<FMGMissionProgress> Result;
	MissionProgress.GenerateValueArray(Result);
	return Result;
}

void UMGCampaignSubsystem::LoadMissionProgress(const TArray<FMGMissionProgress>& Progress)
{
	for (const FMGMissionProgress& P : Progress)
	{
		MissionProgress.Add(P.MissionID, P);
	}

	CheckMissionAvailability();
}

// Internal
void UMGCampaignSubsystem::CheckMissionAvailability()
{
	for (auto& Pair : MissionProgress)
	{
		if (Pair.Value.Status == EMGMissionStatus::Locked)
		{
			const FMGMissionDefinition* Mission = RegisteredMissions.Find(Pair.Key);
			if (Mission && CheckPrerequisites(*Mission))
			{
				Pair.Value.Status = EMGMissionStatus::Available;
				OnMissionAvailable.Broadcast(Pair.Key);
			}
		}
	}
}

void UMGCampaignSubsystem::CheckChapterCompletion()
{
	for (const auto& Pair : RegisteredChapters)
	{
		if (CompletedChapters.Contains(Pair.Key))
		{
			continue;
		}

		bool bAllComplete = true;
		for (FName MissionID : Pair.Value.MissionIDs)
		{
			if (!IsMissionCompleted(MissionID))
			{
				bAllComplete = false;
				break;
			}
		}

		if (bAllComplete)
		{
			CompletedChapters.Add(Pair.Key);
			OnChapterCompleted.Broadcast(Pair.Key);

			// Unlock next chapter
			int32 NextChapter = Pair.Key + 1;
			if (RegisteredChapters.Contains(NextChapter))
			{
				CurrentChapter = NextChapter;
				OnChapterUnlocked.Broadcast(NextChapter);
			}
		}
	}
}

bool UMGCampaignSubsystem::CheckPrerequisites(const FMGMissionDefinition& Mission) const
{
	// Check required missions
	for (FName RequiredID : Mission.RequiredMissions)
	{
		if (!IsMissionCompleted(RequiredID))
		{
			return false;
		}
	}

	// Could also check REP, level, vehicle requirements here
	// Would need access to save/player subsystem

	return true;
}
