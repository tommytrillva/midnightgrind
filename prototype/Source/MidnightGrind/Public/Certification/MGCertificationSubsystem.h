// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGCertificationSubsystem.h
 * Platform Certification Compliance System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Certification Subsystem, which ensures the game meets
 * the strict requirements set by platform holders (Sony, Microsoft, Nintendo,
 * Steam, Epic) for publishing games on their platforms. Without passing
 * certification, a game cannot be released on consoles or storefronts.
 *
 * WHY CERTIFICATION MATTERS:
 * --------------------------
 * When you submit a game to PlayStation, Xbox, or Nintendo, they test it
 * against hundreds of requirements called "TRCs" (Technical Requirements
 * Checklist) or "XRs" (Xbox Requirements). Failing even one can delay
 * your game's release by weeks. This subsystem helps ensure compliance.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PLATFORM-SPECIFIC REQUIREMENTS:
 *    Each platform has unique rules. For example:
 *    - PlayStation: Must handle suspend/resume correctly, show specific icons
 *    - Xbox: Must support Quick Resume, handle controller disconnect properly
 *    - Nintendo Switch: Must handle docked/undocked mode transitions
 *    - Steam/Epic: Must handle overlay properly, support cloud saves
 *
 * 2. AGE RATINGS:
 *    Games must be rated by regional boards before sale:
 *    - ESRB (North America): E, E10+, T, M, AO
 *    - PEGI (Europe): 3, 7, 12, 16, 18
 *    - CERO (Japan): A, B, C, D, Z
 *    - USK (Germany): 0, 6, 12, 16, 18
 *    This affects what content can be shown in each region.
 *
 * 3. CONTENT DESCRIPTORS:
 *    Describe what's in your game (violence, language, online features).
 *    Must be displayed accurately to match your age rating.
 *
 * 4. SUSPEND/RESUME:
 *    Console games must handle being paused by the system (e.g., player
 *    presses PS button, system goes to sleep). The game must:
 *    - Save state properly
 *    - Pause any timers or gameplay
 *    - Resume correctly without losing progress
 *    - Handle network reconnection if needed
 *
 * 5. CONTROLLER DISCONNECT:
 *    If a controller disconnects mid-game, the game must:
 *    - Pause immediately
 *    - Show a clear message to reconnect
 *    - Resume when reconnected (not restart)
 *
 * 6. NETWORK STATUS:
 *    Games must gracefully handle losing internet connection:
 *    - Show appropriate error messages
 *    - Save offline progress
 *    - Not crash or freeze
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Platform OS Events]
 *          |
 *          v
 *   [Unreal Engine Platform Layer]
 *          |
 *          v
 *   [MGCertificationSubsystem] -- Detects platform --> [Platform-specific behavior]
 *          |
 *          +-- OnApplicationSuspending() --> Pause game, save state
 *          +-- OnControllerDisconnected() --> Show reconnect UI
 *          +-- OnNetworkStatusChanged() --> Handle online/offline
 *          +-- OnUserSignedOut() --> Return to title screen
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Game starts, DetectPlatform() identifies we're on PlayStation 5
 * 2. InitializeRequirements() loads PS5-specific certification requirements
 * 3. Throughout gameplay, the subsystem monitors for platform events
 * 4. When player presses PS button, OnApplicationSuspending() is called
 * 5. Game pauses, saves checkpoint, pauses network activity
 * 6. When player returns, OnApplicationResuming() restores state
 *
 * COMMON CERTIFICATION FAILURES TO AVOID:
 * ---------------------------------------
 * - Not pausing when controller disconnects
 * - Crashing when network drops
 * - Not saving when suspended
 * - Incorrect age rating display
 * - Not handling user sign-out properly
 * - Missing required platform-specific UI elements
 *
 * @see EMGPlatform - Supported platforms
 * @see EMGAgeRating - Regional age rating systems
 * @see FMGCertificationRequirement - Individual certification requirements
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCertificationSubsystem.generated.h"

