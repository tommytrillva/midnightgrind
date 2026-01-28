// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGCommunityHighlightsSubsystem.h - Community Content Showcase System
 * =============================================================================
 *
 * OVERVIEW
 * --------
 * This file defines the community highlights system that showcases exceptional
 * player-created content and celebrates outstanding community members. It powers
 * the in-game "Community" hub where players discover featured liveries, tracks,
 * clips, and creators.
 *
 * Think of it like a curated social media feed inside the game, highlighting
 * the best content the community has created.
 *
 * KEY CONCEPTS FOR BEGINNERS
 * --------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    This inherits from UGameInstanceSubsystem:
 *    - One instance persists for the entire game session
 *    - Survives level transitions (community hub is always accessible)
 *    - Access via: GetGameInstance()->GetSubsystem<UMGCommunityHighlightsSubsystem>()
 *
 * 2. HIGHLIGHT TYPES (EMGHighlightType)
 *    Different categories of featured content:
 *    - FeaturedLivery: Custom vehicle paint jobs/wraps
 *    - FeaturedTrack: User-created race courses
 *    - TopRacer: Competitive leaderboard champions
 *    - ClipOfTheDay: Outstanding gameplay videos
 *    - PhotoOfTheDay: Exceptional in-game photography
 *    - CrewSpotlight: Featured racing teams/clans
 *    - RisingTalent: Promising new players
 *    - LegendStatus: All-time great recognition
 *
 * 3. HIGHLIGHT DATA (FMGCommunityHighlight)
 *    Each featured item contains:
 *    - HighlightID: Unique identifier from the backend
 *    - Title/Description: Display text
 *    - CreatorPlayerID/CreatorName: Who made it
 *    - ContentID: Reference to the actual content (livery ID, etc.)
 *    - ThumbnailURL/MediaURL: Images/videos for display
 *    - LikeCount/DownloadCount: Community engagement metrics
 *    - bIsLikedByPlayer: Has the current player liked this?
 *
 * 4. CREATOR PROFILES (FMGCreatorProfile)
 *    Aggregated stats for content creators:
 *    - TotalCreations: How many things they've shared
 *    - TotalDownloads/TotalLikes: Cumulative engagement
 *    - FeatureCount: Times their content was featured
 *    - bIsVerifiedCreator: Official creator badge (checkmark)
 *
 * 5. ASYNC DATA FETCHING
 *    Highlights come from a backend server, not stored locally:
 *    - RefreshHighlights(): Requests fresh data from server
 *    - OnHighlightsFetched: Delegate fires when data arrives
 *    - GetCurrentHighlights(): Returns cached data (may be stale)
 *
 *    Pattern for UI code:
 *      Subsystem->OnHighlightsFetched.AddDynamic(this, &MyWidget::OnDataReceived);
 *      Subsystem->RefreshHighlights();
 *      // ... wait for OnDataReceived to be called ...
 *
 * 6. PLAYER INTERACTIONS
 *    How players engage with featured content:
 *    - LikeHighlight(ID): Upvote content
 *    - UnlikeHighlight(ID): Remove upvote
 *    - DownloadContent(ID): Get the livery/track/etc.
 *    - FollowCreator(PlayerID): Get notifications about their content
 *
 * 7. CONTENT SUBMISSION
 *    Players can submit their own content for featuring:
 *    - SubmitForFeature(ContentID, Type): Request review
 *    - HasPendingSubmission(): Check if awaiting review
 *
 *    Submissions are reviewed by the community team before featuring.
 *    Players are notified via OnPlayerFeatured if selected.
 *
 * COMMON USAGE PATTERNS
 * ---------------------
 *
 * Loading highlights for display:
 *   // In widget initialization
 *   CommunitySubsystem->OnHighlightsFetched.AddDynamic(this, &UMyWidget::PopulateList);
 *   CommunitySubsystem->RefreshHighlights();
 *
 *   void UMyWidget::PopulateList(const TArray<FMGCommunityHighlight>& Highlights)
 *   {
 *       for (const FMGCommunityHighlight& Highlight : Highlights)
 *       {
 *           CreateHighlightCard(Highlight);
 *       }
 *   }
 *
 * Filtering by type:
 *   TArray<FMGCommunityHighlight> Liveries =
 *       CommunitySubsystem->GetHighlightsByType(EMGHighlightType::FeaturedLivery);
 *
 * Liking content:
 *   void UMyWidget::OnLikeButtonClicked(FString HighlightID)
 *   {
 *       CommunitySubsystem->LikeHighlight(HighlightID);
 *       // Optimistically update UI
 *   }
 *
 * Downloading featured content:
 *   void UMyWidget::OnDownloadClicked(FString HighlightID)
 *   {
 *       CommunitySubsystem->DownloadContent(HighlightID);
 *       // This triggers the appropriate download flow based on content type
 *   }
 *
 * Following a creator:
 *   CommunitySubsystem->FollowCreator(CreatorPlayerID);
 *   // Player will get notifications when creator uploads new content
 *
 * CELEBRATION MOMENT
 * ------------------
 * When a player's content gets featured, OnPlayerFeatured fires.
 * This is a great opportunity to show a celebratory notification/animation!
 *
 * @see UMGClipSubsystem for player clip recording
 * @see UMGLiverySubsystem for livery creation and sharing
 */

