// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPlatformIntegrationSubsystem.h
 * @brief Platform Services Integration for achievements, presence, entitlements, and more
 *
 * @section Overview
 * This file defines the Platform Integration Subsystem, which connects the game
 * to platform-specific features like achievements, rich presence, friends lists,
 * entitlements (DLC), and platform overlays. Each platform (Steam, PlayStation,
 * Xbox, etc.) has its own APIs for these features, and this subsystem provides
 * a unified interface.
 *
 * @section WhatIsPlatform What Does "Platform Integration" Mean?
 * When you play a game on Steam, you see Steam achievements pop up, your Steam
 * friends can see you're playing, and Steam handles your purchases. On PlayStation,
 * you get trophies instead, see PSN friends, etc. This subsystem abstracts away
 * those differences so game code can just call "UnlockAchievement" without caring
 * which platform it's running on.
 *
 * @section KeyFeatures Key Features
 *
 * @subsection Achievements 1. Achievements/Trophies
 * Progress tracking and unlocks:
 *   - Steam Achievements
 *   - PlayStation Trophies
 *   - Xbox Achievements
 *   - Google Play / Game Center achievements
 *
 * @subsection RichPresence 2. Rich Presence
 * Shows what you're doing in friends lists:
 *   - "Racing at Midnight Boulevard"
 *   - "Customizing 1999 Skyline GT-R"
 *   - "In Party (3/8)"
 *
 * @subsection Entitlements 3. Entitlements
 * Verifying ownership of DLC, season passes, etc.:
 *   - Did they buy the Deluxe Edition?
 *   - Do they own the Japan Car Pack DLC?
 *   - Is their subscription active?
 *
 * @subsection Friends 4. Friends
 * Access to platform friend lists:
 *   - See who's online
 *   - See who's playing the game
 *   - Invite friends to races
 *
 * @subsection Overlay 5. Overlay
 * Platform UI that appears over the game:
 *   - Steam overlay (Shift+Tab)
 *   - PlayStation quick menu
 *   - Xbox guide
 *
 * @subsection Capture 6. Screenshots/Video
 * Platform capture features:
 *   - Trigger screenshots
 *   - Start/stop video recording
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * @subsection PlatformType 1. Platform Type
 * Which gaming platform (Steam, Epic, PlayStation 4/5, Xbox One/Series,
 * Nintendo Switch, iOS, Android). Each has different capabilities.
 *
 * @subsection Capabilities 2. Capabilities
 * What features the current platform supports. Not all platforms have all
 * features (e.g., Nintendo Switch doesn't have achievements the same way Steam does).
 *
 * @subsection AchievementStatus 3. Achievement Status
 *   - Locked: Not yet earned
 *   - InProgress: Partially complete (e.g., "Win 5/10 races")
 *   - Unlocked: Fully earned
 *   - Hidden: Secret achievement, description hidden until unlocked
 *
 * @subsection PresenceState 4. Presence State
 * What the player is currently doing:
 *   - Online, Away, Busy
 *   - InGame, InRace, InMenu, InGarage
 *   - Matchmaking, Spectating
 *
 * @subsection EntitlementType 5. Entitlement Type
 * Categories of purchasable content:
 *   - BaseGame: The main game itself
 *   - DLC: Downloadable content packs
 *   - SeasonPass: Bundle of future DLC
 *   - Subscription: Time-limited access
 *   - Microtransaction: In-game purchases
 *
 * @section PlatformNotes Platform-Specific Notes
 *
 * @subsection SteamNotes STEAM
 *   - Rich achievements system with global unlock percentages
 *   - Steamworks overlay with browser, chat, etc.
 *   - Steam Cloud for save data
 *
 * @subsection PSNotes PLAYSTATION
 *   - Trophies (Bronze, Silver, Gold, Platinum)
 *   - Activity Cards on PS5
 *   - PlayStation Network friends
 *
 * @subsection XboxNotes XBOX
 *   - Gamerscore achievements
 *   - Xbox Game Bar
 *   - Xbox Live friends and parties
 *
 * @subsection SwitchNotes NINTENDO SWITCH
 *   - Limited online features
 *   - Nintendo Switch Online required for multiplayer
 *
 * @subsection MobileNotes MOBILE (iOS/Android)
 *   - Game Center / Google Play Games achievements
 *   - No overlay system
 *   - In-app purchases
 *
 * @section CodeExamples Code Examples
 *
 * @subsection GettingSubsystem Getting the Subsystem
 * @code
 * UMGPlatformIntegrationSubsystem* Platform =
 *     GetGameInstance()->GetSubsystem<UMGPlatformIntegrationSubsystem>();
 * @endcode
 *
 * @subsection AchievementExample Unlocking Achievements
 * @code
 * // Unlock an achievement when player wins their first race
 * void ARaceManager::OnRaceFinished(int32 Position)
 * {
 *     if (Position == 1)
 *     {
 *         Platform->UnlockAchievement(FName("FirstVictory"));
 *     }
 * }
 *
 * // Update achievement progress (win 10 races)
 * void ARaceManager::OnRaceWon()
 * {
 *     WinCount++;
 *     Platform->UpdateAchievementProgress(FName("TenWins"), static_cast<float>(WinCount));
 * }
 *
 * // Increment progress by a specific amount
 * Platform->IncrementAchievementProgress(FName("DistanceDriven"), DistanceThisRace);
 * @endcode
 *
 * @subsection PresenceExample Setting Rich Presence
 * @code
 * // Set presence when entering different game states
 * void AGameMode::OnStateChanged(EGameState NewState)
 * {
 *     switch (NewState)
 *     {
 *         case EGameState::InRace:
 *             Platform->SetPresenceInRace(TrackId, VehicleId, Position, TotalRacers);
 *             break;
 *         case EGameState::InGarage:
 *             Platform->SetPresenceInGarage(CurrentVehicleId);
 *             break;
 *         case EGameState::InMenu:
 *             Platform->SetPresenceInMenu("Main Menu");
 *             break;
 *         case EGameState::Matchmaking:
 *             Platform->SetPresenceMatchmaking("Quick Race");
 *             break;
 *     }
 * }
 * @endcode
 *
 * @subsection EntitlementExample Checking Entitlements
 * @code
 * // Check if player owns DLC before showing DLC content
 * void ACarDealership::PopulateVehicleList()
 * {
 *     // Show base game vehicles
 *     AddBaseVehicles();
 *
 *     // Show DLC vehicles if owned
 *     if (Platform->HasDLC(FName("JapanCarPack")))
 *     {
 *         AddJapanCarPackVehicles();
 *     }
 *
 *     if (Platform->HasSeasonPass())
 *     {
 *         AddSeasonPassBonusVehicle();
 *     }
 * }
 * @endcode
 *
 * @subsection FriendsExample Working with Friends
 * @code
 * // Get friends who are playing the game
 * TArray<FMGPlatformUser> PlayingFriends = Platform->GetFriendsPlayingGame();
 *
 * for (const FMGPlatformUser& Friend : PlayingFriends)
 * {
 *     DisplayFriendInLobby(Friend.DisplayName, Friend.PresenceText);
 * }
 *
 * // Show platform friend list UI
 * Platform->ShowFriendsUI();
 * @endcode
 *
 * @subsection EventsExample Listening for Events
 * @code
 * // Bind to platform events
 * Platform->OnAchievementUnlocked.AddDynamic(this, &AMyClass::HandleAchievement);
 * Platform->OnOverlayActivated.AddDynamic(this, &AMyClass::HandleOverlayOpen);
 * Platform->OnOverlayDeactivated.AddDynamic(this, &AMyClass::HandleOverlayClose);
 *
 * void AMyClass::HandleAchievement(const FMGPlatformAchievement& Achievement)
 * {
 *     // Show custom achievement celebration
 *     ShowAchievementPopup(Achievement.DisplayName, Achievement.Description);
 * }
 *
 * void AMyClass::HandleOverlayOpen()
 * {
 *     // Pause game when platform overlay opens
 *     PauseGame();
 * }
 *
 * void AMyClass::HandleOverlayClose()
 * {
 *     // Resume game when overlay closes
 *     ResumeGame();
 * }
 * @endcode
 *
 * @section Performance Performance Considerations
 *   - Achievement updates are batched; don't call every frame
 *   - Rich presence updates have rate limits (typically once per 15-30 seconds)
 *   - Friends list queries can be slow; cache results
 *
 * @see MGAccountLinkSubsystem.h Manages cross-platform account identity
 * @see MGCrossPlaySubsystem.h Cross-platform multiplayer
 * @see MGCrossProgressionSubsystem.h Cross-platform save data
 *
 * @author Midnight Grind Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPlatformIntegrationSubsystem.generated.h"

// Forward declarations
class UMGPlatformIntegrationSubsystem;

/**
 * EMGPlatformType - Supported gaming platforms
 */
UENUM(BlueprintType)
enum class EMGPlatformType : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    PC_Steam        UMETA(DisplayName = "PC - Steam"),
    PC_Epic         UMETA(DisplayName = "PC - Epic Games"),
    PC_GOG          UMETA(DisplayName = "PC - GOG"),
    PC_Windows      UMETA(DisplayName = "PC - Windows Store"),
    PlayStation4    UMETA(DisplayName = "PlayStation 4"),
    PlayStation5    UMETA(DisplayName = "PlayStation 5"),
    XboxOne         UMETA(DisplayName = "Xbox One"),
    XboxSeriesX     UMETA(DisplayName = "Xbox Series X|S"),
    NintendoSwitch  UMETA(DisplayName = "Nintendo Switch"),
    Mobile_iOS      UMETA(DisplayName = "Mobile - iOS"),
    Mobile_Android  UMETA(DisplayName = "Mobile - Android")
};

