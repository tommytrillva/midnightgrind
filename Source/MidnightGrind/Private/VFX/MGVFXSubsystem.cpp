// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGVFXSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

void UMGVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize pool
	VFXPool.Empty();
	ActiveVFXCount = 0;

	UE_LOG(LogTemp, Log, TEXT("MGVFXSubsystem initialized"));
}

void UMGVFXSubsystem::Deinitialize()
{
	// Cleanup all pooled components
	for (auto& Pair : VFXPool)
	{
		for (FMGPooledVFX& PooledVFX : Pair.Value)
		{
			if (PooledVFX.Component)
			{
				PooledVFX.Component->DestroyComponent();
			}
		}
	}
	VFXPool.Empty();

	Super::Deinitialize();
}

void UMGVFXSubsystem::Tick(float MGDeltaTime)
{
	// Pool cleanup
	TimeSinceCleanup += DeltaTime;
	if (TimeSinceCleanup >= PoolCleanupInterval)
	{
		CleanupPool();
		TimeSinceCleanup = 0.0f;
	}

	// Update screen effects
	UpdateScreenEffects(DeltaTime);

	// Update material parameters
	UpdateMaterialParams();
}

bool UMGVFXSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ==========================================
// VFX SPAWNING
// ==========================================

UNiagaraComponent* UMGVFXSubsystem::SpawnVFX(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
	if (!System)
	{
		return nullptr;
	}

	// Check if we can spawn more
	if (ActiveVFXCount >= GetMaxActiveForQuality())
	{
		return nullptr;
	}

	// Try to get pooled component
	UNiagaraComponent* Comp = GetPooledComponent(System);

	if (Comp)
	{
		Comp->SetWorldLocation(Location);
		Comp->SetWorldRotation(Rotation);
		Comp->Activate(true);
		ActiveVFXCount++;
	}
	else
	{
		// Create new
		Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			System,
			Location,
			Rotation,
			FVector::OneVector,
			true,
			true
		);

		if (Comp)
		{
			ActiveVFXCount++;
		}
	}

	return Comp;
}

UNiagaraComponent* UMGVFXSubsystem::SpawnVFXAttached(UNiagaraSystem* System, AActor* AttachTo, FName SocketName)
{
	if (!System || !AttachTo)
	{
		return nullptr;
	}

	if (ActiveVFXCount >= GetMaxActiveForQuality())
	{
		return nullptr;
	}

	USceneComponent* AttachComponent = AttachTo->GetRootComponent();
	if (!AttachComponent)
	{
		return nullptr;
	}

	UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System,
		AttachComponent,
		SocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);

	if (Comp)
	{
		ActiveVFXCount++;
	}

	return Comp;
}

UNiagaraComponent* UMGVFXSubsystem::SpawnVFXFromRequest(const FMGVFXSpawnRequest& Request)
{
	if (!Request.System)
	{
		return nullptr;
	}

	// Check priority vs quality
	if (!ShouldSpawnAtQuality(Request.Priority))
	{
		return nullptr;
	}

	UNiagaraComponent* Comp = nullptr;

	if (Request.AttachToActor)
	{
		Comp = SpawnVFXAttached(Request.System, Request.AttachToActor, Request.AttachSocketName);
	}
	else
	{
		Comp = SpawnVFX(Request.System, Request.Location, Request.Rotation);
	}

	if (Comp)
	{
		Comp->SetWorldScale3D(Request.Scale);
	}

	return Comp;
}

void UMGVFXSubsystem::ReturnToPool(UNiagaraComponent* Component)
{
	if (!Component)
	{
		return;
	}

	// Find in pool and mark as available
	for (auto& Pair : VFXPool)
	{
		for (FMGPooledVFX& PooledVFX : Pair.Value)
		{
			if (PooledVFX.Component == Component)
			{
				Component->Deactivate();
				PooledVFX.bInUse = false;
				PooledVFX.LastUsedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
				ActiveVFXCount = FMath::Max(0, ActiveVFXCount - 1);
				return;
			}
		}
	}

	// Not in pool, destroy
	Component->DestroyComponent();
	ActiveVFXCount = FMath::Max(0, ActiveVFXCount - 1);
}

