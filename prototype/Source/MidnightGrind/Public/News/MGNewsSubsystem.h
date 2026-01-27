// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGNewsSubsystem.h
 * @brief News and Announcements Subsystem for Midnight Grind
 *
 * This subsystem manages in-game news, announcements, patch notes, and
 * maintenance notifications. It keeps players informed about game updates,
 * events, and community content directly within the game client.
 *
 * ## Key Features
 *
 * - **News Articles**: Dynamic content including event announcements,
 *   community highlights, and esports updates.
 *
 * - **Patch Notes**: Structured patch note display with categorized
 *   changes (features, improvements, bug fixes, balance changes).
 *
 * - **Priority System**: Critical news (maintenance, urgent fixes) can
 *   interrupt gameplay to ensure players see important information.
 *
 * - **Read Tracking**: Tracks which articles players have read and
 *   provides unread counts for notification badges.
 *
 * - **Maintenance Alerts**: Countdown timers for scheduled maintenance
 *   windows to prevent players from losing progress.
 *
 * ## Content Categories
 *
 * - **PatchNotes**: Game version updates and changes
 * - **Event**: Limited-time events and special occasions
 * - **Community**: Player spotlights, fan art, content creator features
 * - **Esports**: Tournament announcements and competitive news
 * - **Maintenance**: Scheduled downtime notifications
 * - **Feature**: New feature announcements and tutorials
 * - **Season**: Season pass updates and rewards reveals
 *
 * @note News is fetched from the server periodically and cached locally.
 *       Read status is persisted across sessions.
 *
 * @see FMGNewsArticle for article structure
 * @see FMGPatchNote for patch note structure
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNewsSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Categories for news articles
 *
 * Used to organize and filter news content. Each category has its own
 * visual styling in the news UI and can be filtered independently.
 */
UENUM(BlueprintType)
enum class EMGNewsCategory : uint8
{
	PatchNotes,   ///< Game version updates with detailed change lists
	Event,        ///< Limited-time events, seasonal content, special occasions
	Community,    ///< Player spotlights, fan art features, community highlights
	Esports,      ///< Tournament announcements, competitive scene updates
	Maintenance,  ///< Scheduled server downtime and maintenance windows
	Feature,      ///< New feature announcements and how-to guides
	Season        ///< Season pass updates, reward reveals, progression info
};

/**
 * @brief Priority levels for news articles
 *
 * Determines how prominently articles are displayed and whether they
 * can interrupt normal gameplay flow.
 */
UENUM(BlueprintType)
enum class EMGNewsPriority : uint8
{
	Low,      ///< Background content, shown only when player visits news section
	Normal,   ///< Standard news, notification badge but no interruption
	High,     ///< Important news, shown on main menu with highlight
	Critical  ///< Urgent news (maintenance, security), interrupts to show immediately
};

//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief A single news article or announcement
 *
 * Contains all content and metadata for a news item including text,
 * images, timestamps, and interactive elements like action buttons.
 */
USTRUCT(BlueprintType)
struct FMGNewsArticle
{
	GENERATED_BODY()

	//-------------------------------------------------------------------------
	// Identification
	//-------------------------------------------------------------------------

	/// Unique server-assigned identifier for this article
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ArticleID;

	//-------------------------------------------------------------------------
	// Content
	//-------------------------------------------------------------------------

	/// Article headline (keep under 60 characters for display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/// Brief summary shown in article lists (1-2 sentences)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Summary;

	/// Full article content (supports basic formatting)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FullContent;

	//-------------------------------------------------------------------------
	// Classification
	//-------------------------------------------------------------------------

	/// Content category for filtering and styling
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNewsCategory Category = EMGNewsCategory::PatchNotes;

	/// Display priority and interruption behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNewsPriority Priority = EMGNewsPriority::Normal;

	//-------------------------------------------------------------------------
	// Timing
	//-------------------------------------------------------------------------

	/// When this article becomes visible to players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime PublishDate;

	/// When this article should be removed (Zero = never expires)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryDate;

	//-------------------------------------------------------------------------
	// Media and Actions
	//-------------------------------------------------------------------------

	/// URL to header/thumbnail image (empty = no image)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ImageURL;

