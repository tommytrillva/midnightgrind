// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGVehicleVFXComponent.h"
#include "VFX/MGVFXSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

UMGVehicleVFXComponent::UMGVehicleVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Default wheel socket names
	WheelSocketNames.Add(FName("Wheel_FL"));
	WheelSocketNames.Add(FName("Wheel_FR"));
	WheelSocketNames.Add(FName("Wheel_RL"));
	WheelSocketNames.Add(FName("Wheel_RR"));
}

void UMGVehicleVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeTireStates();

	// Setup persistent effects
	if (WindParticlesSystem)
	{
		WindParticlesComp = SpawnAttachedNiagara(WindParticlesSystem, NAME_None);
		if (WindParticlesComp)
		{
			WindParticlesComp->Deactivate();
		}
	}

	if (RainInteractionSystem)
	{
		RainInteractionComp = SpawnAttachedNiagara(RainInteractionSystem, NAME_None);
		if (RainInteractionComp)
		{
			RainInteractionComp->Deactivate();
		}
	}
}

void UMGVehicleVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup all active components
	for (FMGTireVFXState& TireState : TireStates)
	{
		if (TireState.SmokeComponent)
		{
			TireState.SmokeComponent->DestroyComponent();
		}
		if (TireState.SkidmarkComponent)
		{
			TireState.SkidmarkComponent->DestroyComponent();
		}
	}

	for (UNiagaraComponent* Comp : ExhaustFlameComps)
	{
		if (Comp) Comp->DestroyComponent();
	}

	for (UNiagaraComponent* Comp : NOSFlameComps)
	{
		if (Comp) Comp->DestroyComponent();
	}

	for (UNiagaraComponent* Comp : NOSTrailComps)
	{
		if (Comp) Comp->DestroyComponent();
	}

	if (EngineSmokeComp) EngineSmokeComp->DestroyComponent();
	if (EngineFireComp) EngineFireComp->DestroyComponent();
	if (ScrapeSparksComp) ScrapeSparksComp->DestroyComponent();
	if (SpeedLinesComp) SpeedLinesComp->DestroyComponent();
	if (HeatDistortionComp) HeatDistortionComp->DestroyComponent();
	if (WindParticlesComp) WindParticlesComp->DestroyComponent();
	if (RainInteractionComp) RainInteractionComp->DestroyComponent();

	// Cleanup wear VFX components
	if (ClutchOverheatSmokeComp) ClutchOverheatSmokeComp->DestroyComponent();
	if (OilLeakComp) OilLeakComp->DestroyComponent();
	for (UNiagaraComponent* Comp : BrakeGlowComps)
	{
		if (Comp) Comp->DestroyComponent();
	}

	Super::EndPlay(EndPlayReason);
}

void UMGVehicleVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update tire effects
	for (int32 i = 0; i < TireStates.Num(); ++i)
	{
		UpdateTireSmoke(i, DeltaTime);
	}

	// Update other systems
	UpdateExhaustEffects(DeltaTime);
	UpdateDamageEffects(DeltaTime);
}

// ==========================================
// TIRE VFX
// ==========================================

void UMGVehicleVFXComponent::UpdateTireState(int32 WheelIndex, float SlipRatio, float SlipAngle, bool bOnGround, FName SurfaceType)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FMGTireVFXState& State = TireStates[WheelIndex];
	State.SurfaceType = SurfaceType;

	// Calculate combined slip
	float CombinedSlip = FMath::Sqrt(SlipRatio * SlipRatio + FMath::Sin(FMath::DegreesToRadians(SlipAngle)) * FMath::Sin(FMath::DegreesToRadians(SlipAngle)));
	State.SlipAmount = FMath::Clamp(CombinedSlip, 0.0f, 1.0f);

	// Determine if smoking (based on slip and surface)
	bool bShouldSmoke = bOnGround && CombinedSlip > TireSmokeSlipThreshold;

	// Only smoke on appropriate surfaces
	if (SurfaceType == FName("Dirt") || SurfaceType == FName("Grass"))
	{
		bShouldSmoke = bOnGround && CurrentSpeedKPH > 30.0f; // Dust instead of smoke
	}
	else if (SurfaceType == FName("Water") || SurfaceType == FName("Wet"))
	{
		bShouldSmoke = false; // Spray instead
	}

	// Determine if drifting (high slip angle)
	bool bShouldDrift = bOnGround && FMath::Abs(SlipAngle) > DriftTrailMinAngle && CurrentSpeedKPH > 40.0f;

	// Update state
	State.bIsSmoking = bShouldSmoke;
	State.bIsDrifting = bShouldDrift;

	// Update tire temperature (heat builds with slip, cools without)
	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	if (bShouldSmoke || bShouldDrift)
	{
		State.TireTemperature = FMath::Min(State.TireTemperature + TireHeatRate * State.SlipAmount * DeltaSeconds, 1.0f);
	}
	else
	{
		State.TireTemperature = FMath::Max(State.TireTemperature - TireCoolRate * DeltaSeconds, 0.0f);
	}
}

