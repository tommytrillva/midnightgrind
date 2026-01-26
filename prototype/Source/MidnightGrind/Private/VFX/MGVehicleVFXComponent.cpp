// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGVehicleVFXComponent.h"
#include "VFX/MGVFXSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
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
	if (bShouldSmoke || bShouldDrift)
	{
		State.TireTemperature = FMath::Min(State.TireTemperature + TireHeatRate * State.SlipAmount * GetWorld()->GetDeltaSeconds(), 1.0f);
	}
	else
	{
		State.TireTemperature = FMath::Max(State.TireTemperature - TireCoolRate * GetWorld()->GetDeltaSeconds(), 0.0f);
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
		VFXSub->TriggerVFXEvent(EMGVFXEvent::ExhaustBackfire, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), GetOwner());
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
		VFXSub->TriggerVFXEvent(EMGVFXEvent::NOSActivate, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), GetOwner());
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
		VFXSub->TriggerVFXEvent(EMGVFXEvent::NOSDeactivate, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), GetOwner());
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
		float FlickerAmount = FMath::PerlinNoise1D(GetWorld()->GetTimeSeconds() * 3.0f) * 0.2f;
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