/**
 * @file MGCommunityHighlightsSubsystem.h
 * @brief Community Highlights and Featured Content Subsystem for Midnight Grind
 *
 * This subsystem manages the community showcase features that surface exceptional
 * player-created content and celebrate outstanding community members. It powers
 * the in-game "Community" section where players can discover featured liveries,
 * tracks, clips, and creators.
 *
 * Key Features:
 * - Daily/weekly featured content rotation (liveries, tracks, clips, photos)
 * - Creator spotlight profiles and verification system
 * - Top racer recognition and leaderboard integration
 * - Community voting (likes) and download tracking
 * - Content submission pipeline for feature consideration
 * - Creator following system for notifications
 *
 * Usage:
 * Access via UGameInstance::GetSubsystem<UMGCommunityHighlightsSubsystem>().
 * The subsystem fetches highlights from the backend server and caches them locally.
 * UI widgets bind to OnHighlightsFetched to display content when available.
 *
 * @see UMGClipSubsystem for player clip recording
 * @see EMGHighlightType for content categories
 * @see FMGCommunityHighlight for highlight data structure
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCommunityHighlightsSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Types of content that can be featured as community highlights
 *
 * Each type represents a different category of community showcase content,
 * displayed in different sections of the community hub UI.
 */
UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
	FeaturedLivery,     ///< Custom vehicle livery/paint job
	FeaturedTrack,      ///< User-created race track
	TopRacer,           ///< High-performing competitive player
	ClipOfTheDay,       ///< Outstanding gameplay clip/video
	PhotoOfTheDay,      ///< Exceptional in-game photography
	CrewSpotlight,      ///< Featured racing crew/team
	RisingTalent,       ///< New player showing exceptional skill
	LegendStatus        ///< All-time great player recognition
};

//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief Data for a single community highlight entry
 *
 * Contains all information needed to display a featured content item,
 * including creator info, media assets, and engagement metrics.
 */
USTRUCT(BlueprintType)
struct FMGCommunityHighlight
{
	GENERATED_BODY()

	/// Unique identifier for this highlight (from backend)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HighlightID;

	/// Category of this highlight
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHighlightType Type = EMGHighlightType::FeaturedLivery;

	/// Display title (e.g., "Neon Dreams Livery", "Mountain Circuit")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/// Brief description of the highlighted content
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Unique ID of the creator player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CreatorPlayerID;

	/// Display name of the creator
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CreatorName;

	/// Reference ID for the actual content (livery ID, track ID, clip ID, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContentID;

	/// URL to thumbnail image for preview display
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ThumbnailURL;

	/// URL to full media (video URL for clips, full-res image for photos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MediaURL;

	/// Date when this content was featured
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FeaturedDate;

	/// Number of likes/upvotes from community
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LikeCount = 0;

	/// Number of times content has been downloaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DownloadCount = 0;

	/// Has the local player liked this highlight?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLikedByPlayer = false;
};

/**
 * @brief Profile information for a content creator
 *
 * Aggregates statistics and status for players who create and share
 * content with the community.
 */
USTRUCT(BlueprintType)
struct FMGCreatorProfile
{
	GENERATED_BODY()

	/// Unique player identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	/// Total number of creations shared (liveries, tracks, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCreations = 0;

	/// Total downloads across all creations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDownloads = 0;

	/// Total likes across all creations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLikes = 0;

	/// Number of times this creator has been featured
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureCount = 0;

	/// Is this a verified/official creator? (checkmark badge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVerifiedCreator = false;
};

//=============================================================================
// Delegates
//=============================================================================

/// Fires when highlights are fetched from server (success or cached data)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHighlightsFetched, const TArray<FMGCommunityHighlight>&, Highlights);