// =============================================================================
// ENUMERATIONS
// =============================================================================

/**
 * Supported gaming platforms.
 *
 * Each platform has different certification requirements and capabilities.
 * The subsystem detects which platform we're running on at startup and
 * adjusts behavior accordingly.
 */

UENUM(BlueprintType)
enum class EMGPlatform : uint8
{
	PC,				///< Generic Windows/Mac/Linux PC build
	PlayStation5,	///< Sony PlayStation 5 console
	XboxSeriesX,	///< Microsoft Xbox Series X|S consoles
	NintendoSwitch,	///< Nintendo Switch (docked and handheld)
	Steam,			///< PC distributed via Steam (has overlay requirements)
	EpicGames		///< PC distributed via Epic Games Store
};

/**
 * Age ratings from various regional rating boards.
 *
 * Games must display the appropriate rating for each region. Using the
 * wrong rating or selling to underage players can result in legal issues
 * and removal from storefronts.
 *
 * REGIONAL BOARDS:
 * - ESRB: Entertainment Software Rating Board (North America)
 * - PEGI: Pan European Game Information (Europe)
 * - CERO: Computer Entertainment Rating Organization (Japan)
 * - USK: Unterhaltungssoftware Selbstkontrolle (Germany)
 */
UENUM(BlueprintType)
enum class EMGAgeRating : uint8
{
	// ESRB (North America) - Required for US, Canada, Mexico
	ESRB_Everyone,		///< E - Suitable for all ages
	ESRB_Everyone10,	///< E10+ - Suitable for ages 10 and up
	ESRB_Teen,			///< T - Suitable for ages 13 and up
	ESRB_Mature,		///< M - Suitable for ages 17 and up

	// PEGI (Europe) - Required for EU countries
	PEGI_3,				///< Suitable for all ages
	PEGI_7,				///< Suitable for ages 7 and up
	PEGI_12,			///< Suitable for ages 12 and up
	PEGI_16,			///< Suitable for ages 16 and up
	PEGI_18,			///< Adults only

	// CERO (Japan) - Required for Japanese release
	CERO_A,				///< All ages
	CERO_B,				///< Ages 12 and up
	CERO_C,				///< Ages 15 and up

	// USK (Germany) - Required for German release
	USK_0,				///< No age restriction
	USK_6,				///< Ages 6 and up
	USK_12,				///< Ages 12 and up
	USK_16,				///< Ages 16 and up
	USK_18				///< Ages 18 and up (no sale to minors)
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * A single certification requirement from a platform holder.
 *
 * Each platform has dozens to hundreds of requirements. This structure
 * tracks whether each one has been met. During development, you can use
 * these to identify areas that need attention before submission.
 *
 * EXAMPLE REQUIREMENTS:
 * - PS5: "Must pause within 10 seconds of suspend"
 * - Xbox: "Must support Quick Resume without losing progress"
 * - Switch: "Must function in both docked and handheld modes"
 * - All: "Must display correct age rating on boot"
 */
USTRUCT(BlueprintType)
struct FMGCertificationRequirement
{
	GENERATED_BODY()

	/// Unique identifier matching platform documentation (e.g., "PS5_TRC_R4000")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequirementID;

	/// Human-readable description of what the requirement entails
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Which platform this requirement applies to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatform Platform = EMGPlatform::PC;

	/// Whether this requirement has been validated as passing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMet = false;

	/// If not met, explains why (helps debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FailureReason;
};

/**
 * Content descriptors that appear with the age rating.
 *
 * These must accurately reflect the game's content. Misrepresenting content
 * can result in rating changes, fines, or removal from sale. For example,
 * if the game has in-app purchases, bInGamePurchases MUST be true.
 *
 * COMMON DESCRIPTORS:
 * - Violence: Car crashes, combat, property destruction
 * - Mild Language: Minor swear words, insults
 * - Online Interaction: Can encounter other players online
 * - In-Game Purchases: Can spend real money in the game
 * - User Generated Content: Players can create/share content
 */
USTRUCT(BlueprintType)
struct FMGContentDescriptor
{
	GENERATED_BODY()

