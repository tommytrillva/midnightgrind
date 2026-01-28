// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGInsuranceSubsystem.h - Vehicle Insurance & Protection System
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Insurance Subsystem for Midnight Grind. Insurance is
 * a CRITICAL system that protects players' vehicles from permanent loss during
 * pink slip races (where you bet your car and the loser loses their vehicle
 * FOREVER). This creates meaningful risk management gameplay.
 *
 * WHY INSURANCE MATTERS:
 * - Pink slip races have PERMANENT consequences (lose your car forever)
 * - Insurance lets players protect valuable vehicles
 * - Creates interesting cost/benefit decisions
 * - Adds depth to the risk/reward gameplay loop
 *
 * KEY CONCEPTS:
 *
 * 1. INSURANCE TIERS (EInsuranceTier):
 *    - None: No protection (cheapest, highest risk)
 *    - Basic: Theft only
 *    - Standard: Pink slip protection
 *    - Premium: Full coverage
 *    - Elite: Platinum protection with extras
 *    - Collector: For rare/valuable vehicles
 *
 * 2. COVERAGE TYPES (ECoverageType):
 *    - PinkSlipLoss: THE BIG ONE - protects if you lose a pink slip race
 *    - TheftRecovery: If vehicle is "stolen" (game mechanic)
 *    - CollisionDamage: Race accident damage
 *    - PartsDamage: Performance part damage
 *    - TotalLoss: Complete vehicle write-off
 *    - ModificationLoss: Aftermarket parts protection
 *    - RaceAccident: General race damage
 *    - PoliceSeizure: If cops impound your car
 *
 * 3. POLICY LIFECYCLE (EPolicyStatus):
 *    Pending -> Active -> ClaimInProgress -> Active
 *                     -> Lapsed (missed payment)
 *                     -> Cancelled
 *                     -> Expired
 *
 * 4. CLAIMS PROCESS (EClaimStatus):
 *    Pending -> UnderReview -> Approved -> Paid
 *                           -> Denied -> Appealed -> (re-reviewed)
 *                           -> Fraudulent (flagged)
 *
 * 5. RISK ASSESSMENT (FDriverRiskProfile):
 *    - Tracks player's racing history
 *    - Win rate, accidents, previous claims
 *    - Affects premium pricing (bad drivers pay more)
 *    - FraudulentClaimsCount can lock players out
 *    - SafeDriverStreak gives discounts
 *
 * 6. PREMIUM CALCULATION:
 *    Premium = BaseRate * VehicleValue * RiskMultiplier - Discounts
 *    - Higher tier = higher base rate
 *    - Expensive vehicles cost more to insure
 *    - Bad drivers (high risk) pay more
 *    - Safe drivers get discounts
 *
 * 7. DEDUCTIBLES (EDeductibleLevel):
 *    - The amount YOU pay before insurance kicks in
 *    - Higher deductible = lower premium
 *    - None/$500/$1000/$2500/$5000 options
 *    - Risk vs cost tradeoff
 *
 * 8. PAYMENT FREQUENCY (EPaymentFrequency):
 *    - PerRace: Pay only when racing pink slips
 *    - Daily/Weekly/Monthly: Regular payments
 *    - Seasonal/Annual: Long-term savings
 *    - Longer terms = better rates
 *
 * 9. VEHICLE VALUATION (FVehicleValuation):
 *    - BaseValue: Stock vehicle value
 *    - ModificationValue: Aftermarket parts added
 *    - RarityBonus: For rare/limited vehicles
 *    - ConditionModifier: Based on vehicle state
 *    - MarketAdjustment: Current market conditions
 *    - InsuredValue: What insurance will pay out
 *
 * 10. INSURANCE PROVIDERS (FInsuranceProvider):
 *     - Multiple fictional insurance companies
 *     - Different rates, approval rates, processing times
 *     - Some specialize in certain vehicle types
 *     - Unlock better providers as you level up
 *
 * 11. POLICY RIDERS (Add-ons):
 *     - Optional coverage enhancements
 *     - Higher payout limits
 *     - Lower deductibles
 *     - Extra coverage types
 *     - Cost extra but provide better protection
 *
 * 12. FRAUD DETECTION:
 *     - FraudScore: 0.0 to 1.0 probability
 *     - Suspicious patterns trigger investigation
 *     - Repeated claims, disconnect losses, etc.
 *     - Fraud can result in denied claims and bans
 *
 * GAMEPLAY INTEGRATION:
 * - Before pink slip race: Check if vehicle is insured
 * - After loss: Automatically file claim if insured
 * - Claim approved: Receive payout (minus deductible)
 * - No insurance: Vehicle lost forever
 *
 * DELEGATES (Events):
 * - OnPolicyCreated: New policy purchased
 * - OnPolicyUpdated: Policy status changed
 * - OnClaimFiled: Player filed a claim
 * - OnClaimResolved: Claim approved/denied
 * - OnPremiumDue: Payment reminder
 * - OnPremiumPaid: Payment successful
 * - OnPolicyLapsed: Missed payment, coverage lost
 * - OnQuoteGenerated: New insurance quote ready
 * - OnRiskProfileUpdated: Risk assessment changed
 *
 * DESIGN PHILOSOPHY:
 * - Insurance should feel like a meaningful choice
 * - Not so cheap it's automatic
 * - Not so expensive it's useless
 * - Risk/reward balance is key
 *
 * RELATED FILES:
 * - MGInsuranceSubsystem.cpp (implementation)
 * - MGPinkSlipSubsystem.h (pink slip races that trigger claims)
 * - MGGarageSubsystem.h (vehicle management)
 * - MGWagerSubsystem.h (wager/race management)
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGInsuranceSubsystem.generated.h"

