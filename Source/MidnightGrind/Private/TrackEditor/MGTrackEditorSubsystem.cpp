// Copyright Midnight Grind. All Rights Reserved.

#include "TrackEditor/MGTrackEditorSubsystem.h"
#include "HAL/PlatformMisc.h"

void UMGTrackEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LocalPlayerID = FPlatformMisc::GetDeviceId();
}

void UMGTrackEditorSubsystem::Deinitialize()
{
	if (bIsEditing) ExitEditor();
	Super::Deinitialize();
}

void UMGTrackEditorSubsystem::EnterEditor()
{
	bIsEditing = true;
	UndoStack.Empty();
	RedoStack.Empty();
}

void UMGTrackEditorSubsystem::ExitEditor()
{
	bIsEditing = false;
}

void UMGTrackEditorSubsystem::NewTrack(EMGTrackEnvironment Environment)
{
	CurrentTrack = FMGCustomTrackData();
	CurrentTrack.TrackID = FGuid::NewGuid().ToString();
	CurrentTrack.AuthorID = LocalPlayerID;
	CurrentTrack.Environment = Environment;
	CurrentTrack.CreatedDate = FDateTime::Now();
	CurrentTrack.ModifiedDate = FDateTime::Now();
	UndoStack.Empty();
	RedoStack.Empty();
}

bool UMGTrackEditorSubsystem::LoadTrack(const FString& TrackID)
{
	for (const FMGCustomTrackData& Track : LocalTracks)
	{
		if (Track.TrackID == TrackID)
		{
			CurrentTrack = Track;
			RebuildTrackMesh();
			return true;
		}
	}
	return false;
}

FGuid UMGTrackEditorSubsystem::PlacePiece(EMGTrackPieceType Type, FTransform Transform)
{
	PushUndoState();

	FMGTrackPiece NewPiece;
	NewPiece.PieceID = FGuid::NewGuid();
	NewPiece.Type = Type;
	NewPiece.Transform = Transform;

	CurrentTrack.TrackPieces.Add(NewPiece);
	CurrentTrack.ModifiedDate = FDateTime::Now();

	SpawnTrackPieceActor(NewPiece);
	OnTrackPiecePlaced.Broadcast(NewPiece);

	return NewPiece.PieceID;
}

void UMGTrackEditorSubsystem::RemovePiece(FGuid PieceID)
{
	PushUndoState();
	DestroyTrackPieceActor(PieceID);
	CurrentTrack.TrackPieces.RemoveAll([PieceID](const FMGTrackPiece& P) { return P.PieceID == PieceID; });
	CurrentTrack.ModifiedDate = FDateTime::Now();
}

void UMGTrackEditorSubsystem::MovePiece(FGuid PieceID, FTransform NewTransform)
{
	if (FMGTrackPiece* Piece = FindPiece(PieceID))
	{
		PushUndoState();
		Piece->Transform = NewTransform;
		CurrentTrack.ModifiedDate = FDateTime::Now();
		RebuildTrackMesh();
	}
}

void UMGTrackEditorSubsystem::RotatePiece(FGuid PieceID, FRotator Rotation)
{
	if (FMGTrackPiece* Piece = FindPiece(PieceID))
	{
		PushUndoState();
		Piece->Transform.SetRotation(Rotation.Quaternion());
		CurrentTrack.ModifiedDate = FDateTime::Now();
		RebuildTrackMesh();
	}
}

void UMGTrackEditorSubsystem::ScalePiece(FGuid PieceID, FVector Scale)
{
	if (FMGTrackPiece* Piece = FindPiece(PieceID))
	{
		PushUndoState();
		Piece->Scale = Scale;
		CurrentTrack.ModifiedDate = FDateTime::Now();
		RebuildTrackMesh();
	}
}

void UMGTrackEditorSubsystem::SetPieceBanking(FGuid PieceID, float Banking)
{
	if (FMGTrackPiece* Piece = FindPiece(PieceID))
	{
		PushUndoState();
		Piece->Banking = FMath::Clamp(Banking, -45.0f, 45.0f);
		CurrentTrack.ModifiedDate = FDateTime::Now();
		RebuildTrackMesh();
	}
}

