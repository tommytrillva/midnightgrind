// Copyright Midnight Grind. All Rights Reserved.

#include "LiveryEditor/MGLiveryEditorSubsystem.h"
#include "HAL/PlatformMisc.h"

void UMGLiveryEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LocalPlayerID = FPlatformMisc::GetDeviceId();
}

void UMGLiveryEditorSubsystem::Deinitialize()
{
	if (bIsEditing) ExitEditor();
	Super::Deinitialize();
}

void UMGLiveryEditorSubsystem::EnterEditor(FName VehicleID)
{
	bIsEditing = true;
	CurrentLivery.VehicleID = VehicleID;
	UndoStack.Empty();
	RedoStack.Empty();
}

void UMGLiveryEditorSubsystem::ExitEditor()
{
	bIsEditing = false;
}

void UMGLiveryEditorSubsystem::NewLivery()
{
	CurrentLivery = FMGLiveryData();
	CurrentLivery.LiveryID = FGuid::NewGuid().ToString();
	CurrentLivery.AuthorID = LocalPlayerID;
	CurrentLivery.CreatedDate = FDateTime::Now();
	CurrentLivery.ModifiedDate = FDateTime::Now();
	UndoStack.Empty();
	RedoStack.Empty();
}

bool UMGLiveryEditorSubsystem::LoadLivery(const FString& LiveryID)
{
	for (const FMGLiveryData& Livery : LocalLiveries)
	{
		if (Livery.LiveryID == LiveryID)
		{
			CurrentLivery = Livery;
			UpdateVehicleMaterial();
			return true;
		}
	}
	return false;
}

void UMGLiveryEditorSubsystem::ApplyLiveryToVehicle(FName VehicleID, const FString& LiveryID)
{
	// Would apply livery to vehicle actor
}

void UMGLiveryEditorSubsystem::SetBasePaint(const FMGPaintMaterial& Paint)
{
	PushUndoState();
	CurrentLivery.BasePaint = Paint;
	CurrentLivery.ModifiedDate = FDateTime::Now();
	UpdateVehicleMaterial();
	OnPaintChanged.Broadcast();
}

void UMGLiveryEditorSubsystem::SetSecondaryPaint(const FMGPaintMaterial& Paint)
{
	PushUndoState();
	CurrentLivery.SecondaryPaint = Paint;
	CurrentLivery.ModifiedDate = FDateTime::Now();
	UpdateVehicleMaterial();
	OnPaintChanged.Broadcast();
}

void UMGLiveryEditorSubsystem::SetAccentPaint(const FMGPaintMaterial& Paint)
{
	PushUndoState();
	CurrentLivery.AccentPaint = Paint;
	CurrentLivery.ModifiedDate = FDateTime::Now();
	UpdateVehicleMaterial();
	OnPaintChanged.Broadcast();
}

void UMGLiveryEditorSubsystem::SetWindowTint(FLinearColor Color)
{
	PushUndoState();
	CurrentLivery.WindowTint = Color;
	UpdateVehicleMaterial();
}

void UMGLiveryEditorSubsystem::SetRimColor(FLinearColor Color)
{
	PushUndoState();
	CurrentLivery.RimColor = Color;
	UpdateVehicleMaterial();
}

void UMGLiveryEditorSubsystem::SetBrakeColor(FLinearColor Color)
{
	PushUndoState();
	CurrentLivery.BrakeColor = Color;
	UpdateVehicleMaterial();
}

void UMGLiveryEditorSubsystem::SetNeon(bool bEnabled, FLinearColor Color)
{
	PushUndoState();
	CurrentLivery.bNeonEnabled = bEnabled;
	CurrentLivery.NeonColor = Color;
	UpdateVehicleMaterial();
}