/// Fires when the local player's content is featured (celebration moment!)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerFeatured, const FMGCommunityHighlight&, Highlight);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem for community highlights and featured content
 *
 * UMGCommunityHighlightsSubsystem connects to the backend service to fetch
 * and display community-curated content. It handles:
 * - Fetching and caching featured content
 * - Like/unlike interactions
 * - Content download tracking
 * - Feature submission requests
 * - Creator profile lookups
 * - Creator following for notifications
 *
 * The subsystem automatically refreshes highlights periodically and notifies
 * players when their content is featured.
 *
 * Example usage:
 * @code
 * UMGCommunityHighlightsSubsystem* CommunitySys = GetGameInstance()->GetSubsystem<UMGCommunityHighlightsSubsystem>();
 *
 * // Bind to receive highlight updates
 * CommunitySys->OnHighlightsFetched.AddDynamic(this, &UMyWidget::HandleHighlightsReceived);
 *
 * // Request fresh data
 * CommunitySys->RefreshHighlights();
 *
 * // Like a highlight
 * CommunitySys->LikeHighlight(SelectedHighlight.HighlightID);
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCommunityHighlightsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Subsystem Lifecycle
	//-------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//-------------------------------------------------------------------------
	// Highlights - Fetch and query featured content
	//-------------------------------------------------------------------------

	/**
	 * @brief Request fresh highlights from the server
	 *
	 * Initiates an async fetch of current community highlights.
	 * Results are delivered via OnHighlightsFetched delegate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void RefreshHighlights();

	/**
	 * @brief Get cached highlights (may be stale)
	 * @return Array of currently cached highlights
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FMGCommunityHighlight> GetCurrentHighlights() const { return CurrentHighlights; }

	/**
	 * @brief Get highlights of a specific type
	 * @param Type The highlight category to filter by
	 * @return Array of matching highlights
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FMGCommunityHighlight> GetHighlightsByType(EMGHighlightType Type) const;

	/**
	 * @brief Get the primary "highlight of the day"
	 * @return The featured highlight for today (or most recent)
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCommunityHighlight GetHighlightOfTheDay() const;

	//-------------------------------------------------------------------------
	// Interaction - Like, download, and engage with content
	//-------------------------------------------------------------------------

	/**
	 * @brief Like/upvote a highlight
	 * @param HighlightID The highlight to like
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void LikeHighlight(const FString& HighlightID);

	/**
	 * @brief Remove like from a highlight
	 * @param HighlightID The highlight to unlike
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void UnlikeHighlight(const FString& HighlightID);

	/**
	 * @brief Download the content associated with a highlight
	 * @param HighlightID The highlight whose content to download
	 *
	 * This triggers the appropriate download flow based on content type
	 * (livery download, track download, etc.) and increments the download counter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void DownloadContent(const FString& HighlightID);

	//-------------------------------------------------------------------------
	// Submissions - Submit content for feature consideration
	//-------------------------------------------------------------------------

	/**
	 * @brief Submit content to be considered for featuring
	 * @param ContentID ID of the content to submit (livery, track, etc.)
	 * @param Type What type of highlight this would be
	 *
	 * Submissions are reviewed by the community team before being featured.
	 * Players receive notification if their submission is selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void SubmitForFeature(FName ContentID, EMGHighlightType Type);

	/**
	 * @brief Check if player has a pending submission awaiting review
	 * @return True if there is an unreviewed submission
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	bool HasPendingSubmission() const;

	//-------------------------------------------------------------------------
	// Creator Profiles - Look up and follow content creators
	//-------------------------------------------------------------------------

	/**
	 * @brief Get profile information for a creator
	 * @param PlayerID The creator's unique identifier
	 * @return Creator profile data (may be cached)
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCreatorProfile GetCreatorProfile(const FString& PlayerID) const;

	/**
	 * @brief Follow a creator to receive notifications about their content
	 * @param PlayerID The creator to follow
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void FollowCreator(const FString& PlayerID);

	/**
	 * @brief Get list of creators the player is following
	 * @return Array of followed creator player IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FString> GetFollowedCreators() const { return FollowedCreators; }

	//-------------------------------------------------------------------------
	// Delegates - Bindable events
	//-------------------------------------------------------------------------

	/// Fires when highlights are received from server
	UPROPERTY(BlueprintAssignable, Category = "Community|Events")
	FMGOnHighlightsFetched OnHighlightsFetched;

	/// Fires when the local player's content gets featured
	UPROPERTY(BlueprintAssignable, Category = "Community|Events")
	FMGOnPlayerFeatured OnPlayerFeatured;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/// Async fetch highlights from backend API
	void FetchHighlightsFromServer();

	/// Check if any of the player's content was featured and fire notification
	void CheckIfPlayerFeatured();

private:
	//-------------------------------------------------------------------------
	// Data Members
	//-------------------------------------------------------------------------

	/// Cached highlights from last server fetch
	UPROPERTY()
	TArray<FMGCommunityHighlight> CurrentHighlights;

	/// Player IDs of creators the local player follows
	TArray<FString> FollowedCreators;

	/// Set of highlight IDs the player has liked (for quick lookup)
	TSet<FString> LikedHighlights;

	/// Local player's unique identifier (for checking if featured)
	FString LocalPlayerID;
};
