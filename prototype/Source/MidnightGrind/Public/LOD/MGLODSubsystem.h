// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLODSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGLODLevel : uint8
{
	LOD0,  // Full detail
	LOD1,  // High
	LOD2,  // Medium
	LOD3,  // Low
	LOD4,  // Very Low
	Culled // Not rendered
};

UENUM(BlueprintType)
enum class EMGLODCategory : uint8
{
	Vehicle,
	Environment,
	Props,
	Characters,
	Effects,
	UI
};

USTRUCT(BlueprintType)
struct FMGLODSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD1Distance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD2Distance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD3Distance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD4Distance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CullDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LODBias = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForceLOD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLODLevel ForcedLOD = EMGLODLevel::LOD0;
};

USTRUCT(BlueprintType)
struct FMGLODStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalObjects = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LOD0Count = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LOD1Count = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LOD2Count = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LOD3Count = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LOD4Count = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CulledCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 TotalTriangles = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLODSettingsChanged, EMGLODCategory, Category, const FMGLODSettings&, Settings);

UCLASS()
class MIDNIGHTGRIND_API UMGLODSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Settings per category
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetLODSettings(EMGLODCategory Category, const FMGLODSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "LOD")
	FMGLODSettings GetLODSettings(EMGLODCategory Category) const;

	// Quality presets
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void ApplyQualityPreset(int32 QualityLevel);

	// Runtime LOD control
	UFUNCTION(BlueprintPure, Category = "LOD")
	EMGLODLevel CalculateLOD(EMGLODCategory Category, float Distance) const;

	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetGlobalLODBias(float Bias);

	UFUNCTION(BlueprintPure, Category = "LOD")
	float GetGlobalLODBias() const { return GlobalLODBias; }

	// Distance scaling for speed
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetSpeedBasedLODScaling(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "LOD")
	void UpdateSpeedFactor(float CurrentSpeed);

	// Stats
	UFUNCTION(BlueprintPure, Category = "LOD")
	FMGLODStats GetLODStats() const { return CurrentStats; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "LOD|Events")
	FMGOnLODSettingsChanged OnLODSettingsChanged;

protected:
	void UpdateStats();

private:
	UPROPERTY()
	TMap<EMGLODCategory, FMGLODSettings> CategorySettings;

	FMGLODStats CurrentStats;
	float GlobalLODBias = 0.0f;
	float SpeedFactor = 1.0f;
	bool bSpeedBasedScaling = true;
};