void UMGVehicleVFXComponent::StartBurnout(int32 WheelIndex)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FMGTireVFXState& State = TireStates[WheelIndex];

	// Use burnout system for thick smoke
	if (BurnoutSmokeSystem && !State.SmokeComponent)
	{
		FVector WheelLoc;
		FRotator WheelRot;
		if (GetWheelTransform(WheelIndex, WheelLoc, WheelRot))
		{
			State.SmokeComponent = SpawnAttachedNiagara(BurnoutSmokeSystem, WheelSocketNames[WheelIndex]);
		}
	}

	State.bIsSmoking = true;
	State.TireTemperature = 1.0f;
}

void UMGVehicleVFXComponent::StopBurnout(int32 WheelIndex)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FMGTireVFXState& State = TireStates[WheelIndex];

	if (State.SmokeComponent)
	{
		State.SmokeComponent->Deactivate();
	}

	State.bIsSmoking = false;
}

void UMGVehicleVFXComponent::SetDriftTrailColor(FLinearColor Color)
{
	DriftColor = Color;

	// Update active drift trail components
	for (FMGTireVFXState& State : TireStates)
	{
		if (State.SkidmarkComponent)
		{
			State.SkidmarkComponent->SetNiagaraVariableLinearColor(FString("TrailColor"), Color);
		}
	}
}

FMGTireVFXState UMGVehicleVFXComponent::GetTireState(int32 WheelIndex) const
{
	if (TireStates.IsValidIndex(WheelIndex))
	{
		return TireStates[WheelIndex];
	}
	return FMGTireVFXState();
}

// ==========================================
// EXHAUST VFX
// ==========================================

void UMGVehicleVFXComponent::SetExhaustConfigs(const TArray<FMGExhaustConfig>& Configs)
{
	// Cleanup old
	for (UNiagaraComponent* Comp : ExhaustFlameComps)
	{
		if (Comp) Comp->DestroyComponent();
	}
	ExhaustFlameComps.Empty();

	ExhaustConfigs = Configs;

	// Setup new exhaust flame components
	if (ExhaustFlameSystem)
	{
		for (const FMGExhaustConfig& Config : ExhaustConfigs)
		{
			if (Config.bEnabled)
			{
				UNiagaraComponent* Comp = SpawnAttachedNiagara(ExhaustFlameSystem, Config.SocketName, Config.Offset);
				if (Comp)
				{
					Comp->Deactivate(); // Start inactive, controlled by throttle
					ExhaustFlameComps.Add(Comp);
				}
			}
		}
	}
}

void UMGVehicleVFXComponent::TriggerBackfire()
{
	if (!BackfireSystem)
	{
		return;
	}

	// Spawn backfire at each exhaust
	for (const FMGExhaustConfig& Config : ExhaustConfigs)
	{
		if (Config.bEnabled)
		{
			FVector ExhaustLoc = FVector::ZeroVector;
			AActor* Owner = GetOwner();

			if (Owner)
			{
				USceneComponent* Root = Owner->GetRootComponent();
				if (Root)
				{
					if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Root))
					{
						ExhaustLoc = Mesh->GetSocketLocation(Config.SocketName) + Config.Offset;
					}
					else
					{
						ExhaustLoc = Root->GetComponentLocation() + Config.Offset;
					}
				}
			}

			// Spawn one-shot backfire
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				BackfireSystem,
				ExhaustLoc,
				Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator
			);
		}
	}

	// Notify VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		if (AActor* Owner = GetOwner())
		{
			VFXSub->TriggerVFXEvent(EMGVFXEvent::ExhaustBackfire, Owner->GetActorLocation(), Owner->GetActorRotation(), Owner);
		}
	}
}