FGuid UMGLiveryEditorSubsystem::AddDecal(EMGDecalType Type, FName DecalAsset, FVector2D Position)
{
	PushUndoState();

	FMGDecalData NewDecal;
	NewDecal.DecalID = FGuid::NewGuid();
	NewDecal.Type = Type;
	NewDecal.DecalAsset = DecalAsset;
	NewDecal.Position = Position;
	NewDecal.LayerOrder = CurrentLivery.Decals.Num();

	CurrentLivery.Decals.Add(NewDecal);
	CurrentLivery.ModifiedDate = FDateTime::Now();

	OnDecalPlaced.Broadcast(NewDecal);
	return NewDecal.DecalID;
}

void UMGLiveryEditorSubsystem::RemoveDecal(FGuid DecalID)
{
	PushUndoState();
	CurrentLivery.Decals.RemoveAll([DecalID](const FMGDecalData& D) { return D.DecalID == DecalID; });
	CurrentLivery.ModifiedDate = FDateTime::Now();
	UpdateVehicleMaterial();
}

void UMGLiveryEditorSubsystem::MoveDecal(FGuid DecalID, FVector2D NewPosition)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->Position = NewPosition;
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::ScaleDecal(FGuid DecalID, FVector2D NewScale)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->Scale = NewScale;
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::RotateDecal(FGuid DecalID, float NewRotation)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->Rotation = NewRotation;
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::SetDecalColor(FGuid DecalID, FLinearColor Color)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->Color = Color;
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::SetDecalOpacity(FGuid DecalID, float Opacity)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->Opacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::SetDecalLayer(FGuid DecalID, int32 LayerOrder)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->LayerOrder = LayerOrder;
		CurrentLivery.Decals.Sort([](const FMGDecalData& A, const FMGDecalData& B) { return A.LayerOrder < B.LayerOrder; });
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::MirrorDecal(FGuid DecalID)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		PushUndoState();
		Decal->bMirrored = !Decal->bMirrored;
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::DuplicateDecal(FGuid DecalID)
{
	if (FMGDecalData* Decal = FindDecal(DecalID))
	{
		FMGDecalData NewDecal = *Decal;
		NewDecal.DecalID = FGuid::NewGuid();
		NewDecal.Position += FVector2D(0.05f, 0.05f);
		NewDecal.LayerOrder = CurrentLivery.Decals.Num();
		CurrentLivery.Decals.Add(NewDecal);
	}
}

FGuid UMGLiveryEditorSubsystem::AddTextDecal(const FString& Text, FName Font, FVector2D Position, FLinearColor Color)
{
	PushUndoState();

	FMGDecalData NewDecal;
	NewDecal.DecalID = FGuid::NewGuid();
	NewDecal.Type = EMGDecalType::Text;
	NewDecal.TextContent = Text;
	NewDecal.FontAsset = Font;
	NewDecal.Position = Position;
	NewDecal.Color = Color;
	NewDecal.LayerOrder = CurrentLivery.Decals.Num();

	CurrentLivery.Decals.Add(NewDecal);
	OnDecalPlaced.Broadcast(NewDecal);
	return NewDecal.DecalID;
}

TArray<FName> UMGLiveryEditorSubsystem::GetAvailableDecals(EMGDecalType Type) const
{
	TArray<FName> Decals;
	switch (Type)
	{
		case EMGDecalType::Shape:
			Decals = { FName("Circle"), FName("Square"), FName("Triangle"), FName("Star"), FName("Heart"), FName("Arrow") };
			break;
		case EMGDecalType::Stripe:
			Decals = { FName("Racing_Stripe"), FName("Side_Stripe"), FName("Hood_Stripe"), FName("GT_Stripe") };
			break;
		case EMGDecalType::Number:
			for (int32 i = 0; i <= 99; i++) Decals.Add(FName(*FString::Printf(TEXT("Number_%02d"), i)));
			break;
		case EMGDecalType::Logo:
			Decals = { FName("Logo_MidnightGrind"), FName("Logo_Skull"), FName("Logo_Flame"), FName("Logo_Lightning") };
			break;
		default:
			break;
	}
	return Decals;
}

TArray<FName> UMGLiveryEditorSubsystem::GetAvailableFonts() const
{
	return { FName("Racing"), FName("Block"), FName("Script"), FName("Neon"), FName("Graffiti"), FName("Digital") };
}

