// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleConfigApplicator.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Material parameter names
const FName UMGVehicleConfigApplicator::BaseColorParam = TEXT("BaseColor");
const FName UMGVehicleConfigApplicator::SecondaryColorParam = TEXT("SecondaryColor");
const FName UMGVehicleConfigApplicator::MetallicParam = TEXT("Metallic");
const FName UMGVehicleConfigApplicator::RoughnessParam = TEXT("Roughness");
const FName UMGVehicleConfigApplicator::ClearCoatParam = TEXT("ClearCoat");
const FName UMGVehicleConfigApplicator::PearlColorParam = TEXT("PearlShiftColor");

UMGVehicleConfigApplicator::UMGVehicleConfigApplicator()
{
}

// ==========================================
// FULL CONFIG APPLICATION
// ==========================================

bool UMGVehicleConfigApplicator::ApplyFullConfig(AMGVehiclePawn* Vehicle, const FMGVehicleConfig& Config)
{
	if (!Vehicle)
	{
		return false;
	}

	// Apply paint
	ApplyPaint(Vehicle, Config.Paint);

	// Apply vinyls
	ApplyVinyls(Vehicle, Config.Vinyls);

	// Apply tuning
	ApplyTuning(Vehicle, Config.Tuning);

	// Apply parts
	ApplyParts(Vehicle, Config.InstalledParts);

	// Apply wheels
	ApplyWheels(Vehicle, Config.WheelID, Config.WheelColor);

	// Apply visual customization
	ApplyWindowTint(Vehicle, Config.WindowTint);
	ApplyLightColors(Vehicle, Config.HeadlightColor, Config.TaillightColor);
	ApplyUnderglow(Vehicle, Config.UnderglowColor);
	ApplyLicensePlate(Vehicle, Config.LicensePlate);

	OnConfigApplied.Broadcast(Vehicle, true);

	return true;
}

void UMGVehicleConfigApplicator::ResetToStock(AMGVehiclePawn* Vehicle)
{
	if (!Vehicle)
	{
		return;
	}

	// Reset to default config
	FMGVehicleConfig StockConfig;
	StockConfig.Paint.PaintType = EMGPaintType::Solid;
	StockConfig.Paint.PrimaryColor = FLinearColor::White;
	StockConfig.Tuning = FMGTuningConfig();
	StockConfig.Vinyls.Empty();
	StockConfig.InstalledParts.Empty();
	StockConfig.WindowTint = 0.0f;
	StockConfig.HeadlightColor = FLinearColor::White;
	StockConfig.TaillightColor = FLinearColor::Red;
	StockConfig.UnderglowColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);

	ApplyFullConfig(Vehicle, StockConfig);
}

FMGVehicleConfig UMGVehicleConfigApplicator::GetCurrentConfig(AMGVehiclePawn* Vehicle) const
{
	FMGVehicleConfig Config;

	if (!Vehicle)
	{
		return Config;
	}

	// Get paint config
	Config.Paint = GetCurrentPaint(Vehicle);

	// Get tuning config
	Config.Tuning = GetCurrentTuning(Vehicle);

	// Get vinyls
	Config.Vinyls = CachedVinyls;

	return Config;
}

// ==========================================
// PAINT APPLICATION
// ==========================================

void UMGVehicleConfigApplicator::ApplyPaint(AMGVehiclePawn* Vehicle, const FMGPaintConfig& PaintConfig)
{
	if (!Vehicle)
	{
		return;
	}

	UMaterialInstanceDynamic* BodyMaterial = GetOrCreateBodyMaterial(Vehicle);
	if (BodyMaterial)
	{
		SetPaintMaterialParameters(BodyMaterial, PaintConfig);
	}

	OnPaintChanged.Broadcast(Vehicle, PaintConfig);
}

