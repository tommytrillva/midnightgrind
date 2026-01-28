// Copyright Midnight Grind. All Rights Reserved.

/*******************************************************************************
 * MGTrackEditorSubsystem.h - Player Track Creation Tool
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This header defines the Track Editor Subsystem - an in-game tool that lets
 * players BUILD THEIR OWN racing tracks! Think of it like:
 * - Mario Maker but for racing games
 * - Trackmania's track editor
 * - A snap-together construction kit for roads
 *
 * Players place pre-made track pieces, connect them, add decorations, then
 * share their creations with the community!
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. TRACK PIECES (FMGTrackPiece):
 *    - Pre-built road segments players snap together
 *    - Types: Straight, Curve90, Curve45, S-Curve, Hairpin, Jump, Tunnel, Bridge...
 *    - Each piece has a Transform (position + rotation) and Scale
 *    - ConnectedPieces: Links to adjacent pieces (for validation)
 *
 * 2. SNAP POINTS:
 *    - Connection points where pieces can join
 *    - GetNearestSnapPoint() finds valid attachment positions
 *    - Makes building intuitive - pieces "snap" together like LEGO
 *
 * 3. DECORATIONS (FMGTrackDecoration):
 *    - Non-road objects that add visual detail
 *    - Trees, buildings, barriers, signs, lights
 *    - Different decorations available per environment/theme
 *
 * 4. TRACK ENVIRONMENTS (EMGTrackEnvironment):
 *    - Visual themes: Downtown, Industrial, Harbor, Highway, Mountain, etc.
 *    - Affects available decorations and visual style
 *    - "Neon" = the Y2K aesthetic with glowing signs
 *
 * 5. TRACK DATA (FMGCustomTrackData):
 *    - Complete track information:
 *      - Metadata: Name, description, author, creation date
 *      - Content: All pieces and decorations
 *      - Settings: Lap count, max racers
 *      - Community: Downloads, likes, rating
 *    - Thumbnail: Preview image for browsing
 *
 * 6. EDITOR MODE:
 *    - EnterEditor(): Switch to building mode
 *    - ExitEditor(): Return to normal gameplay
 *    - IsInEditor(): Check current mode
 *    - NewTrack(): Start fresh with chosen environment
 *
 * 7. UNDO/REDO SYSTEM:
 *    - UndoStack/RedoStack store previous states
 *    - Undo(): Go back one action
 *    - Redo(): Restore undone action
 *    - PushUndoState(): Called internally when you change something
 *
 * 8. VALIDATION:
 *    - ValidateTrack(): Check if track is raceable
 *    - IsTrackClosed(): Does the track loop back to start?
 *    - HasStartFinish(): Is there a start/finish line piece?
 *    - OutErrors: Returns list of problems to fix
 *
 * 9. PUBLISHING:
 *    - SaveTrack(): Save locally
 *    - PublishTrack(): Upload to community servers
 *    - bIsPublished: Track is public
 *    - bIsFeatured: Track is highlighted by devs
 *
 * COMMUNITY FEATURES:
 * -------------------
 * - GetCommunityTracks(): Browse player-made tracks
 * - GetFeaturedTracks(): See dev-picked highlights
 * - SearchTracks(): Find tracks by name/keyword
 * - GetMyTracks(): View your created tracks
 * - DownloadTrack(): Get someone else's track
 * - RateTrack() / LikeTrack(): Give feedback
 *
 * FGuid EXPLAINED:
 * ----------------
 * - FGuid = Globally Unique Identifier (like a UUID)
 * - 128-bit number that's virtually guaranteed unique
 * - Each piece and track gets one for identification
 * - Example: "A2B4C6D8-1234-5678-9ABC-DEF012345678"
 *
 * FTransform EXPLAINED:
 * ---------------------
 * - Combines Position (FVector) + Rotation (FRotator) + Scale (FVector)
 * - Represents where something is, how it's rotated, and how big it is
 * - Used for placing pieces and decorations in the world
 *
 * HOW TO USE THIS SYSTEM:
 * -----------------------
 * BUILDING:
 *   1. EnterEditor()
 *   2. NewTrack(EMGTrackEnvironment::Downtown)
 *   3. PlacePiece(EMGTrackPieceType::StartFinish, Transform)
 *   4. PlacePiece(EMGTrackPieceType::Straight, NextTransform)
 *   5. ConnectPieces(PieceA_Id, PieceB_Id)
 *   6. Add decorations: PlaceDecoration(AssetName, Transform)
 *   7. Validate: ValidateTrack(Errors)
 *   8. Save/Publish: SaveTrack() then PublishTrack()
 *
 * EDITING:
 *   - MovePiece(): Reposition a piece
 *   - RotatePiece(): Turn a piece
 *   - ScalePiece(): Resize a piece
 *   - SetPieceBanking(): Tilt the road (for banked curves)
 *   - RemovePiece(): Delete a piece
 *   - Undo()/Redo(): Fix mistakes
 *
 * DELEGATES (EVENTS):
 * -------------------
 * - OnTrackPiecePlaced: Piece added
 * - OnTrackSaved: Track saved locally
 * - OnTrackPublished: Track uploaded to community
 * - OnTrackValidated: Validation completed
 *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTrackEditorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTrackPieceType : uint8
{
	Straight		UMETA(DisplayName = "Straight"),
	Curve90			UMETA(DisplayName = "90° Curve"),
	Curve45			UMETA(DisplayName = "45° Curve"),
	SCurve			UMETA(DisplayName = "S-Curve"),
	Hairpin			UMETA(DisplayName = "Hairpin"),
	Chicane			UMETA(DisplayName = "Chicane"),
	Ramp			UMETA(DisplayName = "Ramp"),
	Jump			UMETA(DisplayName = "Jump"),
	Tunnel			UMETA(DisplayName = "Tunnel"),
	Bridge			UMETA(DisplayName = "Bridge"),
	Intersection	UMETA(DisplayName = "Intersection"),
	StartFinish		UMETA(DisplayName = "Start/Finish"),
	Checkpoint		UMETA(DisplayName = "Checkpoint"),
	Shortcut		UMETA(DisplayName = "Shortcut")
};

UENUM(BlueprintType)
enum class EMGTrackEnvironment : uint8
{
	Downtown		UMETA(DisplayName = "Downtown"),
	Industrial		UMETA(DisplayName = "Industrial"),
	Harbor			UMETA(DisplayName = "Harbor"),
	Highway			UMETA(DisplayName = "Highway"),
	Mountain		UMETA(DisplayName = "Mountain"),
	Forest			UMETA(DisplayName = "Forest"),
	Desert			UMETA(DisplayName = "Desert"),
	Neon			UMETA(DisplayName = "Neon City")
};

USTRUCT(BlueprintType)
struct FMGTrackPiece
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid PieceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTrackPieceType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform Transform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Scale = FVector::OneVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Width = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Banking = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasBarriers = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName MaterialOverride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FGuid> ConnectedPieces;
};

USTRUCT(BlueprintType)
struct FMGTrackDecoration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid DecorationID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DecorationAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform Transform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Scale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FMGCustomTrackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TrackID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText TrackName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString AuthorID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText AuthorName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTrackEnvironment Environment;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMGTrackPiece> TrackPieces;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMGTrackDecoration> Decorations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 LapCount = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxRacers = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EstimatedLapTime = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime CreatedDate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime ModifiedDate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Downloads = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Likes = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Rating = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsPublished = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFeatured = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Thumbnail = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackPiecePlaced, const FMGTrackPiece&, Piece);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackSaved, const FString&, TrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackPublished, const FString&, TrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrackValidated);

UCLASS()
class MIDNIGHTGRIND_API UMGTrackEditorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable) FOnTrackPiecePlaced OnTrackPiecePlaced;
	UPROPERTY(BlueprintAssignable) FOnTrackSaved OnTrackSaved;
	UPROPERTY(BlueprintAssignable) FOnTrackPublished OnTrackPublished;
	UPROPERTY(BlueprintAssignable) FOnTrackValidated OnTrackValidated;

	// Editor Control
	UFUNCTION(BlueprintCallable) void EnterEditor();
	UFUNCTION(BlueprintCallable) void ExitEditor();
	UFUNCTION(BlueprintPure) bool IsInEditor() const { return bIsEditing; }
	UFUNCTION(BlueprintCallable) void NewTrack(EMGTrackEnvironment Environment);
	UFUNCTION(BlueprintCallable) bool LoadTrack(const FString& TrackID);

	// Track Building
	UFUNCTION(BlueprintCallable) FGuid PlacePiece(EMGTrackPieceType Type, FTransform Transform);
	UFUNCTION(BlueprintCallable) void RemovePiece(FGuid PieceID);
	UFUNCTION(BlueprintCallable) void MovePiece(FGuid PieceID, FTransform NewTransform);
	UFUNCTION(BlueprintCallable) void RotatePiece(FGuid PieceID, FRotator Rotation);
	UFUNCTION(BlueprintCallable) void ScalePiece(FGuid PieceID, FVector Scale);
	UFUNCTION(BlueprintCallable) void SetPieceBanking(FGuid PieceID, float Banking);
	UFUNCTION(BlueprintCallable) void ConnectPieces(FGuid PieceA, FGuid PieceB);
	UFUNCTION(BlueprintCallable) void DisconnectPiece(FGuid PieceID);
	UFUNCTION(BlueprintCallable) FGuid GetNearestSnapPoint(FVector Location, FRotator& OutRotation);

	// Decorations
	UFUNCTION(BlueprintCallable) FGuid PlaceDecoration(FName DecorationAsset, FTransform Transform);
	UFUNCTION(BlueprintCallable) void RemoveDecoration(FGuid DecorationID);
	UFUNCTION(BlueprintCallable) TArray<FName> GetAvailableDecorations(EMGTrackEnvironment Environment) const;

	// Undo/Redo
	UFUNCTION(BlueprintCallable) void Undo();
	UFUNCTION(BlueprintCallable) void Redo();
	UFUNCTION(BlueprintPure) bool CanUndo() const { return UndoStack.Num() > 0; }
	UFUNCTION(BlueprintPure) bool CanRedo() const { return RedoStack.Num() > 0; }

	// Validation
	UFUNCTION(BlueprintCallable) bool ValidateTrack(TArray<FString>& OutErrors);
	UFUNCTION(BlueprintPure) bool IsTrackClosed() const;
	UFUNCTION(BlueprintPure) bool HasStartFinish() const;
	UFUNCTION(BlueprintPure) int32 GetTrackLength() const;

	// Save/Publish
	UFUNCTION(BlueprintCallable) bool SaveTrack();
	UFUNCTION(BlueprintCallable) bool PublishTrack();
	UFUNCTION(BlueprintCallable) void SetTrackMetadata(FText Name, FText Description, int32 LapCount);
	UFUNCTION(BlueprintCallable) UTexture2D* CaptureThumbnail();

	// Browse Community Tracks
	UFUNCTION(BlueprintCallable) TArray<FMGCustomTrackData> GetCommunityTracks(int32 Page = 0, int32 PageSize = 20);
	UFUNCTION(BlueprintCallable) TArray<FMGCustomTrackData> GetFeaturedTracks();
	UFUNCTION(BlueprintCallable) TArray<FMGCustomTrackData> SearchTracks(const FString& Query);
	UFUNCTION(BlueprintCallable) TArray<FMGCustomTrackData> GetMyTracks();
	UFUNCTION(BlueprintCallable) bool DownloadTrack(const FString& TrackID);
	UFUNCTION(BlueprintCallable) void RateTrack(const FString& TrackID, int32 Rating);
	UFUNCTION(BlueprintCallable) void LikeTrack(const FString& TrackID);

	// Current Track Data
	UFUNCTION(BlueprintPure) FMGCustomTrackData GetCurrentTrackData() const { return CurrentTrack; }
	UFUNCTION(BlueprintPure) TArray<FMGTrackPiece> GetTrackPieces() const { return CurrentTrack.TrackPieces; }

protected:
	UPROPERTY() bool bIsEditing = false;
	UPROPERTY() FMGCustomTrackData CurrentTrack;
	UPROPERTY() TArray<FMGCustomTrackData> UndoStack;
	UPROPERTY() TArray<FMGCustomTrackData> RedoStack;
	UPROPERTY() TArray<FMGCustomTrackData> LocalTracks;
	UPROPERTY() FString LocalPlayerID;

	void PushUndoState();
	void SpawnTrackPieceActor(const FMGTrackPiece& Piece);
	void DestroyTrackPieceActor(FGuid PieceID);
	void RebuildTrackMesh();
	FMGTrackPiece* FindPiece(FGuid PieceID);
};