void UMGTrackEditorSubsystem::ConnectPieces(FGuid PieceA, FGuid PieceB)
{
	FMGTrackPiece* A = FindPiece(PieceA);
	FMGTrackPiece* B = FindPiece(PieceB);
	if (A && B)
	{
		PushUndoState();
		A->ConnectedPieces.AddUnique(PieceB);
		B->ConnectedPieces.AddUnique(PieceA);
	}
}

void UMGTrackEditorSubsystem::DisconnectPiece(FGuid PieceID)
{
	FMGTrackPiece* Piece = FindPiece(PieceID);
	if (Piece)
	{
		PushUndoState();
		for (FGuid ConnectedID : Piece->ConnectedPieces)
		{
			if (FMGTrackPiece* Connected = FindPiece(ConnectedID))
			{
				Connected->ConnectedPieces.Remove(PieceID);
			}
		}
		Piece->ConnectedPieces.Empty();
	}
}

FGuid UMGTrackEditorSubsystem::GetNearestSnapPoint(FVector Location, FRotator& OutRotation)
{
	FGuid NearestID;
	float NearestDist = MAX_FLT;

	for (const FMGTrackPiece& Piece : CurrentTrack.TrackPieces)
	{
		float Dist = FVector::Distance(Location, Piece.Transform.GetLocation());
		if (Dist < NearestDist && Dist < 500.0f)
		{
			NearestDist = Dist;
			NearestID = Piece.PieceID;
			OutRotation = Piece.Transform.Rotator();
		}
	}
	return NearestID;
}

FGuid UMGTrackEditorSubsystem::PlaceDecoration(FName DecorationAsset, FTransform Transform)
{
	PushUndoState();

	FMGTrackDecoration NewDecoration;
	NewDecoration.DecorationID = FGuid::NewGuid();
	NewDecoration.DecorationAsset = DecorationAsset;
	NewDecoration.Transform = Transform;

	CurrentTrack.Decorations.Add(NewDecoration);
	CurrentTrack.ModifiedDate = FDateTime::Now();

	return NewDecoration.DecorationID;
}

void UMGTrackEditorSubsystem::RemoveDecoration(FGuid DecorationID)
{
	PushUndoState();
	CurrentTrack.Decorations.RemoveAll([DecorationID](const FMGTrackDecoration& D) { return D.DecorationID == DecorationID; });
	CurrentTrack.ModifiedDate = FDateTime::Now();
}

TArray<FName> UMGTrackEditorSubsystem::GetAvailableDecorations(EMGTrackEnvironment Environment) const
{
	TArray<FName> Decorations;
	Decorations.Add(FName(TEXT("Barrier_Concrete")));
	Decorations.Add(FName(TEXT("Barrier_TireWall")));
	Decorations.Add(FName(TEXT("Cone_Traffic")));
	Decorations.Add(FName(TEXT("Sign_Speed")));
	Decorations.Add(FName(TEXT("Sign_Direction")));
	Decorations.Add(FName(TEXT("Light_Street")));
	Decorations.Add(FName(TEXT("Tree_Generic")));
	Decorations.Add(FName(TEXT("Building_Generic")));
	return Decorations;
}

void UMGTrackEditorSubsystem::Undo()
{
	if (UndoStack.Num() > 0)
	{
		RedoStack.Add(CurrentTrack);
		CurrentTrack = UndoStack.Pop();
		RebuildTrackMesh();
	}
}

void UMGTrackEditorSubsystem::Redo()
{
	if (RedoStack.Num() > 0)
	{
		UndoStack.Add(CurrentTrack);
		CurrentTrack = RedoStack.Pop();
		RebuildTrackMesh();
	}
}

bool UMGTrackEditorSubsystem::ValidateTrack(TArray<FString>& OutErrors)
{
	OutErrors.Empty();

	if (CurrentTrack.TrackPieces.Num() < 4)
		OutErrors.Add(TEXT("Track must have at least 4 pieces"));

	if (!HasStartFinish())
		OutErrors.Add(TEXT("Track must have a start/finish line"));

	if (!IsTrackClosed())
		OutErrors.Add(TEXT("Track must form a closed loop"));

	if (CurrentTrack.TrackName.IsEmpty())
		OutErrors.Add(TEXT("Track must have a name"));

	if (OutErrors.Num() == 0)
		OnTrackValidated.Broadcast();

	return OutErrors.Num() == 0;
}