void UMGVehicleConfigApplicator::ApplyColor(AMGVehiclePawn* Vehicle, FLinearColor Color, EMGPaintType PaintType)
{
	FMGPaintConfig Config;
	Config.PrimaryColor = Color;
	Config.PaintType = PaintType;

	switch (PaintType)
	{
	case EMGPaintType::Metallic:
		Config.MetallicIntensity = 0.8f;
		Config.ClearCoatIntensity = 0.7f;
		break;
	case EMGPaintType::Matte:
		Config.MetallicIntensity = 0.0f;
		Config.ClearCoatIntensity = 0.0f;
		break;
	case EMGPaintType::Chrome:
		Config.MetallicIntensity = 1.0f;
		Config.ClearCoatIntensity = 1.0f;
		break;
	case EMGPaintType::Pearlescent:
		Config.MetallicIntensity = 0.6f;
		Config.ClearCoatIntensity = 0.8f;
		Config.PearlShiftColor = FLinearColor(Color.B, Color.R, Color.G); // Shift hue
		break;
	default:
		Config.MetallicIntensity = 0.3f;
		Config.ClearCoatIntensity = 0.5f;
		break;
	}

	ApplyPaint(Vehicle, Config);
}

FMGPaintConfig UMGVehicleConfigApplicator::GetCurrentPaint(AMGVehiclePawn* Vehicle) const
{
	FMGPaintConfig Config;

	if (!Vehicle)
	{
		return Config;
	}

	// Try to read from material
	if (USkeletalMeshComponent* Mesh = Vehicle->GetMesh())
	{
		if (UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
		{
			Material->GetVectorParameterValue(BaseColorParam, Config.PrimaryColor);
			Material->GetScalarParameterValue(MetallicParam, Config.MetallicIntensity);
			Material->GetScalarParameterValue(ClearCoatParam, Config.ClearCoatIntensity);
		}
	}

	return Config;
}

// ==========================================
// VINYL/DECAL APPLICATION
// ==========================================

void UMGVehicleConfigApplicator::ApplyVinyls(AMGVehiclePawn* Vehicle, const TArray<FMGVinylLayer>& Vinyls)
{
	if (!Vehicle)
	{
		return;
	}

	CachedVinyls = Vinyls;

	// Clear existing decals
	// Vehicle->ClearDecalComponents();

	// Apply each vinyl layer
	for (int32 i = 0; i < Vinyls.Num(); ++i)
	{
		const FMGVinylLayer& Vinyl = Vinyls[i];
		if (!Vinyl.bVisible || Vinyl.VinylID.IsNone())
		{
			continue;
		}

		// Load vinyl texture
		// Create decal component
		// Position and scale decal
		// Apply color tint

		// For mirror mode, create opposite side
		if (Vinyl.bMirrored && (Vinyl.Placement == 0 || Vinyl.Placement == 1))
		{
			// Create mirrored decal on opposite side
		}
	}
}

int32 UMGVehicleConfigApplicator::AddVinyl(AMGVehiclePawn* Vehicle, const FMGVinylLayer& Vinyl)
{
	if (!Vehicle)
	{
		return -1;
	}

	CachedVinyls.Add(Vinyl);
	ApplyVinyls(Vehicle, CachedVinyls);

	return CachedVinyls.Num() - 1;
}

void UMGVehicleConfigApplicator::UpdateVinyl(AMGVehiclePawn* Vehicle, int32 LayerIndex, const FMGVinylLayer& Vinyl)
{
	if (!Vehicle || !CachedVinyls.IsValidIndex(LayerIndex))
	{
		return;
	}

	CachedVinyls[LayerIndex] = Vinyl;
	ApplyVinyls(Vehicle, CachedVinyls);
}

void UMGVehicleConfigApplicator::RemoveVinyl(AMGVehiclePawn* Vehicle, int32 LayerIndex)
{
	if (!Vehicle || !CachedVinyls.IsValidIndex(LayerIndex))
	{
		return;
	}

	CachedVinyls.RemoveAt(LayerIndex);
	ApplyVinyls(Vehicle, CachedVinyls);
}

void UMGVehicleConfigApplicator::ClearAllVinyls(AMGVehiclePawn* Vehicle)
{
	if (!Vehicle)
	{
		return;
	}

	CachedVinyls.Empty();
	ApplyVinyls(Vehicle, CachedVinyls);
}

// ==========================================
// TUNING APPLICATION
// ==========================================

void UMGVehicleConfigApplicator::ApplyTuning(AMGVehiclePawn* Vehicle, const FMGTuningConfig& TuningConfig)
{
	if (!Vehicle)
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = Vehicle->GetVehicleMovementComponent();
	if (Movement)
	{
		ApplyTuningToMovement(Movement, TuningConfig);
	}

	OnTuningChanged.Broadcast(Vehicle, TuningConfig);
}

void UMGVehicleConfigApplicator::ApplyTuningValue(AMGVehiclePawn* Vehicle, FName ParameterName, float Value)
{
	if (!Vehicle)
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = Vehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	// Apply individual tuning parameter
	FString ParamStr = ParameterName.ToString();

	if (ParamStr == TEXT("PowerAdjust"))
	{
		// Adjust engine power
		float BasePower = Movement->GetMaxEnginePower();
		Movement->SetMaxEnginePower(BasePower * (1.0f + Value));
	}
	else if (ParamStr == TEXT("RideHeight"))
	{
		// Adjust suspension
		// Movement->SetSuspensionOffset(Value * 10.0f); // cm adjustment
	}
	else if (ParamStr == TEXT("SpringStiffness"))
	{
		// float BaseStiffness = Movement->GetSuspensionStiffness();
		// Movement->SetSuspensionStiffness(BaseStiffness * (1.0f + Value * 0.5f));
	}
	else if (ParamStr == TEXT("BrakeBias"))
	{
		Movement->SetBrakeBias(Value);
	}
	else if (ParamStr == TEXT("DiffLock"))
	{
		Movement->SetDifferentialLockRatio(Value);
	}
	else if (ParamStr == TEXT("DownforceRear"))
	{
		Movement->SetDownforceCoefficient(Value * 0.5f);
	}
}

void UMGVehicleConfigApplicator::ResetTuning(AMGVehiclePawn* Vehicle)
{
	FMGTuningConfig DefaultTuning;
	ApplyTuning(Vehicle, DefaultTuning);
}

FMGTuningConfig UMGVehicleConfigApplicator::GetCurrentTuning(AMGVehiclePawn* Vehicle) const
{
	FMGTuningConfig Config;

	if (!Vehicle)
	{
		return Config;
	}

	UMGVehicleMovementComponent* Movement = Vehicle->GetVehicleMovementComponent();
	if (Movement)
	{
		// Read current values from movement component
		Config.BrakeBias = Movement->GetBrakeBias();
		Config.DiffLock = Movement->GetDifferentialLockRatio();
		// ... etc
	}

	return Config;
}

// ==========================================
// PARTS APPLICATION
// ==========================================

void UMGVehicleConfigApplicator::ApplyParts(AMGVehiclePawn* Vehicle, const TArray<FMGInstalledPart>& Parts)
{
	if (!Vehicle)
	{
		return;
	}

	// Calculate stat modifiers from all parts
	CalculatePartModifiers(Parts);

	// Apply visual parts (body kits, spoilers, etc.)
	for (const FMGInstalledPart& Part : Parts)
	{
		// Load part mesh data
		// Attach to appropriate socket/bone
		// Apply condition-based wear visuals
	}
}

void UMGVehicleConfigApplicator::ApplyWheels(AMGVehiclePawn* Vehicle, FName WheelID, FLinearColor WheelColor)
{
	if (!Vehicle || WheelID.IsNone())
	{
		return;
	}

	// Load wheel mesh asset
	// Apply to all wheel components
	// Set wheel material color
}

// ==========================================
// VISUAL CUSTOMIZATION
// ==========================================

void UMGVehicleConfigApplicator::ApplyWindowTint(AMGVehiclePawn* Vehicle, float TintAmount)
{
	if (!Vehicle)
	{
		return;
	}

	// Find window material
	// Adjust opacity parameter
	// TintAmount: 0 = clear, 1 = fully tinted (limo)
}

void UMGVehicleConfigApplicator::ApplyLightColors(AMGVehiclePawn* Vehicle, FLinearColor HeadlightColor, FLinearColor TaillightColor)
{
	if (!Vehicle)
	{
		return;
	}

	// Find headlight components
	// Vehicle->SetHeadlightColor(HeadlightColor);
	// Vehicle->SetTaillightColor(TaillightColor);
}

void UMGVehicleConfigApplicator::ApplyUnderglow(AMGVehiclePawn* Vehicle, FLinearColor Color)
{
	if (!Vehicle)
	{
		return;
	}

	// Enable/disable underglow based on alpha
	bool bEnabled = Color.A > 0.01f;

	// Vehicle->SetUnderglowEnabled(bEnabled);
	// if (bEnabled)
	// {
	//     Vehicle->SetUnderglowColor(Color);
	// }
}

void UMGVehicleConfigApplicator::ApplyLicensePlate(AMGVehiclePawn* Vehicle, const FString& PlateText)
{
	if (!Vehicle)
	{
		return;
	}

	// Find license plate text render component
	// Update text
	// Vehicle->SetLicensePlateText(PlateText);
}

// ==========================================
// PREVIEW MODE
// ==========================================

void UMGVehicleConfigApplicator::BeginPreview(AMGVehiclePawn* Vehicle)
{
	if (!Vehicle || bPreviewMode)
	{
		return;
	}

	// Cache current config
	PreviewCachedConfig = GetCurrentConfig(Vehicle);
	PreviewVehicle = Vehicle;
	bPreviewMode = true;
}

void UMGVehicleConfigApplicator::EndPreview(AMGVehiclePawn* Vehicle, bool bApplyChanges)
{
	if (!bPreviewMode || Vehicle != PreviewVehicle.Get())
	{
		return;
	}

	if (!bApplyChanges)
	{
		// Restore cached config
		ApplyFullConfig(Vehicle, PreviewCachedConfig);
	}

	bPreviewMode = false;
	PreviewVehicle.Reset();
}

// ==========================================
// INTERNAL METHODS
// ==========================================

UMaterialInstanceDynamic* UMGVehicleConfigApplicator::GetOrCreateBodyMaterial(AMGVehiclePawn* Vehicle)
{
	if (!Vehicle)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh = Vehicle->GetMesh();
	if (!Mesh)
	{
		return nullptr;
	}

	// Check if already a dynamic instance
	UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));

	if (!DynMaterial)
	{
		// Create dynamic instance from base material
		UMaterialInterface* BaseMaterial = Mesh->GetMaterial(0);
		if (BaseMaterial)
		{
			DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Vehicle);
			Mesh->SetMaterial(0, DynMaterial);
		}
	}

	return DynMaterial;
}