// ==========================================
// VFX EVENTS
// ==========================================

void UMGVFXSubsystem::TriggerVFXEvent(EMGVFXEvent Event, FVector Location, FRotator Rotation, AActor* Context)
{
	// Check if we have a system registered for this event
	UNiagaraSystem** SystemPtr = EventVFXMap.Find(Event);
	if (SystemPtr && *SystemPtr)
	{
		int32 Priority = EventPriorities.Contains(Event) ? EventPriorities[Event] : 0;

		if (ShouldSpawnAtQuality(Priority))
		{
			if (Context)
			{
				SpawnVFXAttached(*SystemPtr, Context, NAME_None);
			}
			else
			{
				SpawnVFX(*SystemPtr, Location, Rotation);
			}
		}
	}

	// Additional hardcoded behaviors based on event type
	switch (Event)
	{
	case EMGVFXEvent::CollisionImpact:
		TriggerScreenShake(0.5f, 0.3f, true);
		break;

	case EMGVFXEvent::NOSActivate:
		SetChromaticAberration(0.3f);
		SetRadialBlur(0.2f, FVector2D(0.5f, 0.5f));
		break;

	case EMGVFXEvent::NOSDeactivate:
		SetChromaticAberration(0.0f);
		SetRadialBlur(0.0f, FVector2D(0.5f, 0.5f));
		break;

	case EMGVFXEvent::TopSpeed:
		SetRadialBlur(0.15f, FVector2D(0.5f, 0.5f));
		break;

	case EMGVFXEvent::FinishLine:
		FlashScreen(FLinearColor::White, 0.3f, 0.5f);
		break;

	case EMGVFXEvent::NearMiss:
		TriggerScreenShake(0.2f, 0.15f, true);
		FlashScreen(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f), 0.1f, 0.3f);
		break;

	case EMGVFXEvent::FinalLap:
		FlashScreen(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f), 0.2f, 0.4f);
		break;

	case EMGVFXEvent::PositionChange:
		TriggerScreenShake(0.15f, 0.1f, false);
		break;

	default:
		break;
	}

	// Broadcast event
	OnVFXEventTriggered.Broadcast(Event, Location, Context);
}

void UMGVFXSubsystem::RegisterEventVFX(EMGVFXEvent Event, UNiagaraSystem* System, int32 Priority)
{
	EventVFXMap.Add(Event, System);
	EventPriorities.Add(Event, Priority);
}

// ==========================================
// GLOBAL PARAMETERS
// ==========================================

void UMGVFXSubsystem::SetGlobalParams(const FMGGlobalVFXParams& Params)
{
	GlobalParams = Params;
	UpdateMaterialParams();
}