	/// Deep link URL when action button is pressed (empty = no action)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionURL;

	/// Text for the action button (e.g., "View Event", "Open Store")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ActionButtonText;

	//-------------------------------------------------------------------------
	// Player Interaction State
	//-------------------------------------------------------------------------

	/// If true, player must acknowledge before dismissing (for critical news)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresAcknowledgement = false;

	/// True if the player has opened and read this article
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRead = false;

	/// Searchable tags for filtering (e.g., "cars", "pvp", "rewards")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Tags;
};

/**
 * @brief Structured patch notes for a game version
 *
 * Organizes patch changes into categories for easy reading. Each category
 * contains a list of individual changes as bullet points.
 */
USTRUCT(BlueprintType)
struct FMGPatchNote
{
	GENERATED_BODY()

	/// Version string (e.g., "1.2.0", "Season 3 Update")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Version;

	/// When this version was released
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReleaseDate;

	/// New features and content added in this version
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> NewFeatures;

	/// Enhancements to existing features
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Improvements;

	/// Bugs that have been fixed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> BugFixes;

	/// Gameplay balance adjustments (car stats, economy, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> BalanceChanges;

	/// Issues the team is aware of but haven't fixed yet
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> KnownIssues;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when a new article is received from the server
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNewArticleReceived, const FMGNewsArticle&, Article);

/// Broadcast when a critical priority article arrives (maintenance, security)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCriticalNewsReceived, const FMGNewsArticle&, Article);

/// Broadcast when the news feed has been refreshed from the server
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnNewsRefreshed);

