// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCertificationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGPlatform : uint8
{
	PC,
	PlayStation5,
	XboxSeriesX,
	NintendoSwitch,
	Steam,
	EpicGames
};

UENUM(BlueprintType)
enum class EMGAgeRating : uint8
{
	ESRB_Everyone,
	ESRB_Everyone10,
	ESRB_Teen,
	ESRB_Mature,
	PEGI_3,
	PEGI_7,
	PEGI_12,
	PEGI_16,
	PEGI_18,
	CERO_A,
	CERO_B,
	CERO_C,
	USK_0,
	USK_6,
	USK_12,
	USK_16,
	USK_18
};

USTRUCT(BlueprintType)
struct FMGCertificationRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequirementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatform Platform = EMGPlatform::PC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FailureReason;
};

USTRUCT(BlueprintType)
struct FMGContentDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bViolence = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMildLanguage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlineInteraction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInGamePurchases = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUserGeneratedContent = true;
};

UCLASS()
class MIDNIGHTGRIND_API UMGCertificationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Platform Detection
	UFUNCTION(BlueprintPure, Category = "Certification")
	EMGPlatform GetCurrentPlatform() const { return CurrentPlatform; }

	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsPlatform(EMGPlatform Platform) const { return CurrentPlatform == Platform; }

	// Age Rating
	UFUNCTION(BlueprintPure, Category = "Certification")
	EMGAgeRating GetAgeRating() const { return CurrentAgeRating; }

	UFUNCTION(BlueprintPure, Category = "Certification")
	FMGContentDescriptor GetContentDescriptors() const { return ContentDescriptors; }

	// Platform Requirements
	UFUNCTION(BlueprintPure, Category = "Certification")
	TArray<FMGCertificationRequirement> GetRequirements() const { return Requirements; }

	UFUNCTION(BlueprintCallable, Category = "Certification")
	bool ValidateRequirement(FName RequirementID);

	UFUNCTION(BlueprintPure, Category = "Certification")
	bool AreAllRequirementsMet() const;

	// Suspend/Resume (Console requirement)
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnApplicationSuspending();

	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnApplicationResuming();

	// Network State (Console requirement)
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnNetworkStatusChanged(bool bIsOnline);

	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsNetworkAvailable() const { return bNetworkAvailable; }

	// Controller (Console requirement)
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnControllerDisconnected(int32 ControllerID);

	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnControllerReconnected(int32 ControllerID);

	// User Sign-In
	UFUNCTION(BlueprintCallable, Category = "Certification")
	void OnUserSignedOut();

	UFUNCTION(BlueprintPure, Category = "Certification")
	bool IsUserSignedIn() const { return bUserSignedIn; }

protected:
	void DetectPlatform();
	void InitializeRequirements();

private:
	EMGPlatform CurrentPlatform = EMGPlatform::PC;
	EMGAgeRating CurrentAgeRating = EMGAgeRating::ESRB_Everyone;
	FMGContentDescriptor ContentDescriptors;
	TArray<FMGCertificationRequirement> Requirements;
	bool bNetworkAvailable = true;
	bool bUserSignedIn = true;
};