void UMGVFXSubsystem::SetRaceIntensity(float Intensity)
{
	GlobalParams.RaceIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGVFXSubsystem::SetPlayerSpeed(float SpeedNorm)
{
	GlobalParams.PlayerSpeedNorm = FMath::Clamp(SpeedNorm, 0.0f, 1.0f);

	// Auto-adjust radial blur based on speed
	if (CurrentQuality >= EMGVFXQuality::Medium)
	{
		float BlurIntensity = FMath::Lerp(0.0f, 0.1f, FMath::Pow(SpeedNorm, 2.0f));
		SetRadialBlur(BlurIntensity, FVector2D(0.5f, 0.5f));
	}
}

void UMGVFXSubsystem::SetCrewColor(FLinearColor Color)
{
	GlobalParams.CrewColor = Color;
}

// ==========================================
// QUALITY SETTINGS
// ==========================================

void UMGVFXSubsystem::SetQuality(EMGVFXQuality Quality)
{
	CurrentQuality = Quality;

	// Cull low-priority active effects if quality decreased
	// (Simplified - full implementation would track and cull)
}

float UMGVFXSubsystem::GetParticleCountMultiplier() const
{
	switch (CurrentQuality)
	{
	case EMGVFXQuality::Low:
		return 0.25f;
	case EMGVFXQuality::Medium:
		return 0.5f;
	case EMGVFXQuality::High:
		return 1.0f;
	case EMGVFXQuality::Ultra:
		return 1.5f;
	default:
		return 1.0f;
	}
}

bool UMGVFXSubsystem::ShouldSpawnAtQuality(int32 Priority) const
{
	// Priority 0 = always spawn
	// Priority 1 = Medium+
	// Priority 2 = High+
	// Priority 3 = Ultra only

	switch (CurrentQuality)
	{
	case EMGVFXQuality::Low:
		return Priority <= 0;
	case EMGVFXQuality::Medium:
		return Priority <= 1;
	case EMGVFXQuality::High:
		return Priority <= 2;
	case EMGVFXQuality::Ultra:
		return true;
	default:
		return Priority <= 1;
	}
}

// ==========================================
// SCREEN EFFECTS
// ==========================================

void UMGVFXSubsystem::TriggerScreenShake(float Intensity, float Duration, bool bFalloff)
{
	if (CurrentQuality == EMGVFXQuality::Low)
	{
		return; // Skip screen shake on low
	}

	CurrentShakeIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	ShakeDuration = Duration;
	ShakeTimer = 0.0f;
	bShakeFalloff = bFalloff;

	// Apply to player controller camera
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		// Use camera shake if available, otherwise use simpler approach
		// For now, we'll handle in UpdateScreenEffects
	}
}

void UMGVFXSubsystem::SetChromaticAberration(float Intensity)
{
	if (CurrentQuality == EMGVFXQuality::Low)
	{
		return;
	}

	// This would typically set a post-process parameter
	// Implementation depends on post-process setup
}

void UMGVFXSubsystem::SetRadialBlur(float Intensity, FVector2D Center)
{
	if (CurrentQuality == EMGVFXQuality::Low)
	{
		return;
	}

	// Would set radial blur material parameters
}

void UMGVFXSubsystem::SetVignette(float Intensity)
{
	// Set vignette intensity in post-process
}

void UMGVFXSubsystem::FlashScreen(FLinearColor Color, float Duration, float Intensity)
{
	FlashColor = Color;
	FlashDuration = Duration;
	FlashTimer = 0.0f;
	FlashIntensity = Intensity;
}

// ==========================================
// INTERNAL
// ==========================================

UNiagaraComponent* UMGVFXSubsystem::GetPooledComponent(UNiagaraSystem* System)
{
	if (!System)
	{
		return nullptr;
	}

	// Check if we have a pool for this system
	TArray<FMGPooledVFX>* Pool = VFXPool.Find(System);

	if (Pool)
	{
		// Find available component
		for (FMGPooledVFX& PooledVFX : *Pool)
		{
			if (!PooledVFX.bInUse && PooledVFX.Component)
			{
				PooledVFX.bInUse = true;
				PooledVFX.LastUsedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
				return PooledVFX.Component;
			}
		}

		// Pool full, check if we can add more
		if (Pool->Num() < MaxPooledPerSystem)
		{
			return CreatePooledComponent(System, *Pool);
		}

		// Pool at max, return nullptr to spawn non-pooled
		return nullptr;
	}
	else
	{
		// Create new pool
		TArray<FMGPooledVFX>& NewPool = VFXPool.Add(System);
		return CreatePooledComponent(System, NewPool);
	}
}

UNiagaraComponent* UMGVFXSubsystem::CreatePooledComponent(UNiagaraSystem* System, TArray<FMGPooledVFX>& Pool)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UNiagaraComponent* NewComp = NewObject<UNiagaraComponent>(World);
	if (NewComp)
	{
		NewComp->SetAsset(System);
		NewComp->bAutoActivate = false;
		NewComp->RegisterComponent();

		FMGPooledVFX PooledVFX;
		PooledVFX.Component = NewComp;
		PooledVFX.bInUse = true;
		PooledVFX.LastUsedTime = World->GetTimeSeconds();

		Pool.Add(PooledVFX);
	}

	return NewComp;
}

