// Copyright Midnight Grind. All Rights Reserved.

#include "Save/MG_SAVE_Game.h"

UMGSaveGame::UMGSaveGame()
{
	SaveSlotName = GetDefaultSaveSlotName();
	SaveTimestamp = FDateTime::UtcNow();
	SaveVersion = 1;
}

FString UMGSaveGame::GetDefaultSaveSlotName()
{
	return TEXT("MidnightGrindSave");
}

int32 UMGSaveGame::GetDefaultUserIndex()
{
	return 0;
}