void UMGVehicleVFXComponent::ActivateNOS()
{
	if (bNOSActive)
	{
		return;
	}

	bNOSActive = true;

	// Spawn NOS flames at each exhaust
	if (NOSFlameSystem)
	{
		for (const FMGExhaustConfig& Config : ExhaustConfigs)
		{
			if (Config.bEnabled)
			{
				UNiagaraComponent* Comp = SpawnAttachedNiagara(NOSFlameSystem, Config.SocketName, Config.Offset);
				if (Comp)
				{
					NOSFlameComps.Add(Comp);
				}
			}
		}
	}

	// Spawn NOS trails
	if (NOSTrailSystem)
	{
		for (const FMGExhaustConfig& Config : ExhaustConfigs)
		{
			if (Config.bEnabled)
			{
				UNiagaraComponent* Comp = SpawnAttachedNiagara(NOSTrailSystem, Config.SocketName, Config.Offset);
				if (Comp)
				{
					NOSTrailComps.Add(Comp);
				}
			}
		}
	}

	// Notify VFX subsystem for screen effects
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		if (AActor* Owner = GetOwner())
		{
			VFXSub->TriggerVFXEvent(EMGVFXEvent::NOSActivate, Owner->GetActorLocation(), Owner->GetActorRotation(), Owner);
		}
	}
}

void UMGVehicleVFXComponent::DeactivateNOS()
{
	if (!bNOSActive)
	{
		return;
	}

	bNOSActive = false;

	// Cleanup NOS components
	for (UNiagaraComponent* Comp : NOSFlameComps)
	{
		if (Comp)
		{
			Comp->Deactivate();
			Comp->DestroyComponent();
		}
	}
	NOSFlameComps.Empty();

	for (UNiagaraComponent* Comp : NOSTrailComps)
	{
		if (Comp)
		{
			Comp->Deactivate();
			Comp->DestroyComponent();
		}
	}
	NOSTrailComps.Empty();

	// Notify VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		if (AActor* Owner = GetOwner())
		{
			VFXSub->TriggerVFXEvent(EMGVFXEvent::NOSDeactivate, Owner->GetActorLocation(), Owner->GetActorRotation(), Owner);
		}
	}
}

void UMGVehicleVFXComponent::SetExhaustIntensity(float ThrottlePosition, float RPMNormalized)
{
	// Calculate flame intensity
	// High throttle + high RPM = visible flames
	// Lift-off at high RPM = potential backfire
	float FlameIntensity = ThrottlePosition * RPMNormalized;

	// Only show flames at higher intensities
	if (FlameIntensity > 0.7f)
	{
		for (UNiagaraComponent* Comp : ExhaustFlameComps)
		{
			if (Comp)
			{
				if (!Comp->IsActive())
				{
					Comp->Activate();
				}

				// Set flame parameters
				float NormalizedFlame = (FlameIntensity - 0.7f) / 0.3f;
				Comp->SetNiagaraVariableFloat(FString("FlameIntensity"), NormalizedFlame);
			}
		}
	}
	else
	{
		for (UNiagaraComponent* Comp : ExhaustFlameComps)
		{
			if (Comp && Comp->IsActive())
			{
				Comp->Deactivate();
			}
		}
	}

	// Random chance of backfire on lift-off at high RPM
	static float LastThrottle = 0.0f;
	if (LastThrottle > 0.8f && ThrottlePosition < 0.3f && RPMNormalized > 0.85f)
	{
		// Sudden lift-off at high RPM
		if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance
		{
			TriggerBackfire();
		}
	}
	LastThrottle = ThrottlePosition;
}

// ==========================================
// DAMAGE VFX
// ==========================================

