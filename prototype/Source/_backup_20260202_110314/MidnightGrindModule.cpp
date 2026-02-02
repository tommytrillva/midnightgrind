// Copyright Midnight Grind. All Rights Reserved.

#include "MidnightGrindModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FMidnightGrindModule, MidnightGrind, "MidnightGrind");

void FMidnightGrindModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("MidnightGrind module started"));
}

void FMidnightGrindModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("MidnightGrind module shutdown"));
}