void UMGVehicleConfigApplicator::SetPaintMaterialParameters(UMaterialInstanceDynamic* Material, const FMGPaintConfig& Paint)
{
	if (!Material)
	{
		return;
	}

	// Base color
	Material->SetVectorParameterValue(BaseColorParam, Paint.PrimaryColor);

	// Secondary color for two-tone
	if (Paint.bTwoTone)
	{
		Material->SetVectorParameterValue(SecondaryColorParam, Paint.SecondaryColor);
	}

	// Calculate roughness from paint type
	float Roughness = 0.3f;
	float Metallic = Paint.MetallicIntensity;

	switch (Paint.PaintType)
	{
	case EMGPaintType::Solid:
		Roughness = 0.4f;
		Metallic = 0.0f;
		break;

	case EMGPaintType::Metallic:
		Roughness = 0.2f;
		Metallic = 0.8f;
		break;

	case EMGPaintType::Pearlescent:
		Roughness = 0.15f;
		Metallic = 0.6f;
		Material->SetVectorParameterValue(PearlColorParam, Paint.PearlShiftColor);
		break;

	case EMGPaintType::Matte:
		Roughness = 0.9f;
		Metallic = 0.0f;
		break;

	case EMGPaintType::Chrome:
		Roughness = 0.05f;
		Metallic = 1.0f;
		break;

	case EMGPaintType::Chameleon:
		Roughness = 0.1f;
		Metallic = 0.7f;
		// Chameleon effect handled by special shader
		break;

	case EMGPaintType::Candy:
		Roughness = 0.15f;
		Metallic = 0.5f;
		break;

	case EMGPaintType::Satin:
		Roughness = 0.5f;
		Metallic = 0.3f;
		break;
	}

	Material->SetScalarParameterValue(MetallicParam, Metallic);
	Material->SetScalarParameterValue(RoughnessParam, Roughness);
	Material->SetScalarParameterValue(ClearCoatParam, Paint.ClearCoatIntensity);
}