void UMGVehicleVFXComponent::SetDamageState(const FMGVehicleDamageVFXState& DamageState)
{
	CurrentDamageState = DamageState;

	// Handle engine smoke
	if (DamageState.bEngineSmoking && !EngineSmokeComp && EngineSmokeSystem)
	{
		EngineSmokeComp = SpawnAttachedNiagara(EngineSmokeSystem, EngineSocketName);
	}
	else if (!DamageState.bEngineSmoking && EngineSmokeComp)
	{
		EngineSmokeComp->Deactivate();
		EngineSmokeComp->DestroyComponent();
		EngineSmokeComp = nullptr;
	}

	// Handle engine fire
	if (DamageState.bOnFire && !EngineFireComp && EngineFireSystem)
	{
		EngineFireComp = SpawnAttachedNiagara(EngineFireSystem, EngineSocketName);
	}
	else if (!DamageState.bOnFire && EngineFireComp)
	{
		EngineFireComp->Deactivate();
		EngineFireComp->DestroyComponent();
		EngineFireComp = nullptr;
	}

	// Update smoke intensity based on damage
	if (EngineSmokeComp)
	{
		float SmokeIntensity = FMath::Max(DamageState.FrontDamage, DamageState.OverallDamage);
		EngineSmokeComp->SetNiagaraVariableFloat(FString("SmokeIntensity"), SmokeIntensity);
	}

	// Update vehicle mesh materials with damage parameters
	// This enables shader-based scratches, dents, and dirt visualization
	if (AActor* Owner = GetOwner())
	{
		if (USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			// Update damage parameters on all materials
			for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
			{
				UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i));
				if (!DynMat)
				{
					// Create dynamic instance if needed
					UMaterialInterface* BaseMat = Mesh->GetMaterial(i);
					if (BaseMat)
					{
						DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
						Mesh->SetMaterial(i, DynMat);
					}
				}

				if (DynMat)
				{
					// Set zone damage parameters (shader should use these for deformation/scratches)
					DynMat->SetScalarParameterValue(FName("DamageOverall"), DamageState.OverallDamage);
					DynMat->SetScalarParameterValue(FName("DamageFront"), DamageState.FrontDamage);
					DynMat->SetScalarParameterValue(FName("DamageRear"), DamageState.RearDamage);
					DynMat->SetScalarParameterValue(FName("DamageLeft"), DamageState.LeftDamage);
					DynMat->SetScalarParameterValue(FName("DamageRight"), DamageState.RightDamage);

					// Dirt/grime buildup based on overall damage
					DynMat->SetScalarParameterValue(FName("DirtAmount"), DamageState.OverallDamage * 0.5f);
				}
			}
		}
	}
}

void UMGVehicleVFXComponent::TriggerCollisionImpact(FVector ImpactPoint, FVector ImpactNormal, float ImpactForce)
{
	// Spawn collision sparks
	if (CollisionSparksSystem)
	{
		FRotator SparkRotation = ImpactNormal.Rotation();

		UNiagaraComponent* SparksComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionSparksSystem,
			ImpactPoint,
			SparkRotation
		);

		if (SparksComp)
		{
			// Scale sparks based on impact force
			float SparkIntensity = FMath::Clamp(ImpactForce / 10000.0f, 0.2f, 1.0f);
			SparksComp->SetNiagaraVariableFloat(FString("SparkIntensity"), SparkIntensity);
		}
	}

	// Spawn debris for significant impacts
	if (ImpactForce > 5000.0f)
	{
		int32 DebrisCount = FMath::Clamp(FMath::FloorToInt(ImpactForce / 3000.0f), 1, 10);
		SpawnDebris(ImpactPoint, -ImpactNormal, DebrisCount);
	}

	// Notify VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		VFXSub->TriggerVFXEvent(EMGVFXEvent::CollisionImpact, ImpactPoint, ImpactNormal.Rotation(), GetOwner());
	}
}

void UMGVehicleVFXComponent::StartScrapeSparks(FVector ContactPoint, FVector Direction)
{
	if (!ScrapeSparksSystem)
	{
		return;
	}

	if (!ScrapeSparksComp)
	{
		ScrapeSparksComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ScrapeSparksSystem,
			ContactPoint,
			Direction.Rotation()
		);
	}
	else
	{
		ScrapeSparksComp->SetWorldLocation(ContactPoint);
		ScrapeSparksComp->SetWorldRotation(Direction.Rotation());

		if (!ScrapeSparksComp->IsActive())
		{
			ScrapeSparksComp->Activate();
		}
	}

	// Notify VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		VFXSub->TriggerVFXEvent(EMGVFXEvent::ScrapeStart, ContactPoint, Direction.Rotation(), GetOwner());
	}
}

void UMGVehicleVFXComponent::StopScrapeSparks()
{
	if (ScrapeSparksComp)
	{
		ScrapeSparksComp->Deactivate();
	}

	// Notify VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		VFXSub->TriggerVFXEvent(EMGVFXEvent::ScrapeEnd, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), GetOwner());
	}
}

void UMGVehicleVFXComponent::SpawnDebris(FVector Location, FVector Direction, int32 DebrisCount)
{
	if (!DebrisSystem)
	{
		return;
	}

	UNiagaraComponent* DebrisComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DebrisSystem,
		Location,
		Direction.Rotation()
	);

	if (DebrisComp)
	{
		DebrisComp->SetNiagaraVariableInt(FString("SpawnCount"), DebrisCount);
		DebrisComp->SetNiagaraVariableVec3(FString("LaunchDirection"), Direction);
	}
}