//=============================================================================
// News Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing news and announcements
 *
 * Provides APIs for fetching, displaying, and tracking news content.
 * Automatically refreshes from the server periodically and tracks which
 * articles players have read.
 *
 * ## Usage Example (Blueprint)
 * @code
 * // Get unread articles for notification badge
 * int32 UnreadCount = NewsSubsystem->GetUnreadCount();
 *
 * // Display news by category
 * TArray<FMGNewsArticle> EventNews = NewsSubsystem->GetArticlesByCategory(EMGNewsCategory::Event);
 *
 * // Mark article as read when player opens it
 * NewsSubsystem->MarkAsRead(ArticleID);
 * @endcode
 *
 * ## Usage Example (C++)
 * @code
 * if (UMGNewsSubsystem* News = GetGameInstance()->GetSubsystem<UMGNewsSubsystem>())
 * {
 *     // Check for upcoming maintenance
 *     if (News->IsMaintenanceScheduled())
 *     {
 *         FTimespan TimeLeft = News->GetTimeUntilMaintenance();
 *         ShowMaintenanceWarning(TimeLeft);
 *     }
 * }
 * @endcode
 *
 * @note Bind to OnCriticalNewsReceived to handle urgent notifications that
 *       should interrupt gameplay (like imminent maintenance).
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNewsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/// Called when game instance creates this subsystem; starts news refresh timer
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when game instance shuts down; saves read status
	virtual void Deinitialize() override;

	//=========================================================================
	// News Retrieval
	//=========================================================================

	/**
	 * @brief Forces an immediate refresh of news from the server
	 * @note Also triggered automatically on a timer
	 */
	UFUNCTION(BlueprintCallable, Category = "News")
	void RefreshNews();

	/**
	 * @brief Returns all cached news articles
	 * @return Array of all articles, sorted by publish date (newest first)
	 */
	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetAllArticles() const { return Articles; }

	/**
	 * @brief Returns articles filtered by category
	 * @param Category The category to filter by
	 * @return Array of matching articles
	 */
	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetArticlesByCategory(EMGNewsCategory Category) const;

	/**
	 * @brief Returns only unread articles
	 * @return Array of articles the player hasn't opened yet
	 */
	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetUnreadArticles() const;

	/**
	 * @brief Returns the count of unread articles
	 * @return Number of unread articles (for notification badges)
	 */
	UFUNCTION(BlueprintPure, Category = "News")
	int32 GetUnreadCount() const;

	/**
	 * @brief Retrieves a specific article by ID
	 * @param ArticleID The unique article identifier
	 * @return The article if found, empty struct otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "News")
	FMGNewsArticle GetArticle(const FString& ArticleID) const;

	//=========================================================================
	// Article Interaction
	//=========================================================================

	/**
	 * @brief Marks an article as read
	 * @param ArticleID The article to mark
	 */
	UFUNCTION(BlueprintCallable, Category = "News")
	void MarkAsRead(const FString& ArticleID);

	/**
	 * @brief Marks all articles as read
	 * @note Useful for "Mark All Read" button in news UI
	 */
	UFUNCTION(BlueprintCallable, Category = "News")
	void MarkAllAsRead();

	/**
	 * @brief Acknowledges a critical article (required before dismissing)
	 * @param ArticleID The article to acknowledge
	 * @note Only needed for articles with bRequiresAcknowledgement = true
	 */
	UFUNCTION(BlueprintCallable, Category = "News")
	void AcknowledgeArticle(const FString& ArticleID);

	//=========================================================================
	// Patch Notes
	//=========================================================================

	/**
	 * @brief Returns patch notes for the current game version
	 * @return Structured patch note data
	 */
	UFUNCTION(BlueprintPure, Category = "News|Patch")
	FMGPatchNote GetCurrentPatchNotes() const { return CurrentPatch; }

	/**
	 * @brief Returns patch notes for previous versions
	 * @return Array of historical patch notes
	 */
	UFUNCTION(BlueprintPure, Category = "News|Patch")
	TArray<FMGPatchNote> GetPatchHistory() const { return PatchHistory; }

	/**
	 * @brief Checks if there are patch notes the player hasn't seen
	 * @return True if current version notes haven't been viewed
	 */
	UFUNCTION(BlueprintPure, Category = "News|Patch")
	bool HasUnseenPatchNotes() const;

	/**
	 * @brief Marks current patch notes as seen
	 * @note Call when player opens the patch notes UI
	 */
	UFUNCTION(BlueprintCallable, Category = "News|Patch")
	void MarkPatchNotesAsSeen();

	//=========================================================================
	// Maintenance Information
	//=========================================================================

	/**
	 * @brief Checks if maintenance is scheduled
	 * @return True if there's upcoming maintenance
	 */
	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	bool IsMaintenanceScheduled() const;

	/**
	 * @brief Returns the start time of the next maintenance window
	 * @return DateTime of maintenance start (check IsMaintenanceScheduled first)
	 */
	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	FDateTime GetNextMaintenanceTime() const;

	/**
	 * @brief Returns time remaining until maintenance begins
	 * @return Timespan until maintenance (negative if maintenance is in progress)
	 */
	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	FTimespan GetTimeUntilMaintenance() const;

	//=========================================================================
	// Events
	//=========================================================================

	/// Broadcast when a new article is received
	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnNewArticleReceived OnNewArticleReceived;

	/// Broadcast when a critical article requires immediate attention
	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnCriticalNewsReceived OnCriticalNewsReceived;

	/// Broadcast when news list is refreshed from server
	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnNewsRefreshed OnNewsRefreshed;

protected:
	//-------------------------------------------------------------------------
	// Internal Methods
	//-------------------------------------------------------------------------

	/// Initiates async news fetch from server
	void FetchNewsFromServer();

	/// Processes newly received articles, checking for new/critical items
	void ProcessNewArticles(const TArray<FMGNewsArticle>& NewArticles);

	/// Loads read/acknowledged status from local storage
	void LoadReadStatus();

	/// Saves read/acknowledged status to local storage
	void SaveReadStatus();

private:
	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// All cached news articles
	UPROPERTY()
	TArray<FMGNewsArticle> Articles;

	/// Current version's patch notes
	UPROPERTY()
	FMGPatchNote CurrentPatch;

	/// Historical patch notes for previous versions
	UPROPERTY()
	TArray<FMGPatchNote> PatchHistory;

	/// Set of article IDs the player has read
	TSet<FString> ReadArticleIDs;

	/// Set of article IDs the player has acknowledged
	TSet<FString> AcknowledgedArticleIDs;

	/// Version string of last patch notes the player viewed
	FString LastSeenPatchVersion;

	/// Start time of next scheduled maintenance window
	FDateTime ScheduledMaintenanceTime;

	/// Timer handle for periodic news refresh
	FTimerHandle RefreshTimerHandle;
};
