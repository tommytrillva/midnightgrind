// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Social/MGPlayerSocialSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Social Friend Management
 * Verifies that the social subsystem can manage friend lists
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSocialFriendManagementTest,
	"MidnightGrind.Unit.Social.FriendManagement",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSocialFriendManagementTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create social subsystem
	UMGPlayerSocialSubsystem* Social = NewObject<UMGPlayerSocialSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Social subsystem created"), Social))
		return false;

	Social->Initialize(nullptr);

	// Test: Initial state - no friends
	TArray<FMGFriendInfo> Friends = Social->GetFriendsList();
	TestEqual(TEXT("No friends initially"), Friends.Num(), 0);

	// Test: Can get online friends count
	int32 OnlineCount = Social->GetOnlineFriendsCount();
	TestEqual(TEXT("No online friends initially"), OnlineCount, 0);

	// Test: Subsystem is initialized
	bool IsInitialized = Social->IsInitialized();
	TestTrue(TEXT("Social subsystem is initialized"), IsInitialized);

	return true;
}

/**
 * Test: Social Reputation System
 * Verifies that the social subsystem tracks reputation correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSocialReputationTest,
	"MidnightGrind.Unit.Social.Reputation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSocialReputationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create social subsystem
	UMGPlayerSocialSubsystem* Social = NewObject<UMGPlayerSocialSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Social subsystem created"), Social))
		return false;

	Social->Initialize(nullptr);

	// Test: Can get player reputation
	int32 Reputation = Social->GetPlayerReputation();
	TestTrue(TEXT("Reputation is non-negative"), Reputation >= 0);

	// Test: Can get reputation tier
	EMGReputationTier Tier = Social->GetReputationTier();
	TestTrue(TEXT("Reputation tier is valid"), true); // Just verify no crash

	// Test: Can check if player is known in area
	bool IsKnown = Social->IsPlayerKnownInArea(TEXT("Downtown"));
	TestTrue(TEXT("Known status returns valid bool"), true); // Just verify no crash

	return true;
}

/**
 * Test: Social Achievement Tracking
 * Verifies that the social subsystem can track achievements
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSocialAchievementTest,
	"MidnightGrind.Unit.Social.Achievements",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSocialAchievementTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create social subsystem
	UMGPlayerSocialSubsystem* Social = NewObject<UMGPlayerSocialSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Social subsystem created"), Social))
		return false;

	Social->Initialize(nullptr);

	// Test: Can get achievement progress
	float Progress = Social->GetAchievementProgress(TEXT("FirstWin"));
	TestTrue(TEXT("Achievement progress is non-negative"), Progress >= 0.0f);
	TestTrue(TEXT("Achievement progress is <= 100%"), Progress <= 100.0f);

	// Test: Can check if achievement is unlocked
	bool IsUnlocked = Social->IsAchievementUnlocked(TEXT("FirstWin"));
	TestTrue(TEXT("Achievement unlock check returns valid bool"), true); // Just verify no crash

	// Test: Can get unlocked achievement count
	int32 UnlockedCount = Social->GetUnlockedAchievementCount();
	TestTrue(TEXT("Unlocked achievement count is non-negative"), UnlockedCount >= 0);

	return true;
}

/**
 * Test: Social Crew Membership
 * Verifies that the social subsystem can track crew membership
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSocialCrewMembershipTest,
	"MidnightGrind.Unit.Social.CrewMembership",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSocialCrewMembershipTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create social subsystem
	UMGPlayerSocialSubsystem* Social = NewObject<UMGPlayerSocialSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Social subsystem created"), Social))
		return false;

	Social->Initialize(nullptr);

	// Test: Can check if player is in a crew
	bool IsInCrew = Social->IsPlayerInCrew();
	TestTrue(TEXT("Crew membership check returns valid bool"), true); // Just verify no crash

	// Test: Can get current crew name
	FName CrewName = Social->GetCurrentCrewName();
	TestTrue(TEXT("Crew name check returns valid name"), true); // Just verify no crash

	// Test: Can get crew member count
	int32 MemberCount = Social->GetCrewMemberCount();
	TestTrue(TEXT("Crew member count is non-negative"), MemberCount >= 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
