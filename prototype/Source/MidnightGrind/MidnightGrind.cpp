// Copyright Midnight Grind. All Rights Reserved.

#include "MidnightGrind.h"
#include "Modules/ModuleManager.h"
#include "Misc/App.h"

DEFINE_LOG_CATEGORY(LogMidnightGrind);

void FMidnightGrindModule::StartupModule()
{
	UE_LOG(LogMidnightGrind, Log, TEXT("=================================================="));
	UE_LOG(LogMidnightGrind, Log, TEXT("MIDNIGHT GRIND - Arcade Street Racing"));
	UE_LOG(LogMidnightGrind, Log, TEXT("Version: 0.1.0 | UE5.7"));
	UE_LOG(LogMidnightGrind, Log, TEXT("=================================================="));
	UE_LOG(LogMidnightGrind, Log, TEXT("MidnightGrind module starting up"));

	// Log build configuration
#if UE_BUILD_DEBUG
	UE_LOG(LogMidnightGrind, Log, TEXT("Build: DEBUG"));
#elif UE_BUILD_DEVELOPMENT
	UE_LOG(LogMidnightGrind, Log, TEXT("Build: DEVELOPMENT"));
#elif UE_BUILD_TEST
	UE_LOG(LogMidnightGrind, Log, TEXT("Build: TEST"));
#elif UE_BUILD_SHIPPING
	UE_LOG(LogMidnightGrind, Log, TEXT("Build: SHIPPING"));
#endif

	// Log platform
#if PLATFORM_WINDOWS
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: Windows"));
#elif PLATFORM_MAC
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: Mac"));
#elif PLATFORM_LINUX
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: Linux"));
#elif PLATFORM_PS5
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: PlayStation 5"));
#elif PLATFORM_XSX
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: Xbox Series X|S"));
#elif PLATFORM_SWITCH
	UE_LOG(LogMidnightGrind, Log, TEXT("Platform: Nintendo Switch"));
#endif
}

void FMidnightGrindModule::ShutdownModule()
{
	UE_LOG(LogMidnightGrind, Log, TEXT("MidnightGrind module shutting down"));
	UE_LOG(LogMidnightGrind, Log, TEXT("=================================================="));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FMidnightGrindModule, MidnightGrind, "MidnightGrind");