// ============================================================================
// INSURANCE SUBSYSTEM - Vehicle Protection & Risk Management
// ============================================================================
// Provides comprehensive insurance coverage for vehicles, protecting players
// from permanent loss during pink slip races. Includes premium calculations,
// claims processing, policy management, and risk assessment.
// ============================================================================

// ----------------------------------------------------------------------------
// ENUMS
// ----------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EInsuranceTier : uint8
{
    None            UMETA(DisplayName = "No Coverage"),
    Basic           UMETA(DisplayName = "Basic - Theft Only"),
    Standard        UMETA(DisplayName = "Standard - Pink Slip Protection"),
    Premium         UMETA(DisplayName = "Premium - Full Coverage"),
    Elite           UMETA(DisplayName = "Elite - Platinum Protection"),
    Collector       UMETA(DisplayName = "Collector - Rare Vehicle Coverage")
};

UENUM(BlueprintType)
enum class EClaimStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending Review"),
    UnderReview     UMETA(DisplayName = "Under Review"),
    Approved        UMETA(DisplayName = "Approved"),
    Denied          UMETA(DisplayName = "Denied"),
    Paid            UMETA(DisplayName = "Claim Paid"),
    Appealed        UMETA(DisplayName = "Under Appeal"),
    Fraudulent      UMETA(DisplayName = "Flagged Fraudulent"),
    Expired         UMETA(DisplayName = "Claim Expired")
};

UENUM(BlueprintType)
enum class ECoverageType : uint8
{
    PinkSlipLoss        UMETA(DisplayName = "Pink Slip Loss"),
    TheftRecovery       UMETA(DisplayName = "Theft Recovery"),
    CollisionDamage     UMETA(DisplayName = "Collision Damage"),
    PartsDamage         UMETA(DisplayName = "Parts Damage"),
    TotalLoss           UMETA(DisplayName = "Total Loss"),
    ModificationLoss    UMETA(DisplayName = "Modification Loss"),
    RaceAccident        UMETA(DisplayName = "Race Accident"),
    PoliceSeizure       UMETA(DisplayName = "Police Seizure")
};