void UMGVehicleVFXComponent::SetHeadlightsBroken(bool bBroken)
{
	if (bHeadlightsBroken == bBroken)
	{
		return;
	}

	bHeadlightsBroken = bBroken;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh)
	{
		return;
	}

	// Update emissive parameter on materials
	for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i));
		if (DynMat)
		{
			// Set headlight emissive to 0 when broken
			DynMat->SetScalarParameterValue(HeadlightEmissiveParam, bBroken ? 0.0f : 1.0f);
		}
	}

	// Spawn glass debris when breaking
	if (bBroken)
	{
		for (const FName& SocketName : HeadlightSocketNames)
		{
			if (Mesh->DoesSocketExist(SocketName))
			{
				FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
				FVector Forward = Owner->GetActorForwardVector();

				// Spawn glass debris flying forward
				SpawnDebris(SocketLocation, Forward, 8);
			}
		}
	}
}

void UMGVehicleVFXComponent::SetTaillightsBroken(bool bBroken)
{
	if (bTaillightsBroken == bBroken)
	{
		return;
	}

	bTaillightsBroken = bBroken;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh)
	{
		return;
	}

	// Update emissive parameter on materials
	for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i));
		if (DynMat)
		{
			// Set taillight emissive to 0 when broken
			DynMat->SetScalarParameterValue(TaillightEmissiveParam, bBroken ? 0.0f : 1.0f);
		}
	}

	// Spawn glass debris when breaking
	if (bBroken)
	{
		for (const FName& SocketName : TaillightSocketNames)
		{
			if (Mesh->DoesSocketExist(SocketName))
			{
				FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
				FVector Backward = -Owner->GetActorForwardVector();

				// Spawn glass debris flying backward
				SpawnDebris(SocketLocation, Backward, 6);
			}
		}
	}
}

// ==========================================
// ENVIRONMENT INTERACTION
// ==========================================

void UMGVehicleVFXComponent::TriggerPuddleSplash(FVector Location, float Speed)
{
	if (!PuddleSplashSystem)
	{
		return;
	}

	UNiagaraComponent* SplashComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		PuddleSplashSystem,
		Location
	);

	if (SplashComp)
	{
		// Scale splash based on speed
		float SplashScale = FMath::Clamp(Speed / 100.0f, 0.5f, 2.0f);
		SplashComp->SetWorldScale3D(FVector(SplashScale));
		SplashComp->SetNiagaraVariableFloat(FString("SplashIntensity"), SplashScale);
	}
}

void UMGVehicleVFXComponent::TriggerDustCloud(FVector Location, float Intensity)
{
	if (!DustCloudSystem)
	{
		return;
	}

	UNiagaraComponent* DustComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DustCloudSystem,
		Location
	);

	if (DustComp)
	{
		DustComp->SetNiagaraVariableFloat(FString("DustIntensity"), Intensity);
	}
}

void UMGVehicleVFXComponent::TriggerDebrisScatter(FVector Location, FVector Direction)
{
	if (!DebrisScatterSystem)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DebrisScatterSystem,
		Location,
		Direction.Rotation()
	);
}

void UMGVehicleVFXComponent::SetInRain(bool bInRain)
{
	bIsInRain = bInRain;

	if (RainInteractionComp)
	{
		if (bInRain && !RainInteractionComp->IsActive())
		{
			RainInteractionComp->Activate();
		}
		else if (!bInRain && RainInteractionComp->IsActive())
		{
			RainInteractionComp->Deactivate();
		}
	}
}

// ==========================================
// SPEED EFFECTS
// ==========================================