/**
 * EMGAchievementStatus - Status of platform achievements
 */
UENUM(BlueprintType)
enum class EMGAchievementStatus : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Unlocked        UMETA(DisplayName = "Unlocked"),
    Hidden          UMETA(DisplayName = "Hidden")
};

/**
 * EMGPresenceState - Player presence states
 */
UENUM(BlueprintType)
enum class EMGPresenceState : uint8
{
    Offline         UMETA(DisplayName = "Offline"),
    Online          UMETA(DisplayName = "Online"),
    Away            UMETA(DisplayName = "Away"),
    Busy            UMETA(DisplayName = "Busy"),
    InGame          UMETA(DisplayName = "In Game"),
    InRace          UMETA(DisplayName = "In Race"),
    InMenu          UMETA(DisplayName = "In Menu"),
    InGarage        UMETA(DisplayName = "In Garage"),
    Matchmaking     UMETA(DisplayName = "Matchmaking"),
    Spectating      UMETA(DisplayName = "Spectating")
};

/**
 * EMGOnlineCapability - Platform online capabilities
 */
UENUM(BlueprintType)
enum class EMGOnlineCapability : uint8
{
    None            UMETA(DisplayName = "None"),
    Multiplayer     UMETA(DisplayName = "Multiplayer"),
    Leaderboards    UMETA(DisplayName = "Leaderboards"),
    CloudSave       UMETA(DisplayName = "Cloud Save"),
    Achievements    UMETA(DisplayName = "Achievements"),
    Friends         UMETA(DisplayName = "Friends"),
    VoiceChat       UMETA(DisplayName = "Voice Chat"),
    Streaming       UMETA(DisplayName = "Streaming"),
    UGC             UMETA(DisplayName = "User Generated Content")
};