UENUM(BlueprintType)
enum class ERiskCategory : uint8
{
    VeryLow         UMETA(DisplayName = "Very Low Risk"),
    Low             UMETA(DisplayName = "Low Risk"),
    Moderate        UMETA(DisplayName = "Moderate Risk"),
    High            UMETA(DisplayName = "High Risk"),
    VeryHigh        UMETA(DisplayName = "Very High Risk"),
    Extreme         UMETA(DisplayName = "Extreme Risk")
};

UENUM(BlueprintType)
enum class EPolicyStatus : uint8
{
    Active          UMETA(DisplayName = "Active"),
    Pending         UMETA(DisplayName = "Pending Activation"),
    Lapsed          UMETA(DisplayName = "Lapsed - Payment Due"),
    Cancelled       UMETA(DisplayName = "Cancelled"),
    Expired         UMETA(DisplayName = "Expired"),
    Suspended       UMETA(DisplayName = "Suspended"),
    ClaimInProgress UMETA(DisplayName = "Claim In Progress")
};

UENUM(BlueprintType)
enum class EDeductibleLevel : uint8
{
    None            UMETA(DisplayName = "No Deductible"),
    Low             UMETA(DisplayName = "Low ($500)"),
    Standard        UMETA(DisplayName = "Standard ($1000)"),
    High            UMETA(DisplayName = "High ($2500)"),
    VeryHigh        UMETA(DisplayName = "Very High ($5000)")
};

UENUM(BlueprintType)
enum class EPaymentFrequency : uint8
{
    PerRace         UMETA(DisplayName = "Per Race"),
    Daily           UMETA(DisplayName = "Daily"),
    Weekly          UMETA(DisplayName = "Weekly"),
    Monthly         UMETA(DisplayName = "Monthly"),
    Seasonal        UMETA(DisplayName = "Seasonal"),
    Annual          UMETA(DisplayName = "Annual")
};

// ----------------------------------------------------------------------------
// STRUCTS
// ----------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FVehicleValuation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString VehicleId;

    UPROPERTY(BlueprintReadWrite)
    int32 BaseValue = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ModificationValue = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 RarityBonus = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ConditionModifier = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 MarketAdjustment = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalValue = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 InsuredValue = 0;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ValuationDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ExpirationDate;

    UPROPERTY(BlueprintReadWrite)
    bool bIsApproved = false;
};

USTRUCT(BlueprintType)
struct FDriverRiskProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite)
    ERiskCategory RiskCategory = ERiskCategory::Moderate;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalRaces = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 RacesWon = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PinkSlipsLost = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PinkSlipsWon = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 AccidentCount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimsFiledCount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimsApprovedCount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimsDeniedCount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 FraudulentClaimsCount = 0;

    UPROPERTY(BlueprintReadWrite)
    float WinRate = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float ClaimFrequency = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float RiskScore = 50.0f;

    UPROPERTY(BlueprintReadWrite)
    int32 SafeDriverStreak = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimFreeMonths = 0;

    UPROPERTY(BlueprintReadWrite)
    float PremiumMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FCoverageDetails
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECoverageType CoverageType = ECoverageType::PinkSlipLoss;

    UPROPERTY(BlueprintReadWrite)
    bool bIsEnabled = false;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxPayout = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Deductible = 0;

    UPROPERTY(BlueprintReadWrite)
    float CoveragePercent = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimsRemaining = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 WaitingPeriodHours = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ExcludedVehicleClasses;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ExcludedRaceTypes;
};

