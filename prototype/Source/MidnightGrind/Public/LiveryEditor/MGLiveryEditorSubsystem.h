// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLiveryEditorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGDecalType : uint8
{
	Shape			UMETA(DisplayName = "Shape"),
	Text			UMETA(DisplayName = "Text"),
	Number			UMETA(DisplayName = "Number"),
	Logo			UMETA(DisplayName = "Logo"),
	Stripe			UMETA(DisplayName = "Stripe"),
	Pattern			UMETA(DisplayName = "Pattern"),
	Sponsor			UMETA(DisplayName = "Sponsor"),
	Custom			UMETA(DisplayName = "Custom Import")
};

UENUM(BlueprintType)
enum class EMGLiveryLayer : uint8
{
	Base		UMETA(DisplayName = "Base Paint"),
	Secondary	UMETA(DisplayName = "Secondary"),
	Accent		UMETA(DisplayName = "Accent"),
	Decal		UMETA(DisplayName = "Decals"),
	Effect		UMETA(DisplayName = "Effects")
};

USTRUCT(BlueprintType)
struct FMGDecalData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid DecalID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGDecalType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DecalAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position = FVector2D(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Scale = FVector2D(1.0f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Rotation = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Opacity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 LayerOrder = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMirrored = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TextContent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName FontAsset;
};

USTRUCT(BlueprintType)
struct FMGPaintMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BaseColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Metallic = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Roughness = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ClearCoat = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Pearlescent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor PearlescentColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMatte = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bChrome = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SpecialFinish; // Carbon, Camo, etc.
};

USTRUCT(BlueprintType)
struct FMGLiveryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LiveryID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText LiveryName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName VehicleID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString AuthorID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText AuthorName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMGPaintMaterial BasePaint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMGPaintMaterial SecondaryPaint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMGPaintMaterial AccentPaint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMGDecalData> Decals;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor WindowTint = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor RimColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BrakeColor = FLinearColor::Red;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor NeonColor = FLinearColor::Blue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bNeonEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime CreatedDate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime ModifiedDate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Downloads = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Likes = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsPublished = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Thumbnail = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDecalPlaced, const FMGDecalData&, Decal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPaintChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiverySaved, const FString&, LiveryID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiveryPublished, const FString&, LiveryID);