/**
 * EMGEntitlementType - Types of platform entitlements
 */
UENUM(BlueprintType)
enum class EMGEntitlementType : uint8
{
    BaseGame        UMETA(DisplayName = "Base Game"),
    DLC             UMETA(DisplayName = "DLC"),
    SeasonPass      UMETA(DisplayName = "Season Pass"),
    Subscription    UMETA(DisplayName = "Subscription"),
    Microtransaction UMETA(DisplayName = "Microtransaction"),
    PreOrder        UMETA(DisplayName = "Pre-Order Bonus"),
    DeluxeEdition   UMETA(DisplayName = "Deluxe Edition"),
    UltimateEdition UMETA(DisplayName = "Ultimate Edition")
};

/**
 * FMGPlatformUser - Platform-specific user data
 */
USTRUCT(BlueprintType)
struct FMGPlatformUser
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformUserId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AvatarUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPlatformType Platform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPresenceState PresenceState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PresenceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFriend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GamerScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    FMGPlatformUser()
        : Platform(EMGPlatformType::Unknown)
        , PresenceState(EMGPresenceState::Offline)
        , bIsOnline(false)
        , bIsFriend(false)
        , bIsBlocked(false)
        , GamerScore(0)
        , Level(1)
    {}
};

/**
 * FMGPlatformAchievement - Platform achievement definition
 */