void UMGVehicleConfigApplicator::ApplyTuningToMovement(UMGVehicleMovementComponent* Movement, const FMGTuningConfig& Tuning)
{
	if (!Movement)
	{
		return;
	}

	// ==========================================
	// ENGINE
	// ==========================================

	// Power adjustment affects max torque/HP
	float PowerMult = 1.0f + Tuning.PowerAdjust;
	float BasePower = Movement->GetMaxEnginePower();
	Movement->SetMaxEnginePower(BasePower * PowerMult);

	// ==========================================
	// TRANSMISSION
	// ==========================================

	// Final drive affects top speed vs acceleration trade-off
	float FinalDriveMult = 1.0f + Tuning.FinalDriveAdjust;
	// Movement->SetFinalDriveRatio(BaseFinalDrive * FinalDriveMult);

	// ==========================================
	// SUSPENSION
	// ==========================================

	// Ride height affects center of gravity
	// Negative = lower, more grip but scrapes
	float RideHeightOffset = Tuning.RideHeight * 5.0f; // cm

	// Spring stiffness affects body roll and responsiveness
	float StiffnessMult = 1.0f + Tuning.SpringStiffness * 0.5f;

	// Anti-roll bars affect weight transfer
	// Higher front = more understeer, higher rear = more oversteer

	// ==========================================
	// STEERING
	// ==========================================

	// Steering ratio affects turn-in speed
	float SteeringMult = 1.0f + Tuning.SteeringRatio * 0.3f;
	Movement->SetSteeringSensitivity(Tuning.SteeringSensitivity * SteeringMult);

	// ==========================================
	// BRAKES
	// ==========================================

	// Brake bias affects stability under braking
	// Higher = more front bias, safer but slower
	Movement->SetBrakeBias(Tuning.BrakeBias);

	// Brake force multiplier
	float BaseBrakeForce = Movement->GetMaxBrakingForce();
	Movement->SetMaxBrakingForce(BaseBrakeForce * Tuning.BrakeForce);

	// ==========================================
	// DIFFERENTIAL
	// ==========================================

	// Lock percentage affects traction and handling
	// More lock = better traction, worse turning
	Movement->SetDifferentialLockRatio(Tuning.DiffLock);

	// AWD torque split (if applicable)
	if (Movement->GetDriveType() == EMGDriveType::AWD)
	{
		Movement->SetTorqueSplit(Tuning.TorqueSplit);
	}

	// ==========================================
	// TIRES
	// ==========================================

	// Pressure affects grip and wear
	// Lower = more grip, faster wear
	// Camber affects cornering grip
	// Negative camber = more grip at limit

	// ==========================================
	// AERO
	// ==========================================

	// Downforce affects high-speed grip at cost of top speed
	float TotalDownforce = Tuning.DownforceFront + Tuning.DownforceRear;
	Movement->SetDownforceCoefficient(TotalDownforce * 0.5f);

	// ==========================================
	// NOS
	// ==========================================

	// NOS power and duration
	// Movement->SetNOSPowerMultiplier(Tuning.NOSPower);
	// Movement->SetNOSDurationMultiplier(Tuning.NOSDuration);
}

void UMGVehicleConfigApplicator::CalculatePartModifiers(const TArray<FMGInstalledPart>& Parts)
{
	// Calculate cumulative modifiers from all installed parts
	// Each part contributes to various stats based on its type

	float PowerBonus = 0.0f;
	float WeightReduction = 0.0f;
	float GripBonus = 0.0f;
	float AeroBonus = 0.0f;
	float BrakeBonus = 0.0f;

	for (const FMGInstalledPart& Part : Parts)
	{
		// Look up part data
		// Apply condition modifier (worn parts have reduced effect)
		float ConditionMod = FMath::Lerp(0.5f, 1.0f, Part.Condition);

		// Add part's bonuses to totals
		// PowerBonus += PartData.PowerBonus * ConditionMod;
		// etc...
	}

	// Store calculated modifiers for application
}