void UMGVFXSubsystem::CleanupPool()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	float MaxIdleTime = 60.0f; // Destroy components unused for 60 seconds

	for (auto& Pair : VFXPool)
	{
		TArray<FMGPooledVFX>& Pool = Pair.Value;

		// Remove old unused components
		for (int32 i = Pool.Num() - 1; i >= 0; --i)
		{
			FMGPooledVFX& PooledVFX = Pool[i];

			if (!PooledVFX.bInUse && (CurrentTime - PooledVFX.LastUsedTime) > MaxIdleTime)
			{
				if (PooledVFX.Component)
				{
					PooledVFX.Component->DestroyComponent();
				}
				Pool.RemoveAt(i);
			}
		}
	}

	// Remove empty pools
	for (auto It = VFXPool.CreateIterator(); It; ++It)
	{
		if (It.Value().Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
}

void UMGVFXSubsystem::UpdateMaterialParams()
{
	if (!GlobalParamCollection)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(GlobalParamCollection);
	if (!Instance)
	{
		return;
	}

	// Set scalar parameters
	Instance->SetScalarParameterValue(FName("RaceIntensity"), GlobalParams.RaceIntensity);
	Instance->SetScalarParameterValue(FName("PlayerSpeed"), GlobalParams.PlayerSpeedNorm);
	Instance->SetScalarParameterValue(FName("TimeOfDay"), GlobalParams.TimeOfDay);
	Instance->SetScalarParameterValue(FName("WeatherIntensity"), GlobalParams.WeatherIntensity);

	// Set vector parameters
	Instance->SetVectorParameterValue(FName("CrewColor"), GlobalParams.CrewColor);

	// Boolean as scalar
	Instance->SetScalarParameterValue(FName("PlayerInFirst"), GlobalParams.bPlayerInFirst ? 1.0f : 0.0f);
	Instance->SetScalarParameterValue(FName("FinalLap"), GlobalParams.bFinalLap ? 1.0f : 0.0f);
}

void UMGVFXSubsystem::UpdateScreenEffects(float MGDeltaTime)
{
	// Update screen shake
	if (ShakeTimer < ShakeDuration)
	{
		ShakeTimer += DeltaTime;

		float ShakeProgress = ShakeTimer / ShakeDuration;
		float CurrentShake = CurrentShakeIntensity;

		if (bShakeFalloff)
		{
			CurrentShake *= (1.0f - ShakeProgress);
		}

		// Apply shake to camera (would connect to camera manager)
		if (CurrentShake > 0.0f)
		{
			// Random offset based on shake intensity
			FVector ShakeOffset;
			ShakeOffset.X = FMath::RandRange(-1.0f, 1.0f) * CurrentShake * 5.0f;
			ShakeOffset.Y = FMath::RandRange(-1.0f, 1.0f) * CurrentShake * 5.0f;
			ShakeOffset.Z = FMath::RandRange(-1.0f, 1.0f) * CurrentShake * 2.0f;

			// This would be applied to camera component
		}
	}

	// Update screen flash
	if (FlashTimer < FlashDuration)
	{
		FlashTimer += DeltaTime;

		float FlashProgress = FlashTimer / FlashDuration;
		float CurrentFlash = FlashIntensity * (1.0f - FlashProgress);

		// Apply flash (would set HUD or post-process parameter)
	}
}

int32 UMGVFXSubsystem::GetMaxActiveForQuality() const
{
	switch (CurrentQuality)
	{
	case EMGVFXQuality::Low:
		return MaxActiveVFX / 4;
	case EMGVFXQuality::Medium:
		return MaxActiveVFX / 2;
	case EMGVFXQuality::High:
		return MaxActiveVFX;
	case EMGVFXQuality::Ultra:
		return MaxActiveVFX * 2;
	default:
		return MaxActiveVFX;
	}
}