void UMGVehicleVFXComponent::UpdateSpeedEffects(float SpeedKPH, float SpeedNormalized)
{
	CurrentSpeedKPH = SpeedKPH;
	CurrentSpeedNorm = SpeedNormalized;

	// Update VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		VFXSub->SetPlayerSpeed(SpeedNormalized);
	}

	// Speed lines effect
	if (bSpeedLinesEnabled && SpeedLinesSystem)
	{
		bool bShouldShowSpeedLines = SpeedKPH > SpeedEffectsThreshold;

		if (bShouldShowSpeedLines)
		{
			if (!SpeedLinesComp)
			{
				SpeedLinesComp = SpawnAttachedNiagara(SpeedLinesSystem, NAME_None);
			}

			if (SpeedLinesComp)
			{
				if (!SpeedLinesComp->IsActive())
				{
					SpeedLinesComp->Activate();
				}

				float SpeedLinesIntensity = (SpeedKPH - SpeedEffectsThreshold) / (200.0f - SpeedEffectsThreshold);
				SpeedLinesComp->SetNiagaraVariableFloat(FString("Intensity"), FMath::Clamp(SpeedLinesIntensity, 0.0f, 1.0f));
			}
		}
		else if (SpeedLinesComp && SpeedLinesComp->IsActive())
		{
			SpeedLinesComp->Deactivate();
		}
	}

	// Heat distortion (at very high speeds)
	if (bHeatDistortionEnabled && HeatDistortionSystem)
	{
		bool bShouldShowHeatDistortion = SpeedKPH > 180.0f;

		if (bShouldShowHeatDistortion)
		{
			if (!HeatDistortionComp)
			{
				HeatDistortionComp = SpawnAttachedNiagara(HeatDistortionSystem, NAME_None);
			}

			if (HeatDistortionComp && !HeatDistortionComp->IsActive())
			{
				HeatDistortionComp->Activate();
			}
		}
		else if (HeatDistortionComp && HeatDistortionComp->IsActive())
		{
			HeatDistortionComp->Deactivate();
		}
	}

	// Wind particles at speed
	if (WindParticlesComp)
	{
		bool bShouldShowWind = SpeedKPH > 80.0f;

		if (bShouldShowWind)
		{
			if (!WindParticlesComp->IsActive())
			{
				WindParticlesComp->Activate();
			}

			float WindIntensity = FMath::Clamp((SpeedKPH - 80.0f) / 120.0f, 0.0f, 1.0f);
			WindParticlesComp->SetNiagaraVariableFloat(FString("WindIntensity"), WindIntensity);
		}
		else if (WindParticlesComp->IsActive())
		{
			WindParticlesComp->Deactivate();
		}
	}

	// Top speed event
	if (SpeedNormalized >= 0.98f)
	{
		if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
		{
			VFXSub->TriggerVFXEvent(EMGVFXEvent::TopSpeed, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), GetOwner());
		}
	}
}

void UMGVehicleVFXComponent::SetSpeedLinesEnabled(bool bEnabled)
{
	bSpeedLinesEnabled = bEnabled;

	if (!bEnabled && SpeedLinesComp)
	{
		SpeedLinesComp->Deactivate();
	}
}

void UMGVehicleVFXComponent::SetHeatDistortionEnabled(bool bEnabled)
{
	bHeatDistortionEnabled = bEnabled;

	if (!bEnabled && HeatDistortionComp)
	{
		HeatDistortionComp->Deactivate();
	}
}

// ==========================================
// INTERNAL
// ==========================================

UMGVFXSubsystem* UMGVehicleVFXComponent::GetVFXSubsystem() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetSubsystem<UMGVFXSubsystem>();
	}
	return nullptr;
}

void UMGVehicleVFXComponent::InitializeTireStates()
{
	TireStates.Empty();
	TireStates.SetNum(WheelSocketNames.Num());
}

void UMGVehicleVFXComponent::UpdateTireSmoke(int32 WheelIndex, float DeltaTime)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FMGTireVFXState& State = TireStates[WheelIndex];

	// Handle tire smoke
	if (State.bIsSmoking)
	{
		if (!State.SmokeComponent && TireSmokeSystem)
		{
			State.SmokeComponent = SpawnAttachedNiagara(TireSmokeSystem, WheelSocketNames[WheelIndex]);
		}

		if (State.SmokeComponent)
		{
			if (!State.SmokeComponent->IsActive())
			{
				State.SmokeComponent->Activate();
			}

			// Update smoke parameters based on slip and temperature
			float SmokeIntensity = State.SlipAmount * (0.5f + 0.5f * State.TireTemperature);
			State.SmokeComponent->SetNiagaraVariableFloat(FString("SmokeIntensity"), SmokeIntensity);
			State.SmokeComponent->SetNiagaraVariableFloat(FString("TireTemperature"), State.TireTemperature);
		}
	}
	else if (State.SmokeComponent && State.SmokeComponent->IsActive())
	{
		State.SmokeComponent->Deactivate();
	}

	// Handle drift trails
	if (State.bIsDrifting)
	{
		if (!State.SkidmarkComponent && DriftTrailSystem)
		{
			State.SkidmarkComponent = SpawnAttachedNiagara(DriftTrailSystem, WheelSocketNames[WheelIndex]);
			if (State.SkidmarkComponent)
			{
				State.SkidmarkComponent->SetNiagaraVariableLinearColor(FString("TrailColor"), DriftColor);
			}
		}

		if (State.SkidmarkComponent && !State.SkidmarkComponent->IsActive())
		{
			State.SkidmarkComponent->Activate();
		}
	}
	else if (State.SkidmarkComponent && State.SkidmarkComponent->IsActive())
	{
		State.SkidmarkComponent->Deactivate();
	}
}