USTRUCT(BlueprintType)
struct FMGPlatformAchievement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AchievementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformAchievementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText HiddenDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGAchievementStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRare;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GlobalUnlockPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> LockedIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> UnlockedIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime UnlockTime;

    FMGPlatformAchievement()
        : AchievementId(NAME_None)
        , Status(EMGAchievementStatus::Locked)
        , Progress(0.0f)
        , MaxProgress(1.0f)
        , PointValue(10)
        , bIsHidden(false)
        , bIsRare(false)
        , GlobalUnlockPercentage(0.0f)
    {}

    bool IsComplete() const { return Progress >= MaxProgress; }
    float GetProgressPercent() const { return MaxProgress > 0.0f ? Progress / MaxProgress : 0.0f; }
};

/**
 * FMGRichPresence - Rich presence data for platforms
 */
USTRUCT(BlueprintType)
struct FMGRichPresence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPresenceState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Details;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LargeImageKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LargeImageText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SmallImageKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SmallImageText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PartyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PartySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PartyMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bJoinable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString JoinSecret;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentVehicle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentMode;

    FMGRichPresence()
        : State(EMGPresenceState::Online)
        , PartySize(1)
        , PartyMax(1)
        , bJoinable(false)
    {}
};

/**
 * FMGEntitlement - Platform entitlement data
 */
USTRUCT(BlueprintType)
struct FMGEntitlement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EntitlementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformEntitlementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEntitlementType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOwned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConsumed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PurchaseDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ExpirationDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedContent;

    FMGEntitlement()
        : EntitlementId(NAME_None)
        , Type(EMGEntitlementType::BaseGame)
        , bOwned(false)
        , bConsumed(false)
    {}

    bool IsValid() const { return bOwned && (ExpirationDate == FDateTime() || FDateTime::Now() < ExpirationDate); }
};

/**
 * FMGPlatformCapabilities - Platform capability flags
 */
USTRUCT(BlueprintType)
struct FMGPlatformCapabilities
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsAchievements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsLeaderboards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsCloudSave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsFriends;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsRichPresence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsUGC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsCrossPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsInvites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsOverlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsScreenshots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsVideoCapture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPartySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxFriends;

    FMGPlatformCapabilities()
        : bSupportsAchievements(true)
        , bSupportsLeaderboards(true)
        , bSupportsCloudSave(true)
        , bSupportsFriends(true)
        , bSupportsRichPresence(true)
        , bSupportsVoiceChat(true)
        , bSupportsUGC(false)
        , bSupportsStreaming(true)
        , bSupportsCrossPlay(true)
        , bSupportsInvites(true)
        , bSupportsOverlay(true)
        , bSupportsScreenshots(true)
        , bSupportsVideoCapture(true)
        , MaxPartySize(8)
        , MaxFriends(1000)
    {}
};

/**
 * FMGPlatformStats - Platform-specific statistics
 */