USTRUCT(BlueprintType)
struct FInsurancePolicy
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PolicyId;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite)
    FString VehicleId;

    UPROPERTY(BlueprintReadWrite)
    EInsuranceTier Tier = EInsuranceTier::None;

    UPROPERTY(BlueprintReadWrite)
    EPolicyStatus Status = EPolicyStatus::Pending;

    UPROPERTY(BlueprintReadWrite)
    TArray<FCoverageDetails> Coverages;

    UPROPERTY(BlueprintReadWrite)
    EDeductibleLevel DeductibleLevel = EDeductibleLevel::Standard;

    UPROPERTY(BlueprintReadWrite)
    int32 DeductibleAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PremiumAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    EPaymentFrequency PaymentFrequency = EPaymentFrequency::Monthly;

    UPROPERTY(BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime NextPaymentDate;

    UPROPERTY(BlueprintReadWrite)
    int32 InsuredValue = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxClaimsPerPeriod = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimsUsedThisPeriod = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalClaimsPaid = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalPremiumsPaid = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bAutoRenew = true;

    UPROPERTY(BlueprintReadWrite)
    bool bIsTransferable = false;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> PolicyRiders;

    UPROPERTY(BlueprintReadWrite)
    FString ProviderName;
};

USTRUCT(BlueprintType)
struct FInsuranceClaim
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ClaimId;

    UPROPERTY(BlueprintReadWrite)
    FString PolicyId;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite)
    FString VehicleId;

    UPROPERTY(BlueprintReadWrite)
    ECoverageType ClaimType = ECoverageType::PinkSlipLoss;

    UPROPERTY(BlueprintReadWrite)
    EClaimStatus Status = EClaimStatus::Pending;

    UPROPERTY(BlueprintReadWrite)
    FString IncidentDescription;

    UPROPERTY(BlueprintReadWrite)
    FString RaceId;

    UPROPERTY(BlueprintReadWrite)
    FString OpponentId;

    UPROPERTY(BlueprintReadWrite)
    FDateTime IncidentDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ClaimFiledDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ReviewDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ResolutionDate;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ApprovedAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 DeductibleApplied = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PayoutAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    FString DenialReason;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> SupportingEvidence;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ReviewNotes;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAppealed = false;

    UPROPERTY(BlueprintReadWrite)
    FString AppealReason;

    UPROPERTY(BlueprintReadWrite)
    float FraudScore = 0.0f;
};

USTRUCT(BlueprintType)
struct FInsuranceQuote
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString QuoteId;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite)
    FString VehicleId;

    UPROPERTY(BlueprintReadWrite)
    EInsuranceTier Tier = EInsuranceTier::Standard;

    UPROPERTY(BlueprintReadWrite)
    int32 MonthlyPremium = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 AnnualPremium = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PerRacePremium = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<FCoverageDetails> IncludedCoverages;

    UPROPERTY(BlueprintReadWrite)
    TArray<FCoverageDetails> OptionalCoverages;

    UPROPERTY(BlueprintReadWrite)
    FDriverRiskProfile RiskProfile;

    UPROPERTY(BlueprintReadWrite)
    FVehicleValuation Valuation;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> AvailableDiscounts;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalDiscountPercent = 0;

    UPROPERTY(BlueprintReadWrite)
    FDateTime QuoteDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime ExpirationDate;

    UPROPERTY(BlueprintReadWrite)
    bool bIsExpired = false;
};

USTRUCT(BlueprintType)
struct FInsuranceProvider
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ProviderId;

    UPROPERTY(BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    TArray<EInsuranceTier> AvailableTiers;

    UPROPERTY(BlueprintReadWrite)
    float BaseRateMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float ClaimApprovalRate = 0.85f;

    UPROPERTY(BlueprintReadWrite)
    int32 ClaimProcessingDays = 3;

    UPROPERTY(BlueprintReadWrite)
    int32 MinPlayerLevel = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 MinReputation = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Specializations;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ExcludedVehicleTypes;

    UPROPERTY(BlueprintReadWrite)
    float CustomerRating = 4.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUnlocked = true;
};

USTRUCT(BlueprintType)
struct FPolicyRider
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString RiderId;

    UPROPERTY(BlueprintReadWrite)
    FString RiderName;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 AdditionalPremium = 0;

    UPROPERTY(BlueprintReadWrite)
    float PremiumMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    ECoverageType AffectedCoverage = ECoverageType::PinkSlipLoss;

    UPROPERTY(BlueprintReadWrite)
    int32 AdditionalMaxPayout = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 DeductibleReduction = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<EInsuranceTier> CompatibleTiers;

    UPROPERTY(BlueprintReadWrite)
    bool bIsStackable = false;
};

