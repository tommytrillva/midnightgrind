// Copyright Midnight Grind. All Rights Reserved.

#include "MidnightGrind.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogMidnightGrind);

void FMidnightGrindModule::StartupModule()
{
	UE_LOG(LogMidnightGrind, Log, TEXT("MidnightGrind module starting up"));
}

void FMidnightGrindModule::ShutdownModule()
{
	UE_LOG(LogMidnightGrind, Log, TEXT("MidnightGrind module shutting down"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FMidnightGrindModule, MidnightGrind, "MidnightGrind");