bool UMGTrackEditorSubsystem::IsTrackClosed() const
{
	// Would check if track forms a closed loop via connected pieces
	return true;
}

bool UMGTrackEditorSubsystem::HasStartFinish() const
{
	for (const FMGTrackPiece& Piece : CurrentTrack.TrackPieces)
	{
		if (Piece.Type == EMGTrackPieceType::StartFinish)
			return true;
	}
	return false;
}

int32 UMGTrackEditorSubsystem::GetTrackLength() const
{
	int32 Length = 0;
	for (const FMGTrackPiece& Piece : CurrentTrack.TrackPieces)
	{
		Length += 100; // Simplified - would calculate actual length
	}
	return Length;
}

bool UMGTrackEditorSubsystem::SaveTrack()
{
	TArray<FString> Errors;
	if (!ValidateTrack(Errors))
		return false;

	CurrentTrack.ModifiedDate = FDateTime::Now();

	int32 ExistingIndex = LocalTracks.IndexOfByPredicate([this](const FMGCustomTrackData& T) {
		return T.TrackID == CurrentTrack.TrackID;
	});

	if (ExistingIndex != INDEX_NONE)
		LocalTracks[ExistingIndex] = CurrentTrack;
	else
		LocalTracks.Add(CurrentTrack);

	OnTrackSaved.Broadcast(CurrentTrack.TrackID);
	return true;
}

bool UMGTrackEditorSubsystem::PublishTrack()
{
	if (!SaveTrack())
		return false;

	CurrentTrack.bIsPublished = true;
	OnTrackPublished.Broadcast(CurrentTrack.TrackID);
	return true;
}

void UMGTrackEditorSubsystem::SetTrackMetadata(FText Name, FText Description, int32 LapCount)
{
	CurrentTrack.TrackName = Name;
	CurrentTrack.Description = Description;
	CurrentTrack.LapCount = FMath::Clamp(LapCount, 1, 10);
	CurrentTrack.ModifiedDate = FDateTime::Now();
}

UTexture2D* UMGTrackEditorSubsystem::CaptureThumbnail()
{
	// Would capture screenshot of track from overhead view
	return nullptr;
}

TArray<FMGCustomTrackData> UMGTrackEditorSubsystem::GetCommunityTracks(int32 Page, int32 PageSize)
{
	// Would fetch from server
	return TArray<FMGCustomTrackData>();
}

TArray<FMGCustomTrackData> UMGTrackEditorSubsystem::GetFeaturedTracks()
{
	return TArray<FMGCustomTrackData>();
}

TArray<FMGCustomTrackData> UMGTrackEditorSubsystem::SearchTracks(const FString& Query)
{
	return TArray<FMGCustomTrackData>();
}

TArray<FMGCustomTrackData> UMGTrackEditorSubsystem::GetMyTracks()
{
	return LocalTracks;
}

bool UMGTrackEditorSubsystem::DownloadTrack(const FString& TrackID)
{
	// Would download from server
	return false;
}

void UMGTrackEditorSubsystem::RateTrack(const FString& TrackID, int32 Rating)
{
	// Would send rating to server
}

void UMGTrackEditorSubsystem::LikeTrack(const FString& TrackID)
{
	// Would send like to server
}

void UMGTrackEditorSubsystem::PushUndoState()
{
	UndoStack.Add(CurrentTrack);
	if (UndoStack.Num() > 50)
		UndoStack.RemoveAt(0);
	RedoStack.Empty();
}

void UMGTrackEditorSubsystem::SpawnTrackPieceActor(const FMGTrackPiece& Piece)
{
	// Would spawn actor in world
}

void UMGTrackEditorSubsystem::DestroyTrackPieceActor(FGuid PieceID)
{
	// Would destroy actor in world
}

void UMGTrackEditorSubsystem::RebuildTrackMesh()
{
	// Would rebuild procedural track mesh
}

FMGTrackPiece* UMGTrackEditorSubsystem::FindPiece(FGuid PieceID)
{
	return CurrentTrack.TrackPieces.FindByPredicate([PieceID](const FMGTrackPiece& P) { return P.PieceID == PieceID; });
}