	/// Does the game contain violence? (crashes, combat, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bViolence = false;

	/// Does the game contain mild language? (minor swearing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMildLanguage = false;

	/// Can players interact with others online?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlineInteraction = true;

	/// Can players spend real money in the game?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInGamePurchases = true;

	/// Can players create or share content (liveries, tracks, etc.)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUserGeneratedContent = true;
};

// =============================================================================
// CERTIFICATION SUBSYSTEM CLASS
// =============================================================================

/**
 * Manages platform certification requirements and system events.
 *
 * This subsystem helps ensure the game meets all platform holder requirements
 * for certification. It handles platform detection, tracks requirement compliance,
 * and provides hooks for system events that must be handled correctly.
 *
 * USAGE EXAMPLE:
 * @code
 * // Get the subsystem
 * UMGCertificationSubsystem* Cert = GameInstance->GetSubsystem<UMGCertificationSubsystem>();
 *
 * // Check what platform we're on
 * if (Cert->IsPlatform(EMGPlatform::PlayStation5))
 * {
 *     // Enable PS5-specific features like DualSense haptics
 * }
 *
 * // Check if all certification requirements are met (useful for QA)
 * if (!Cert->AreAllRequirementsMet())
 * {
 *     // Show warning in development builds
 *     for (auto& Req : Cert->GetRequirements())
 *     {
 *         if (!Req.bIsMet)
 *         {
 *             UE_LOG(LogGame, Warning, TEXT("Cert requirement not met: %s"), *Req.Description.ToString());
 *         }
 *     }
 * }
 * @endcode
 *
 * SYSTEM EVENT HANDLING:
 * The game must call these functions when system events occur:
 * - OnApplicationSuspending(): When game is being suspended (PS button, etc.)
 * - OnApplicationResuming(): When game resumes from suspension
 * - OnControllerDisconnected(): When a controller loses connection
 * - OnControllerReconnected(): When a controller reconnects
 * - OnNetworkStatusChanged(): When network connectivity changes
 * - OnUserSignedOut(): When the user signs out of their platform account
 *
 * These are typically wired up in your Game Instance or a dedicated handler class.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCertificationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called automatically when the game instance is created
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ==========================================
	// PLATFORM DETECTION
	// ==========================================
	// Use these to implement platform-specific behavior.

	/**
	 * Gets the platform the game is currently running on.
	 * @return Current platform enum value
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	EMGPlatform GetCurrentPlatform() const { return CurrentPlatform; }

	/**
	 * Checks if we're running on a specific platform.
	 * @param Platform The platform to check against
	 * @return True if we're on that platform
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsPlatform(EMGPlatform Platform) const { return CurrentPlatform == Platform; }

	// ==========================================
	// AGE RATING
	// ==========================================
	// Display the correct rating based on region.

	/**
	 * Gets the age rating for the current region.
	 * Display this on the title screen as required by certification.
	 * @return Current age rating
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	EMGAgeRating GetAgeRating() const { return CurrentAgeRating; }

	/**
	 * Gets the content descriptors (violence, online interaction, etc.).
	 * Display these alongside the age rating.
	 * @return Content descriptor flags
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	FMGContentDescriptor GetContentDescriptors() const { return ContentDescriptors; }

	// ==========================================
	// REQUIREMENT TRACKING
	// ==========================================
	// Used during development to track certification compliance.

	/**
	 * Gets all certification requirements for the current platform.
	 * Useful for generating compliance reports during QA.
	 * @return Array of all requirements
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	TArray<FMGCertificationRequirement> GetRequirements() const { return Requirements; }

	/**
	 * Manually validates a specific requirement (marks it as tested).
	 * @param RequirementID The ID of the requirement to validate
	 * @return True if the requirement passed validation
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	bool ValidateRequirement(FName RequirementID);

	/**
	 * Checks if all certification requirements for this platform are met.
	 * @return True if all requirements pass, false if any fail
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	bool AreAllRequirementsMet() const;

	// ==========================================
	// SUSPEND/RESUME (Console Requirement)
	// ==========================================
	// Consoles require games to handle being suspended gracefully.
	// Failure to implement these correctly will fail certification.

	/**
	 * Called when the application is being suspended.
	 *
	 * MUST be called when:
	 * - PS5: Player presses PS button to go to home screen
	 * - Xbox: Player presses Xbox button or system enters Quick Resume
	 * - Switch: Player presses Home button or closes the lid
	 *
	 * WHAT THIS SHOULD DO:
	 * - Pause all gameplay immediately
	 * - Save any unsaved progress
	 * - Pause network activity and timers
	 * - Prepare for potential termination (save state)
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnApplicationSuspending();

	/**
	 * Called when the application resumes from suspension.
	 *
	 * WHAT THIS SHOULD DO:
	 * - Check network connectivity (may have changed)
	 * - Verify user is still signed in
	 * - Refresh any time-sensitive data
	 * - Resume gameplay only after user input
	 * - Check for game updates if connected
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnApplicationResuming();

	// ==========================================
	// NETWORK STATE (Console Requirement)
	// ==========================================
	// Games must gracefully handle network changes.

	/**
	 * Called when network connectivity status changes.
	 *
	 * WHAT THIS SHOULD DO:
	 * - If offline: Show appropriate message, disable online features
	 * - If online: Attempt to reconnect services
	 *
	 * @param bIsOnline True if network is now available, false if lost
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnNetworkStatusChanged(bool bIsOnline);

	/**
	 * Checks current network availability.
	 * @return True if network is available
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsNetworkAvailable() const { return bNetworkAvailable; }

	// ==========================================
	// CONTROLLER (Console Requirement)
	// ==========================================
	// Games MUST pause and show a message when controllers disconnect.

	/**
	 * Called when a controller is disconnected.
	 *
	 * CERTIFICATION REQUIREMENT:
	 * - MUST pause gameplay immediately (within 10 seconds on PS5)
	 * - MUST show a clear message asking to reconnect
	 * - MUST NOT continue gameplay without input
	 *
	 * @param ControllerID Which controller was disconnected
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnControllerDisconnected(int32 ControllerID);

	/**
	 * Called when a controller is reconnected.
	 *
	 * WHAT THIS SHOULD DO:
	 * - Dismiss the reconnection message
	 * - Allow gameplay to resume (but don't auto-resume during races)
	 *
	 * @param ControllerID Which controller was reconnected
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnControllerReconnected(int32 ControllerID);

	// ==========================================
	// USER SIGN-IN (Console Requirement)
	// ==========================================
	// If the user signs out, the game must handle it appropriately.

	/**
	 * Called when the user signs out of their platform account.
	 *
	 * CERTIFICATION REQUIREMENT:
	 * - MUST stop any online activity immediately
	 * - MUST return to title screen or show sign-in prompt
	 * - MUST NOT continue as if signed in
	 *
	 * This can happen if another user signs in on the same console.
	 */
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnUserSignedOut();

	/**
	 * Checks if a user is currently signed in.
	 * @return True if user is signed in
	 */
	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsUserSignedIn() const { return bUserSignedIn; }

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================

	/** Detects which platform we're running on at startup */
	void DetectPlatform();

	/** Loads certification requirements for the current platform */
	void InitializeRequirements();

private:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Which platform we detected at startup */
	EMGPlatform CurrentPlatform = EMGPlatform::PC;

	/** Age rating for the current region */
	EMGAgeRating CurrentAgeRating = EMGAgeRating::ESRB_Everyone;

	/** Content descriptor flags */
	FMGContentDescriptor ContentDescriptors;

	/** All certification requirements for this platform */
	TArray<FMGCertificationRequirement> Requirements;

	/** Current network connectivity status */
	bool bNetworkAvailable = true;

	/** Whether a user is currently signed in */
	bool bUserSignedIn = true;
};