USTRUCT(BlueprintType)
struct FMGPlatformStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StatId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformStatId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsIncremental;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdated;

    FMGPlatformStats()
        : StatId(NAME_None)
        , Value(0.0f)
        , MinValue(0.0f)
        , MaxValue(999999.0f)
        , bIsIncremental(true)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlatformInitialized, EMGPlatformType, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnUserLoggedIn, const FMGPlatformUser&, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnUserLoggedOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAchievementUnlocked, const FMGPlatformAchievement&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAchievementProgressUpdated, FName, AchievementId, float, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEntitlementUpdated, const FMGEntitlement&, Entitlement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPresenceUpdated, const FMGRichPresence&, Presence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnFriendPresenceChanged, const FMGPlatformUser&, Friend, EMGPresenceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnOverlayActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnOverlayDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlatformError, const FString&, ErrorMessage);

/**
 * UMGPlatformIntegrationSubsystem
 *
 * Manages platform-specific integrations for Midnight Grind.
 * Features include:
 * - Platform detection and capability queries
 * - Achievement system integration
 * - Rich presence updates
 * - Entitlement verification
 * - Platform statistics
 * - Friend list integration
 * - Overlay detection
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPlatformIntegrationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGPlatformIntegrationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Platform Info =====

    UFUNCTION(BlueprintPure, Category = "Platform|Info")
    EMGPlatformType GetCurrentPlatform() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Info")
    FMGPlatformCapabilities GetPlatformCapabilities() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Info")
    bool HasCapability(EMGOnlineCapability Capability) const;

    UFUNCTION(BlueprintPure, Category = "Platform|Info")
    FString GetPlatformName() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Info")
    bool IsPlatformOnline() const;

    // ===== User Management =====

    UFUNCTION(BlueprintPure, Category = "Platform|User")
    FMGPlatformUser GetCurrentUser() const;

    UFUNCTION(BlueprintPure, Category = "Platform|User")
    bool IsUserLoggedIn() const;

    UFUNCTION(BlueprintCallable, Category = "Platform|User")
    void RequestLogin();

    UFUNCTION(BlueprintCallable, Category = "Platform|User")
    void RequestLogout();

    UFUNCTION(BlueprintPure, Category = "Platform|User")
    FString GetUserDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Platform|User")
    FString GetUserId() const;

    // ===== Achievements =====

    UFUNCTION(BlueprintCallable, Category = "Platform|Achievements")
    void UnlockAchievement(FName AchievementId);

    UFUNCTION(BlueprintCallable, Category = "Platform|Achievements")
    void UpdateAchievementProgress(FName AchievementId, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Platform|Achievements")
    void IncrementAchievementProgress(FName AchievementId, float Amount);

    UFUNCTION(BlueprintPure, Category = "Platform|Achievements")
    FMGPlatformAchievement GetAchievement(FName AchievementId) const;

    UFUNCTION(BlueprintPure, Category = "Platform|Achievements")
    TArray<FMGPlatformAchievement> GetAllAchievements() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Achievements")
    TArray<FMGPlatformAchievement> GetUnlockedAchievements() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Achievements")
    float GetAchievementCompletionPercent() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Achievements")
    int32 GetTotalGamerScore() const;

    UFUNCTION(BlueprintCallable, Category = "Platform|Achievements")
    void ShowAchievementUI();

    // ===== Rich Presence =====

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetRichPresence(const FMGRichPresence& Presence);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceState(EMGPresenceState State);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceDetails(const FString& Details);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceInRace(FName TrackId, FName VehicleId, int32 Position, int32 TotalRacers);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceInGarage(FName VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceInMenu(const FString& MenuName);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void SetPresenceMatchmaking(const FString& ModeSearching);

    UFUNCTION(BlueprintCallable, Category = "Platform|Presence")
    void ClearRichPresence();

    UFUNCTION(BlueprintPure, Category = "Platform|Presence")
    FMGRichPresence GetCurrentPresence() const;

    // ===== Entitlements =====

    UFUNCTION(BlueprintCallable, Category = "Platform|Entitlements")
    void RefreshEntitlements();

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    bool HasEntitlement(FName EntitlementId) const;

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    FMGEntitlement GetEntitlement(FName EntitlementId) const;

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    TArray<FMGEntitlement> GetAllEntitlements() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    TArray<FMGEntitlement> GetOwnedEntitlements() const;

    UFUNCTION(BlueprintCallable, Category = "Platform|Entitlements")
    void ConsumeEntitlement(FName EntitlementId);

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    bool HasDLC(FName DLCId) const;

    UFUNCTION(BlueprintPure, Category = "Platform|Entitlements")
    bool HasSeasonPass() const;

    // ===== Platform Stats =====

    UFUNCTION(BlueprintCallable, Category = "Platform|Stats")
    void SetPlatformStat(FName StatId, float Value);

    UFUNCTION(BlueprintCallable, Category = "Platform|Stats")
    void IncrementPlatformStat(FName StatId, float Amount);

    UFUNCTION(BlueprintPure, Category = "Platform|Stats")
    float GetPlatformStat(FName StatId) const;

    UFUNCTION(BlueprintCallable, Category = "Platform|Stats")
    void FlushPlatformStats();

    // ===== Friends =====

    UFUNCTION(BlueprintPure, Category = "Platform|Friends")
    TArray<FMGPlatformUser> GetFriendsList() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Friends")
    TArray<FMGPlatformUser> GetOnlineFriends() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Friends")
    TArray<FMGPlatformUser> GetFriendsPlayingGame() const;

    UFUNCTION(BlueprintPure, Category = "Platform|Friends")
    bool IsFriend(const FString& UserId) const;

    UFUNCTION(BlueprintCallable, Category = "Platform|Friends")
    void RefreshFriendsList();

    UFUNCTION(BlueprintCallable, Category = "Platform|Friends")
    void ShowFriendsUI();

    // ===== Overlay =====

    UFUNCTION(BlueprintPure, Category = "Platform|Overlay")
    bool IsOverlayActive() const;

    UFUNCTION(BlueprintCallable, Category = "Platform|Overlay")
    void ShowPlatformOverlay();

    UFUNCTION(BlueprintCallable, Category = "Platform|Overlay")
    void ShowStorePageOverlay();

    UFUNCTION(BlueprintCallable, Category = "Platform|Overlay")
    void ShowUserProfileOverlay(const FString& UserId);

    // ===== Screenshots/Video =====

    UFUNCTION(BlueprintCallable, Category = "Platform|Capture")
    void TriggerScreenshot();

    UFUNCTION(BlueprintCallable, Category = "Platform|Capture")
    void StartVideoCapture();

    UFUNCTION(BlueprintCallable, Category = "Platform|Capture")
    void StopVideoCapture();

    UFUNCTION(BlueprintPure, Category = "Platform|Capture")
    bool IsVideoCapturing() const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnPlatformInitialized OnPlatformInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnUserLoggedIn OnUserLoggedIn;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnUserLoggedOut OnUserLoggedOut;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnAchievementUnlocked OnAchievementUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnAchievementProgressUpdated OnAchievementProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnEntitlementUpdated OnEntitlementUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnPresenceUpdated OnPresenceUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnFriendPresenceChanged OnFriendPresenceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnOverlayActivated OnOverlayActivated;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnOverlayDeactivated OnOverlayDeactivated;

    UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
    FMGOnPlatformError OnPlatformError;

protected:
    void DetectPlatform();
    void InitializePlatformServices();
    void InitializeDefaultAchievements();
    void SyncAchievementsWithPlatform();

private:
    // Platform info
    UPROPERTY()
    EMGPlatformType CurrentPlatform;

    UPROPERTY()
    FMGPlatformCapabilities Capabilities;

    // User data
    UPROPERTY()
    FMGPlatformUser CurrentUser;

    bool bUserLoggedIn;

    // Achievements
    UPROPERTY()
    TMap<FName, FMGPlatformAchievement> Achievements;

    // Entitlements
    UPROPERTY()
    TMap<FName, FMGEntitlement> Entitlements;

    // Stats
    UPROPERTY()
    TMap<FName, FMGPlatformStats> PlatformStats;

    // Friends
    UPROPERTY()
    TArray<FMGPlatformUser> FriendsList;

    // Presence
    UPROPERTY()
    FMGRichPresence CurrentPresence;

    // State
    bool bOverlayActive;
    bool bVideoCapturing;
    bool bPlatformInitialized;
};