TArray<FName> UMGLiveryEditorSubsystem::GetAvailableFinishes() const
{
	return { FName("Carbon"), FName("Camo"), FName("Brushed_Metal"), FName("Holographic"), FName("Glitter") };
}

TArray<FLinearColor> UMGLiveryEditorSubsystem::GetPresetColors() const
{
	return {
		FLinearColor::Red, FLinearColor::Blue, FLinearColor::Green, FLinearColor::Yellow,
		FLinearColor::Black, FLinearColor::White, FLinearColor(1,0.5f,0), FLinearColor(0.5f,0,1),
		FLinearColor(0,1,1), FLinearColor(1,0,1), FLinearColor(0.5f,0.5f,0.5f), FLinearColor(1,0.84f,0)
	};
}

void UMGLiveryEditorSubsystem::Undo()
{
	if (UndoStack.Num() > 0)
	{
		RedoStack.Add(CurrentLivery);
		CurrentLivery = UndoStack.Pop();
		UpdateVehicleMaterial();
	}
}

void UMGLiveryEditorSubsystem::Redo()
{
	if (RedoStack.Num() > 0)
	{
		UndoStack.Add(CurrentLivery);
		CurrentLivery = RedoStack.Pop();
		UpdateVehicleMaterial();
	}
}

bool UMGLiveryEditorSubsystem::SaveLivery()
{
	CurrentLivery.ModifiedDate = FDateTime::Now();

	int32 ExistingIndex = LocalLiveries.IndexOfByPredicate([this](const FMGLiveryData& L) {
		return L.LiveryID == CurrentLivery.LiveryID;
	});

	if (ExistingIndex != INDEX_NONE)
		LocalLiveries[ExistingIndex] = CurrentLivery;
	else
		LocalLiveries.Add(CurrentLivery);

	OnLiverySaved.Broadcast(CurrentLivery.LiveryID);
	return true;
}

bool UMGLiveryEditorSubsystem::PublishLivery()
{
	if (!SaveLivery()) return false;
	CurrentLivery.bIsPublished = true;
	OnLiveryPublished.Broadcast(CurrentLivery.LiveryID);
	return true;
}

void UMGLiveryEditorSubsystem::SetLiveryName(FText Name)
{
	CurrentLivery.LiveryName = Name;
}

UTexture2D* UMGLiveryEditorSubsystem::CaptureThumbnail()
{
	return nullptr;
}

bool UMGLiveryEditorSubsystem::ExportLivery(const FString& FilePath)
{
	return false;
}

bool UMGLiveryEditorSubsystem::ImportLivery(const FString& FilePath)
{
	return false;
}

TArray<FMGLiveryData> UMGLiveryEditorSubsystem::GetCommunityLiveries(FName VehicleID, int32 Page)
{
	return TArray<FMGLiveryData>();
}

TArray<FMGLiveryData> UMGLiveryEditorSubsystem::SearchLiveries(const FString& Query, FName VehicleID)
{
	return TArray<FMGLiveryData>();
}

TArray<FMGLiveryData> UMGLiveryEditorSubsystem::GetMyLiveries()
{
	return LocalLiveries;
}

bool UMGLiveryEditorSubsystem::DownloadLivery(const FString& LiveryID)
{
	return false;
}

void UMGLiveryEditorSubsystem::LikeLivery(const FString& LiveryID)
{
}

void UMGLiveryEditorSubsystem::PushUndoState()
{
	UndoStack.Add(CurrentLivery);
	if (UndoStack.Num() > 50) UndoStack.RemoveAt(0);
	RedoStack.Empty();
}

void UMGLiveryEditorSubsystem::UpdateVehicleMaterial()
{
	// Would update vehicle material instance dynamic
}

FMGDecalData* UMGLiveryEditorSubsystem::FindDecal(FGuid DecalID)
{
	return CurrentLivery.Decals.FindByPredicate([DecalID](const FMGDecalData& D) { return D.DecalID == DecalID; });
}
