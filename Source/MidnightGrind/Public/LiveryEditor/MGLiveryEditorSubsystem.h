// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGLiveryEditorSubsystem.h
 * @brief Vehicle Livery Editor and Paint Customization System
 *
 * @section overview Overview
 * This subsystem provides a comprehensive in-game livery editor that allows players
 * to customize the visual appearance of their vehicles. Similar to paint editors in
 * games like Forza Horizon or Gran Turismo, players can create unique designs using
 * paint materials, decals, and various visual effects.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection livery What is a Livery?
 * A "livery" is the complete visual design of a vehicle, including:
 * - Base paint color and material properties
 * - Secondary and accent colors
 * - Decals (stickers, logos, numbers, stripes)
 * - Window tint, rim color, brake caliper color
 * - Underglow/neon lighting effects
 *
 * Think of it like a "skin" for your car that you can fully customize.
 *
 * @subsection decals Decal System (FMGDecalData)
 * Decals are 2D images placed on the vehicle's surface:
 * - **Shape**: Basic geometric shapes (circles, squares, triangles)
 * - **Text**: Custom text with selectable fonts
 * - **Number**: Racing numbers (commonly used on doors/hood)
 * - **Logo**: Pre-made brand logos and designs
 * - **Stripe**: Racing stripes and lines
 * - **Pattern**: Repeating patterns (carbon fiber, camo, etc.)
 * - **Sponsor**: Sponsor logos for realistic racing liveries
 * - **Custom**: User-imported images
 *
 * Each decal has position, scale, rotation, color, opacity, and layer order.
 *
 * @subsection paint Paint Materials (FMGPaintMaterial)
 * Paint materials define how the vehicle surface looks:
 * - **BaseColor**: The primary color (RGB)
 * - **Metallic**: How reflective/metallic (0 = matte plastic, 1 = chrome)
 * - **Roughness**: Surface texture (0 = mirror smooth, 1 = rough)
 * - **ClearCoat**: Protective glossy layer intensity
 * - **Pearlescent**: Color-shifting effect based on viewing angle
 * - **Matte/Chrome**: Special finish flags
 * - **SpecialFinish**: Named finishes like "Carbon", "Camo", "Brushed Metal"
 *
 * @subsection layers Layer System (EMGLiveryLayer)
 * Liveries are built in layers that stack on top of each other:
 * 1. **Base**: Primary paint covering the entire vehicle
 * 2. **Secondary**: Secondary color zones (roof, mirrors, etc.)
 * 3. **Accent**: Trim and detail areas
 * 4. **Decal**: Stickers and graphics layer
 * 5. **Effect**: Special effects like pearlescent or color-shift
 *
 * @section workflow Typical Workflow
 * @code
 * // Get the subsystem
 * UMGLiveryEditorSubsystem* Livery = GetGameInstance()->GetSubsystem<UMGLiveryEditorSubsystem>();
 *
 * // Enter editor mode for a specific vehicle
 * Livery->EnterEditor(FName("nissan_skyline_r34"));
 *
 * // Start with a fresh livery
 * Livery->NewLivery();
 *
 * // Set up base paint (metallic blue)
 * FMGPaintMaterial BluePaint;
 * BluePaint.BaseColor = FLinearColor(0.0f, 0.2f, 0.8f);
 * BluePaint.Metallic = 0.8f;
 * BluePaint.Roughness = 0.2f;
 * BluePaint.ClearCoat = 1.0f;
 * Livery->SetBasePaint(BluePaint);
 *
 * // Add a racing number decal
 * FGuid NumberDecal = Livery->AddDecal(
 *     EMGDecalType::Number,
 *     FName("RacingNumber_32"),
 *     FVector2D(0.3f, 0.5f)  // Position on UV map
 * );
 *
 * // Customize the decal
 * Livery->SetDecalColor(NumberDecal, FLinearColor::White);
 * Livery->SetDecalScale(NumberDecal, FVector2D(1.5f, 1.5f));
 *
 * // Add text decal
 * FGuid TeamName = Livery->AddTextDecal(
 *     "MIDNIGHT GRIND",
 *     FName("Font_Racing"),
 *     FVector2D(0.5f, 0.8f),
 *     FLinearColor::Yellow
 * );
 *
 * // Enable underglow
 * Livery->SetNeon(true, FLinearColor::Blue);
 *
 * // Save the livery
 * Livery->SetLiveryName(FText::FromString("Blue Thunder"));
 * Livery->SaveLivery();
 *
 * // Share with the community
 * Livery->PublishLivery();
 *
 * // Exit editor
 * Livery->ExitEditor();
 * @endcode
 *
 * @section undo Undo/Redo System
 * The editor maintains an undo stack for mistake recovery:
 * - Every action (paint change, decal placement) pushes to the stack
 * - Undo() reverts to the previous state
 * - Redo() restores an undone action
 * - CanUndo()/CanRedo() check if operations are available
 *
 * @section community Community Features
 * Players can share their creations:
 * - **PublishLivery()**: Upload to community servers
 * - **GetCommunityLiveries()**: Browse other players' designs
 * - **SearchLiveries()**: Find specific designs by keyword
 * - **DownloadLivery()**: Get a community livery
 * - **LikeLivery()**: Show appreciation for a design
 * - Downloads and likes tracked per livery
 *
 * @section delegates Event Delegates
 * Subscribe to these events to respond to editor actions:
 * - **OnDecalPlaced**: A decal was added to the livery
 * - **OnPaintChanged**: Paint material was modified
 * - **OnLiverySaved**: Livery was saved locally
 * - **OnLiveryPublished**: Livery was uploaded to community
 *
 * @section technical Technical Notes
 * - This is a GameInstanceSubsystem (persists across level loads)
 * - UV coordinates use 0-1 normalized space
 * - Decal LayerOrder determines render priority (higher = on top)
 * - Mirrored decals automatically duplicate to opposite side
 * - Thumbnails are captured for preview in menus
 *
 * @see FMGLiveryData for the complete livery data structure
 * @see FMGDecalData for individual decal properties
 * @see FMGPaintMaterial for paint surface properties
 */

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
