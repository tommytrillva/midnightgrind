// Copyright Midnight Grind. All Rights Reserved.

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