USTRUCT(BlueprintType)
struct FInsuranceDiscount
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString DiscountId;

    UPROPERTY(BlueprintReadWrite)
    FString DiscountName;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 DiscountPercent = 0;

    UPROPERTY(BlueprintReadWrite)
    FString RequirementType;

    UPROPERTY(BlueprintReadWrite)
    int32 RequirementValue = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsStackable = true;

    UPROPERTY(BlueprintReadWrite)
    TArray<EInsuranceTier> ApplicableTiers;
};

USTRUCT(BlueprintType)
struct FClaimInvestigation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString InvestigationId;

    UPROPERTY(BlueprintReadWrite)
    FString ClaimId;

    UPROPERTY(BlueprintReadWrite)
    FString InvestigatorId;

    UPROPERTY(BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime CompletionDate;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> FindingsSummary;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> EvidenceReviewed;

    UPROPERTY(BlueprintReadWrite)
    float FraudProbability = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bRecommendApproval = true;

    UPROPERTY(BlueprintReadWrite)
    FString Recommendation;
};

USTRUCT(BlueprintType)
struct FPremiumPayment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PaymentId;

    UPROPERTY(BlueprintReadWrite)
    FString PolicyId;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite)
    int32 Amount = 0;

    UPROPERTY(BlueprintReadWrite)
    FDateTime PaymentDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime CoveragePeriodStart;

    UPROPERTY(BlueprintReadWrite)
    FDateTime CoveragePeriodEnd;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAutomatic = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsSuccessful = true;

    UPROPERTY(BlueprintReadWrite)
    FString FailureReason;
};

USTRUCT(BlueprintType)
struct FInsuranceBundle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString BundleId;

    UPROPERTY(BlueprintReadWrite)
    FString BundleName;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxVehicles = 5;

    UPROPERTY(BlueprintReadWrite)
    EInsuranceTier BaseTier = EInsuranceTier::Standard;

    UPROPERTY(BlueprintReadWrite)
    int32 BundleDiscountPercent = 15;

    UPROPERTY(BlueprintReadWrite)
    int32 MonthlyPremium = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> IncludedVehicleIds;

    UPROPERTY(BlueprintReadWrite)
    TArray<FCoverageDetails> BundleCoverages;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = false;
};

// ----------------------------------------------------------------------------
// DELEGATES
// ----------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPolicyCreated, const FString&, PolicyId, const FInsurancePolicy&, Policy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPolicyUpdated, const FString&, PolicyId, EPolicyStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClaimFiled, const FString&, ClaimId, const FInsuranceClaim&, Claim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClaimResolved, const FString&, ClaimId, EClaimStatus, Status, int32, PayoutAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPremiumDue, const FString&, PolicyId, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPremiumPaid, const FString&, PolicyId, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPolicyLapsed, const FString&, PolicyId, const FString&, VehicleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuoteGenerated, const FString&, QuoteId, const FInsuranceQuote&, Quote);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRiskProfileUpdated, const FString&, PlayerId, const FDriverRiskProfile&, Profile);