UCLASS()
class MIDNIGHTGRIND_API UMGLiveryEditorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable) FOnDecalPlaced OnDecalPlaced;
	UPROPERTY(BlueprintAssignable) FOnPaintChanged OnPaintChanged;
	UPROPERTY(BlueprintAssignable) FOnLiverySaved OnLiverySaved;
	UPROPERTY(BlueprintAssignable) FOnLiveryPublished OnLiveryPublished;

	// Editor Control
	UFUNCTION(BlueprintCallable) void EnterEditor(FName VehicleID);
	UFUNCTION(BlueprintCallable) void ExitEditor();
	UFUNCTION(BlueprintPure) bool IsInEditor() const { return bIsEditing; }
	UFUNCTION(BlueprintCallable) void NewLivery();
	UFUNCTION(BlueprintCallable) bool LoadLivery(const FString& LiveryID);
	UFUNCTION(BlueprintCallable) void ApplyLiveryToVehicle(FName VehicleID, const FString& LiveryID);

	// Paint
	UFUNCTION(BlueprintCallable) void SetBasePaint(const FMGPaintMaterial& Paint);
	UFUNCTION(BlueprintCallable) void SetSecondaryPaint(const FMGPaintMaterial& Paint);
	UFUNCTION(BlueprintCallable) void SetAccentPaint(const FMGPaintMaterial& Paint);
	UFUNCTION(BlueprintCallable) void SetWindowTint(FLinearColor Color);
	UFUNCTION(BlueprintCallable) void SetRimColor(FLinearColor Color);
	UFUNCTION(BlueprintCallable) void SetBrakeColor(FLinearColor Color);
	UFUNCTION(BlueprintCallable) void SetNeon(bool bEnabled, FLinearColor Color);

	// Decals
	UFUNCTION(BlueprintCallable) FGuid AddDecal(EMGDecalType Type, FName DecalAsset, FVector2D Position);
	UFUNCTION(BlueprintCallable) void RemoveDecal(FGuid DecalID);
	UFUNCTION(BlueprintCallable) void MoveDecal(FGuid DecalID, FVector2D NewPosition);
	UFUNCTION(BlueprintCallable) void ScaleDecal(FGuid DecalID, FVector2D NewScale);
	UFUNCTION(BlueprintCallable) void RotateDecal(FGuid DecalID, float NewRotation);
	UFUNCTION(BlueprintCallable) void SetDecalColor(FGuid DecalID, FLinearColor Color);
	UFUNCTION(BlueprintCallable) void SetDecalOpacity(FGuid DecalID, float Opacity);
	UFUNCTION(BlueprintCallable) void SetDecalLayer(FGuid DecalID, int32 LayerOrder);
	UFUNCTION(BlueprintCallable) void MirrorDecal(FGuid DecalID);
	UFUNCTION(BlueprintCallable) void DuplicateDecal(FGuid DecalID);
	UFUNCTION(BlueprintCallable) FGuid AddTextDecal(const FString& Text, FName Font, FVector2D Position, FLinearColor Color);

	// Assets
	UFUNCTION(BlueprintPure) TArray<FName> GetAvailableDecals(EMGDecalType Type) const;
	UFUNCTION(BlueprintPure) TArray<FName> GetAvailableFonts() const;
	UFUNCTION(BlueprintPure) TArray<FName> GetAvailableFinishes() const;
	UFUNCTION(BlueprintPure) TArray<FLinearColor> GetPresetColors() const;

	// Undo/Redo
	UFUNCTION(BlueprintCallable) void Undo();
	UFUNCTION(BlueprintCallable) void Redo();
	UFUNCTION(BlueprintPure) bool CanUndo() const { return UndoStack.Num() > 0; }
	UFUNCTION(BlueprintPure) bool CanRedo() const { return RedoStack.Num() > 0; }

	// Save/Publish
	UFUNCTION(BlueprintCallable) bool SaveLivery();
	UFUNCTION(BlueprintCallable) bool PublishLivery();
	UFUNCTION(BlueprintCallable) void SetLiveryName(FText Name);
	UFUNCTION(BlueprintCallable) UTexture2D* CaptureThumbnail();
	UFUNCTION(BlueprintCallable) bool ExportLivery(const FString& FilePath);
	UFUNCTION(BlueprintCallable) bool ImportLivery(const FString& FilePath);

	// Browse Community
	UFUNCTION(BlueprintCallable) TArray<FMGLiveryData> GetCommunityLiveries(FName VehicleID, int32 Page = 0);
	UFUNCTION(BlueprintCallable) TArray<FMGLiveryData> SearchLiveries(const FString& Query, FName VehicleID);
	UFUNCTION(BlueprintCallable) TArray<FMGLiveryData> GetMyLiveries();
	UFUNCTION(BlueprintCallable) bool DownloadLivery(const FString& LiveryID);
	UFUNCTION(BlueprintCallable) void LikeLivery(const FString& LiveryID);

	// Current Data
	UFUNCTION(BlueprintPure) FMGLiveryData GetCurrentLivery() const { return CurrentLivery; }

protected:
	UPROPERTY() bool bIsEditing = false;
	UPROPERTY() FMGLiveryData CurrentLivery;
	UPROPERTY() TArray<FMGLiveryData> UndoStack;
	UPROPERTY() TArray<FMGLiveryData> RedoStack;
	UPROPERTY() TArray<FMGLiveryData> LocalLiveries;
	UPROPERTY() FString LocalPlayerID;

	void PushUndoState();
	void UpdateVehicleMaterial();
	FMGDecalData* FindDecal(FGuid DecalID);
};