void UMGVehicleVFXComponent::UpdateExhaustEffects(float DeltaTime)
{
	// Exhaust effects are updated via SetExhaustIntensity called from vehicle
}

void UMGVehicleVFXComponent::UpdateDamageEffects(float DeltaTime)
{
	// Update smoke intensity over time based on damage
	if (EngineSmokeComp && EngineSmokeComp->IsActive())
	{
		// Add flicker/variation to smoke
		UWorld* World = GetWorld();
		float TimeSeconds = World ? World->GetTimeSeconds() : 0.0f;
		float FlickerAmount = FMath::PerlinNoise1D(TimeSeconds * 3.0f) * 0.2f;
		float BaseIntensity = FMath::Max(CurrentDamageState.FrontDamage, CurrentDamageState.OverallDamage);
		EngineSmokeComp->SetNiagaraVariableFloat(FString("SmokeIntensity"), BaseIntensity + FlickerAmount);
	}
}

UNiagaraComponent* UMGVehicleVFXComponent::SpawnAttachedNiagara(UNiagaraSystem* System, FName SocketName, FVector Offset)
{
	if (!System)
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	USceneComponent* AttachComponent = Owner->GetRootComponent();
	if (!AttachComponent)
	{
		return nullptr;
	}

	UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System,
		AttachComponent,
		SocketName,
		Offset,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);

	return Comp;
}

bool UMGVehicleVFXComponent::GetWheelTransform(int32 WheelIndex, FVector& OutLocation, FRotator& OutRotation) const
{
	if (!WheelSocketNames.IsValidIndex(WheelIndex))
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh)
	{
		FTransform SocketTransform = Mesh->GetSocketTransform(WheelSocketNames[WheelIndex]);
		OutLocation = SocketTransform.GetLocation();
		OutRotation = SocketTransform.Rotator();
		return true;
	}

	return false;
}

// ==========================================
// WEAR SYSTEM VFX IMPLEMENTATIONS
// ==========================================

void UMGVehicleVFXComponent::TriggerClutchOverheatSmoke(float Intensity)
{
	ClutchOverheatIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	if (ClutchOverheatIntensity > 0.0f)
	{
		// Spawn or update clutch smoke
		if (!ClutchOverheatSmokeComp && ClutchOverheatSmokeSystem)
		{
			// Spawn at bell housing area (between engine and transmission)
			ClutchOverheatSmokeComp = SpawnAttachedNiagara(ClutchOverheatSmokeSystem, EngineSocketName,
				FVector(-30.0f, 0.0f, -20.0f)); // Offset toward transmission
		}

		if (ClutchOverheatSmokeComp)
		{
			ClutchOverheatSmokeComp->Activate();
			ClutchOverheatSmokeComp->SetVariableFloat(FName("SmokeIntensity"), ClutchOverheatIntensity);

			// Smoke color gets darker/blacker as intensity increases (friction material burning)
			const float Darkness = FMath::Lerp(0.5f, 0.1f, ClutchOverheatIntensity);
			ClutchOverheatSmokeComp->SetVariableLinearColor(FName("SmokeColor"),
				FLinearColor(Darkness, Darkness, Darkness, 1.0f));
		}
	}
}

void UMGVehicleVFXComponent::StopClutchOverheatSmoke()
{
	ClutchOverheatIntensity = 0.0f;

	if (ClutchOverheatSmokeComp)
	{
		ClutchOverheatSmokeComp->Deactivate();
	}
}

void UMGVehicleVFXComponent::TriggerTireBlowout(int32 WheelIndex)
{
	FVector WheelLocation;
	FRotator WheelRotation;

	if (GetWheelTransform(WheelIndex, WheelLocation, WheelRotation))
	{
		// Spawn blowout debris and smoke burst
		if (TireBlowoutSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				TireBlowoutSystem,
				WheelLocation,
				WheelRotation,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease
			);
		}

		// Also spawn regular debris
		SpawnDebris(WheelLocation, WheelRotation.Vector() * -1.0f, 10);

		// Update tire state to show it's blown
		if (TireStates.IsValidIndex(WheelIndex))
		{
			TireStates[WheelIndex].bIsSmoking = true;
		}
	}
}