// ============================================================================
// INSURANCE SUBSYSTEM CLASS
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGInsuranceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ------------------------------------------------------------------------
    // POLICY MANAGEMENT
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    FInsuranceQuote GenerateQuote(const FString& PlayerId, const FString& VehicleId, EInsuranceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    FInsurancePolicy CreatePolicy(const FString& PlayerId, const FString& QuoteId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool CancelPolicy(const FString& PolicyId, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool RenewPolicy(const FString& PolicyId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool UpgradePolicy(const FString& PolicyId, EInsuranceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool DowngradePolicy(const FString& PolicyId, EInsuranceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool AddPolicyRider(const FString& PolicyId, const FString& RiderId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool RemovePolicyRider(const FString& PolicyId, const FString& RiderId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool TransferPolicy(const FString& PolicyId, const FString& NewVehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    FInsurancePolicy GetPolicy(const FString& PolicyId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    TArray<FInsurancePolicy> GetPlayerPolicies(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    FInsurancePolicy GetVehiclePolicy(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool IsVehicleInsured(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Policy")
    bool HasActiveCoverage(const FString& VehicleId, ECoverageType CoverageType) const;

    // ------------------------------------------------------------------------
    // CLAIMS MANAGEMENT
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    FInsuranceClaim FileClaim(const FString& PolicyId, ECoverageType ClaimType, const FString& Description, const FString& RaceId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    bool AppealClaim(const FString& ClaimId, const FString& AppealReason);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    bool AddClaimEvidence(const FString& ClaimId, const FString& EvidenceId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    EClaimStatus GetClaimStatus(const FString& ClaimId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    FInsuranceClaim GetClaim(const FString& ClaimId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    TArray<FInsuranceClaim> GetPlayerClaims(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    TArray<FInsuranceClaim> GetPolicyClaims(const FString& PolicyId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    bool CanFileClaim(const FString& PolicyId, ECoverageType ClaimType) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims")
    int32 EstimateClaimPayout(const FString& PolicyId, ECoverageType ClaimType) const;

    // ------------------------------------------------------------------------
    // CLAIMS PROCESSING (Internal/Admin)
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims|Admin")
    bool ProcessClaim(const FString& ClaimId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims|Admin")
    bool ApproveClaim(const FString& ClaimId, int32 ApprovedAmount);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims|Admin")
    bool DenyClaim(const FString& ClaimId, const FString& DenialReason);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims|Admin")
    FClaimInvestigation InvestigateClaim(const FString& ClaimId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Claims|Admin")
    float CalculateFraudScore(const FString& ClaimId) const;

    // ------------------------------------------------------------------------
    // PREMIUM MANAGEMENT
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    int32 CalculatePremium(const FString& VehicleId, EInsuranceTier Tier, EPaymentFrequency Frequency);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    bool PayPremium(const FString& PolicyId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    bool EnableAutoPayment(const FString& PolicyId, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    int32 GetNextPremiumAmount(const FString& PolicyId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    FDateTime GetNextPaymentDate(const FString& PolicyId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    TArray<FPremiumPayment> GetPaymentHistory(const FString& PolicyId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    int32 ApplyDiscounts(int32 BasePremium, const FString& PlayerId, const FString& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Premium")
    TArray<FInsuranceDiscount> GetAvailableDiscounts(const FString& PlayerId) const;

    // ------------------------------------------------------------------------
    // VEHICLE VALUATION
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Valuation")
    FVehicleValuation GetVehicleValuation(const FString& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Valuation")
    int32 CalculateBaseValue(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Valuation")
    int32 CalculateModificationValue(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Valuation")
    int32 CalculateRarityBonus(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Valuation")
    bool RequestValuationAppraisal(const FString& VehicleId);

    // ------------------------------------------------------------------------
    // RISK ASSESSMENT
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    FDriverRiskProfile GetRiskProfile(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    void UpdateRiskProfile(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    ERiskCategory CalculateRiskCategory(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    float CalculatePremiumMultiplier(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    void RecordPinkSlipResult(const FString& PlayerId, bool bWon, const FString& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Risk")
    void RecordAccident(const FString& PlayerId, const FString& RaceId);

    // ------------------------------------------------------------------------
    // PROVIDERS
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Providers")
    TArray<FInsuranceProvider> GetAvailableProviders(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Providers")
    FInsuranceProvider GetProvider(const FString& ProviderId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Providers")
    bool UnlockProvider(const FString& PlayerId, const FString& ProviderId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Providers")
    TArray<FInsuranceProvider> GetProvidersForVehicle(const FString& VehicleId) const;

    // ------------------------------------------------------------------------
    // BUNDLES
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    FInsuranceBundle CreateBundle(const FString& PlayerId, const TArray<FString>& VehicleIds, EInsuranceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    bool AddVehicleToBundle(const FString& BundleId, const FString& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    bool RemoveVehicleFromBundle(const FString& BundleId, const FString& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    FInsuranceBundle GetBundle(const FString& BundleId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    TArray<FInsuranceBundle> GetPlayerBundles(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Bundles")
    int32 CalculateBundleSavings(const TArray<FString>& VehicleIds, EInsuranceTier Tier) const;

    // ------------------------------------------------------------------------
    // RIDERS & ADD-ONS
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Riders")
    TArray<FPolicyRider> GetAvailableRiders(EInsuranceTier Tier) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Riders")
    FPolicyRider GetRider(const FString& RiderId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Riders")
    int32 CalculateRiderCost(const FString& RiderId, int32 BasePremium) const;

    // ------------------------------------------------------------------------
    // COVERAGE QUERIES
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Coverage")
    TArray<ECoverageType> GetTierCoverages(EInsuranceTier Tier) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Coverage")
    FCoverageDetails GetCoverageDetails(const FString& PolicyId, ECoverageType CoverageType) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Coverage")
    int32 GetCoverageLimit(const FString& PolicyId, ECoverageType CoverageType) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Coverage")
    int32 GetRemainingClaims(const FString& PolicyId, ECoverageType CoverageType) const;

    // ------------------------------------------------------------------------
    // STATISTICS
    // ------------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Insurance|Stats")
    int32 GetTotalPremiumsPaid(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Stats")
    int32 GetTotalClaimsPaid(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Stats")
    int32 GetActivePolicyCount(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Stats")
    float GetClaimApprovalRate(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Insurance|Stats")
    int32 GetClaimFreeStreak(const FString& PlayerId) const;

    // ------------------------------------------------------------------------
    // DELEGATES
    // ------------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnPolicyCreated OnPolicyCreated;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnPolicyUpdated OnPolicyUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnClaimFiled OnClaimFiled;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnClaimResolved OnClaimResolved;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnPremiumDue OnPremiumDue;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnPremiumPaid OnPremiumPaid;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnPolicyLapsed OnPolicyLapsed;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnQuoteGenerated OnQuoteGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Insurance|Events")
    FOnRiskProfileUpdated OnRiskProfileUpdated;

protected:
    // Policy storage
    UPROPERTY()
    TMap<FString, FInsurancePolicy> ActivePolicies;

    UPROPERTY()
    TMap<FString, FInsuranceClaim> Claims;

    UPROPERTY()
    TMap<FString, FInsuranceQuote> PendingQuotes;

    UPROPERTY()
    TMap<FString, FDriverRiskProfile> RiskProfiles;

    UPROPERTY()
    TMap<FString, FInsuranceProvider> Providers;

    UPROPERTY()
    TMap<FString, FInsuranceBundle> Bundles;

    UPROPERTY()
    TMap<FString, FPolicyRider> AvailableRiders;

    UPROPERTY()
    TMap<FString, FVehicleValuation> CachedValuations;

    UPROPERTY()
    TMap<FString, TArray<FPremiumPayment>> PaymentHistories;

private:
    void InitializeProviders();
    void InitializeRiders();
    void InitializeDiscounts();
    void ProcessPendingClaims();
    void CheckPolicyExpirations();
    void ProcessAutoRenewals();

    FString GeneratePolicyId();
    FString GenerateClaimId();
    FString GenerateQuoteId();
    FString GenerateBundleId();

    int32 GetBaseRateForTier(EInsuranceTier Tier) const;
    int32 GetDeductibleAmount(EDeductibleLevel Level) const;
    float GetFrequencyMultiplier(EPaymentFrequency Frequency) const;

    bool ValidateClaimEligibility(const FInsuranceClaim& Claim) const;
    bool CheckForFraudIndicators(const FInsuranceClaim& Claim) const;

    TArray<FInsuranceDiscount> AvailableDiscounts;
    FTimerHandle ClaimProcessingTimer;
    FTimerHandle PolicyCheckTimer;
};