void UMGVehicleVFXComponent::SetBrakeGlowIntensity(int32 WheelIndex, float GlowIntensity)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	BrakeGlowIntensities[WheelIndex] = FMath::Clamp(GlowIntensity, 0.0f, 1.0f);

	// Initialize brake glow components if needed
	if (BrakeGlowComps.Num() < 4 && BrakeGlowSystem)
	{
		BrakeGlowComps.SetNum(4);
		for (int32 i = 0; i < 4; ++i)
		{
			if (WheelSocketNames.IsValidIndex(i))
			{
				BrakeGlowComps[i] = SpawnAttachedNiagara(BrakeGlowSystem, WheelSocketNames[i],
					FVector(5.0f, 0.0f, 0.0f)); // Offset slightly inward toward brake
				if (BrakeGlowComps[i])
				{
					BrakeGlowComps[i]->Deactivate();
				}
			}
		}
	}

	// Update the specific brake's glow
	if (BrakeGlowComps.IsValidIndex(WheelIndex) && BrakeGlowComps[WheelIndex])
	{
		if (GlowIntensity > 0.1f)
		{
			BrakeGlowComps[WheelIndex]->Activate();
			BrakeGlowComps[WheelIndex]->SetVariableFloat(FName("GlowIntensity"), GlowIntensity);

			// Color from dull red (low) to bright orange/yellow (high)
			FLinearColor GlowColor = FLinearColor::LerpUsingHSV(
				FLinearColor(0.8f, 0.2f, 0.0f), // Dull red
				FLinearColor(1.0f, 0.9f, 0.3f), // Bright yellow-white
				GlowIntensity
			);
			BrakeGlowComps[WheelIndex]->SetVariableLinearColor(FName("GlowColor"), GlowColor);
		}
		else
		{
			BrakeGlowComps[WheelIndex]->Deactivate();
		}
	}
}

void UMGVehicleVFXComponent::TriggerEngineDamageSmoke(int32 SmokeType)
{
	// 0 = light (oil), 1 = medium (coolant), 2 = heavy (failure)

	if (!EngineSmokeSystem)
	{
		return;
	}

	if (!EngineSmokeComp)
	{
		EngineSmokeComp = SpawnAttachedNiagara(EngineSmokeSystem, EngineSocketName);
	}

	if (EngineSmokeComp)
	{
		EngineSmokeComp->Activate();

		// Configure based on smoke type
		float Intensity = 0.0f;
		FLinearColor SmokeColor;

		switch (SmokeType)
		{
		case 0: // Light oil smoke - blue/gray tint
			Intensity = 0.3f;
			SmokeColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
			break;

		case 1: // Coolant steam - white
			Intensity = 0.6f;
			SmokeColor = FLinearColor(0.9f, 0.9f, 0.95f, 1.0f);
			break;

		case 2: // Heavy failure - dark black/gray
		default:
			Intensity = 1.0f;
			SmokeColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
			break;
		}

		EngineSmokeComp->SetVariableFloat(FName("SmokeIntensity"), Intensity);
		EngineSmokeComp->SetVariableLinearColor(FName("SmokeColor"), SmokeColor);
	}
}

void UMGVehicleVFXComponent::StopEngineDamageSmoke()
{
	if (EngineSmokeComp)
	{
		EngineSmokeComp->Deactivate();
	}
}

void UMGVehicleVFXComponent::TriggerTransmissionGrind()
{
	if (!TransmissionGrindSystem)
	{
		return;
	}

	// Spawn one-shot grind sparks at transmission location
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Transmission is roughly center-rear of engine
	FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * -50.0f;
	SpawnLocation.Z -= 30.0f; // Below the vehicle

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		TransmissionGrindSystem,
		SpawnLocation,
		Owner->GetActorRotation(),
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::AutoRelease
	);
}

void UMGVehicleVFXComponent::SetOilLeakRate(float LeakRate)
{
	CurrentOilLeakRate = FMath::Clamp(LeakRate, 0.0f, 1.0f);

	if (CurrentOilLeakRate > 0.0f)
	{
		if (!OilLeakComp && OilLeakSystem)
		{
			// Spawn at oil pan location (bottom center of engine)
			OilLeakComp = SpawnAttachedNiagara(OilLeakSystem, EngineSocketName,
				FVector(0.0f, 0.0f, -40.0f));
		}

		if (OilLeakComp)
		{
			OilLeakComp->Activate();
			OilLeakComp->SetVariableFloat(FName("DripRate"), CurrentOilLeakRate);
		}
	}
	else if (OilLeakComp)
	{
		OilLeakComp->Deactivate();
	}
}
