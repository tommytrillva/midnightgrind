// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputRemap/MGInputRemapSubsystem.h"
#include "Checkpoint/MGCheckpointSubsystem.h"
#include "NearMiss/MGNearMissSubsystem.h"
#include "Drift/MGDriftSubsystem.h"
#include "Airtime/MGAirtimeSubsystem.h"
#include "Fuel/MGFuelSubsystem.h"
#include "Tire/MGTireSubsystem.h"
#include "Collision/MGCollisionSubsystem.h"
#include "PitStop/MGPitStopSubsystem.h"
#include "Bonus/MGBonusSubsystem.h"
#include "Pursuit/MGPursuitSubsystem.h"
#include "Speedtrap/MGSpeedtrapSubsystem.h"
#include "Destruction/MGDestructionSubsystem.h"
#include "Aerodynamics/MGAerodynamicsSubsystem.h"
#include "Scoring/MGScoringSubsystem.h"
#include "Achievements/MGAchievementSubsystem.h"
#include "Streak/MGStreakSubsystem.h"
#include "Prestige/MGPrestigeSubsystem.h"
#include "NitroBoost/MGNitroBoostSubsystem.h"
#include "Stunt/MGStuntSubsystem.h"
#include "Takedown/MGTakedownSubsystem.h"
#include "Powerup/MGPowerupSubsystem.h"
#include "Vehicle/MGVehicleWearSubsystem.h"
#include "Weather/MGWeatherSubsystem.h"
#include "Caution/MGCautionSubsystem.h"
#include "Penalty/MGPenaltySubsystem.h"
#include "HeatLevel/MGHeatLevelSubsystem.h"
#include "Bounty/MGBountySubsystem.h"
#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "License/MGLicenseSubsystem.h"
#include "Contract/MGContractSubsystem.h"
#include "Challenges/MGChallengeSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "DailyRewards/MGDailyRewardsSubsystem.h"
#include "Reputation/MGReputationSubsystem.h"
#include "Ghost/MGGhostSubsystem.h"
#include "Shortcut/MGShortcutSubsystem.h"
#include "Career/MGCareerSubsystem.h"
#include "Rivals/MGRivalsSubsystem.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

AMGPlayerController::AMGPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AMGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get input remap subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InputRemapSubsystem = GameInstance->GetSubsystem<UMGInputRemapSubsystem>();
	}

	// Set up default input mapping
	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// Bind to checkpoint subsystem's events
		if (UWorld* World = GetWorld())
		{
			if (UMGCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UMGCheckpointSubsystem>())
			{
				CheckpointSubsystem->OnWrongWay.AddDynamic(this, &AMGPlayerController::OnWrongWayDetected);
				CheckpointSubsystem->OnCheckpointPassed.AddDynamic(this, &AMGPlayerController::OnCheckpointPassed);
				CheckpointSubsystem->OnLapCompleted.AddDynamic(this, &AMGPlayerController::OnLapCompleted);
			}

			// Bind to near miss subsystem for HUD popups
			if (UMGNearMissSubsystem* NearMissSubsystem = World->GetSubsystem<UMGNearMissSubsystem>())
			{
				NearMissSubsystem->OnNearMissOccurred.AddDynamic(this, &AMGPlayerController::OnNearMissDetected);
			}

			// Bind to vehicle wear subsystem for engine warnings
			if (UMGVehicleWearSubsystem* WearSubsystem = World->GetSubsystem<UMGVehicleWearSubsystem>())
			{
				WearSubsystem->OnEngineOverheat.AddDynamic(this, &AMGPlayerController::OnEngineOverheat);
			}

			// Bind to weather subsystem for weather change notifications
			if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
			{
				WeatherSubsystem->OnWeatherTransitionStarted.AddDynamic(this, &AMGPlayerController::OnWeatherTransitionStarted);
			}
		}

		// Bind to drift subsystem for score popups (GameInstance subsystem)
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGDriftSubsystem* DriftSubsystem = GI->GetSubsystem<UMGDriftSubsystem>())
			{
				DriftSubsystem->OnDriftEnded.AddDynamic(this, &AMGPlayerController::OnDriftEnded);
			}

			// Bind to airtime subsystem for jump/trick popups
			if (UMGAirtimeSubsystem* AirtimeSubsystem = GI->GetSubsystem<UMGAirtimeSubsystem>())
			{
				AirtimeSubsystem->OnJumpEnded.AddDynamic(this, &AMGPlayerController::OnJumpEnded);
				AirtimeSubsystem->OnTrickCompleted.AddDynamic(this, &AMGPlayerController::OnTrickCompleted);
			}

			// Bind to fuel subsystem for low fuel warnings
			if (UMGFuelSubsystem* FuelSubsystem = GI->GetSubsystem<UMGFuelSubsystem>())
			{
				FuelSubsystem->OnFuelAlert.AddDynamic(this, &AMGPlayerController::OnFuelAlert);
				FuelSubsystem->OnFuelEmpty.AddDynamic(this, &AMGPlayerController::OnFuelEmpty);
			}

			// Bind to tire subsystem for puncture warnings
			if (UMGTireSubsystem* TireSubsystem = GI->GetSubsystem<UMGTireSubsystem>())
			{
				TireSubsystem->OnTirePunctured.AddDynamic(this, &AMGPlayerController::OnTirePunctured);
				TireSubsystem->OnTireConditionChanged.AddDynamic(this, &AMGPlayerController::OnTireConditionChanged);
			}

			// Bind to collision subsystem for takedown notifications and damage state
			if (UMGCollisionSubsystem* CollisionSubsystem = GI->GetSubsystem<UMGCollisionSubsystem>())
			{
				CollisionSubsystem->OnTakedownDealt.AddDynamic(this, &AMGPlayerController::OnTakedownDealt);
				CollisionSubsystem->OnTakedownChain.AddDynamic(this, &AMGPlayerController::OnTakedownChain);
				CollisionSubsystem->OnRevengeComplete.AddDynamic(this, &AMGPlayerController::OnRevengeComplete);
				CollisionSubsystem->OnDamageStateChanged.AddDynamic(this, &AMGPlayerController::OnDamageStateChanged);
			}

			// Bind to pit stop subsystem for pit notifications
			if (UMGPitStopSubsystem* PitStopSubsystem = GI->GetSubsystem<UMGPitStopSubsystem>())
			{
				PitStopSubsystem->OnPitStopCompleted.AddDynamic(this, &AMGPlayerController::OnPitStopCompleted);
				PitStopSubsystem->OnPitLaneViolation.AddDynamic(this, &AMGPlayerController::OnPitLaneViolation);
			}

			// Bind to bonus subsystem for pickup notifications
			if (UMGBonusSubsystem* BonusSubsystem = GI->GetSubsystem<UMGBonusSubsystem>())
			{
				BonusSubsystem->OnBonusCollected.AddDynamic(this, &AMGPlayerController::OnBonusCollected);
				BonusSubsystem->OnComboBonusTriggered.AddDynamic(this, &AMGPlayerController::OnComboBonusTriggered);
				BonusSubsystem->OnSecretBonusFound.AddDynamic(this, &AMGPlayerController::OnSecretBonusFound);
			}

			// Bind to pursuit subsystem for chase notifications
			if (UMGPursuitSubsystem* PursuitSubsystem = GI->GetSubsystem<UMGPursuitSubsystem>())
			{
				PursuitSubsystem->OnPursuitStarted.AddDynamic(this, &AMGPlayerController::OnPursuitStarted);
				PursuitSubsystem->OnPursuitEnded.AddDynamic(this, &AMGPlayerController::OnPursuitEnded);
				PursuitSubsystem->OnUnitDisabled.AddDynamic(this, &AMGPlayerController::OnUnitDisabled);
				PursuitSubsystem->OnRoadblockEvaded.AddDynamic(this, &AMGPlayerController::OnRoadblockEvaded);
			}

			// Bind to speedtrap subsystem for speed camera feedback
			if (UMGSpeedtrapSubsystem* SpeedtrapSubsystem = GI->GetSubsystem<UMGSpeedtrapSubsystem>())
			{
				SpeedtrapSubsystem->OnSpeedtrapRecorded.AddDynamic(this, &AMGPlayerController::OnSpeedtrapRecorded);
				SpeedtrapSubsystem->OnSpeedtrapNewPersonalBest.AddDynamic(this, &AMGPlayerController::OnSpeedtrapNewPersonalBest);
				SpeedtrapSubsystem->OnSpeedtrapDiscovered.AddDynamic(this, &AMGPlayerController::OnSpeedtrapDiscovered);
			}

			// Bind to destruction subsystem for smash feedback
			if (UMGDestructionSubsystem* DestructionSubsystem = GI->GetSubsystem<UMGDestructionSubsystem>())
			{
				DestructionSubsystem->OnDestructibleDestroyed.AddDynamic(this, &AMGPlayerController::OnDestructibleDestroyed);
				DestructionSubsystem->OnDestructionComboUpdated.AddDynamic(this, &AMGPlayerController::OnDestructionComboUpdated);
				DestructionSubsystem->OnSpectacularDestruction.AddDynamic(this, &AMGPlayerController::OnSpectacularDestruction);
			}

			// Bind to aerodynamics subsystem for slipstream feedback
			if (UMGAerodynamicsSubsystem* AeroSubsystem = GI->GetSubsystem<UMGAerodynamicsSubsystem>())
			{
				AeroSubsystem->OnSlipstreamEntered.AddDynamic(this, &AMGPlayerController::OnSlipstreamEntered);
				AeroSubsystem->OnSlingshotReady.AddDynamic(this, &AMGPlayerController::OnSlingshotReady);
				AeroSubsystem->OnSlingshotUsed.AddDynamic(this, &AMGPlayerController::OnSlingshotUsed);
			}

			// Bind to scoring subsystem for score popups
			if (UMGScoringSubsystem* ScoringSubsystem = GI->GetSubsystem<UMGScoringSubsystem>())
			{
				ScoringSubsystem->OnScoreEvent.AddDynamic(this, &AMGPlayerController::OnScoreEvent);
				ScoringSubsystem->OnChainExtended.AddDynamic(this, &AMGPlayerController::OnChainExtended);
			}

			// Bind to achievement subsystem for unlock notifications
			if (UMGAchievementSubsystem* AchievementSubsystem = GI->GetSubsystem<UMGAchievementSubsystem>())
			{
				AchievementSubsystem->OnAchievementUnlocked.AddDynamic(this, &AMGPlayerController::OnAchievementUnlocked);
			}

			// Bind to streak subsystem for streak notifications
			if (UMGStreakSubsystem* StreakSubsystem = GI->GetSubsystem<UMGStreakSubsystem>())
			{
				StreakSubsystem->OnStreakTierUp.AddDynamic(this, &AMGPlayerController::OnStreakTierUp);
				StreakSubsystem->OnNewStreakRecord.AddDynamic(this, &AMGPlayerController::OnNewStreakRecord);
			}

			// Bind to prestige subsystem for rank up notifications
			if (UMGPrestigeSubsystem* PrestigeSubsystem = GI->GetSubsystem<UMGPrestigeSubsystem>())
			{
				PrestigeSubsystem->OnPrestigeRankUp.AddDynamic(this, &AMGPlayerController::OnPrestigeRankUp);
				PrestigeSubsystem->OnPrestigeLevelUp.AddDynamic(this, &AMGPlayerController::OnPrestigeLevelUp);
			}

			// Bind to nitro boost subsystem for nitro feedback
			if (UMGNitroBoostSubsystem* NitroSubsystem = GI->GetSubsystem<UMGNitroBoostSubsystem>())
			{
				NitroSubsystem->OnNitroDepleted.AddDynamic(this, &AMGPlayerController::OnNitroDepleted);
				NitroSubsystem->OnNitroOverheat.AddDynamic(this, &AMGPlayerController::OnNitroOverheat);
			}

			// Bind to stunt subsystem for stunt completion feedback
			if (UMGStuntSubsystem* StuntSubsystem = GI->GetSubsystem<UMGStuntSubsystem>())
			{
				StuntSubsystem->OnStuntCompleted.AddDynamic(this, &AMGPlayerController::OnStuntCompleted);
			}

			// Bind to takedown subsystem for rampage notifications
			if (UMGTakedownSubsystem* TakedownSubsystem = GI->GetSubsystem<UMGTakedownSubsystem>())
			{
				TakedownSubsystem->OnRampageActivated.AddDynamic(this, &AMGPlayerController::OnRampageActivated);
			}

			// Bind to powerup subsystem for pickup/hit feedback
			if (UMGPowerupSubsystem* PowerupSubsystem = GI->GetSubsystem<UMGPowerupSubsystem>())
			{
				PowerupSubsystem->OnPowerupCollected.AddDynamic(this, &AMGPlayerController::OnPowerupCollected);
				PowerupSubsystem->OnPowerupHit.AddDynamic(this, &AMGPlayerController::OnPowerupHit);
			}

			// Bind to caution subsystem for yellow flag/safety car
			if (UMGCautionSubsystem* CautionSubsystem = GI->GetSubsystem<UMGCautionSubsystem>())
			{
				CautionSubsystem->OnCautionDeployed.AddDynamic(this, &AMGPlayerController::OnCautionDeployed);
				CautionSubsystem->OnCautionEnded.AddDynamic(this, &AMGPlayerController::OnCautionEnded);
				CautionSubsystem->OnSafetyCarDeployed.AddDynamic(this, &AMGPlayerController::OnSafetyCarDeployed);
				CautionSubsystem->OnSafetyCarIn.AddDynamic(this, &AMGPlayerController::OnSafetyCarIn);
			}

			// Bind to penalty subsystem for race penalties
			if (UMGPenaltySubsystem* PenaltySubsystem = GI->GetSubsystem<UMGPenaltySubsystem>())
			{
				PenaltySubsystem->OnPenaltyIssued.AddDynamic(this, &AMGPlayerController::OnPenaltyIssued);
				PenaltySubsystem->OnPenaltyServed.AddDynamic(this, &AMGPlayerController::OnPenaltyServed);
			}

			// Bind to heat level subsystem for police pursuit notifications
			if (UMGHeatLevelSubsystem* HeatSubsystem = GI->GetSubsystem<UMGHeatLevelSubsystem>())
			{
				HeatSubsystem->OnHeatLevelChanged.AddDynamic(this, &AMGPlayerController::OnHeatLevelChanged);
				HeatSubsystem->OnPursuitEvaded.AddDynamic(this, &AMGPlayerController::OnPursuitEvaded);
				HeatSubsystem->OnPlayerBusted.AddDynamic(this, &AMGPlayerController::OnPlayerBusted);
				HeatSubsystem->OnHelicopterDeployed.AddDynamic(this, &AMGPlayerController::OnHelicopterDeployed);
			}

			// Bind to bounty subsystem for bounty progress
			if (UMGBountySubsystem* BountySubsystem = GI->GetSubsystem<UMGBountySubsystem>())
			{
				BountySubsystem->OnBountyCompleted.AddDynamic(this, &AMGPlayerController::OnBountyCompleted);
				BountySubsystem->OnBountyFailed.AddDynamic(this, &AMGPlayerController::OnBountyFailed);
				BountySubsystem->OnBountyObjectiveCompleted.AddDynamic(this, &AMGPlayerController::OnBountyObjectiveCompleted);
			}

			// Bind to race director for dramatic moments
			if (UMGRaceDirectorSubsystem* DirectorSubsystem = GI->GetSubsystem<UMGRaceDirectorSubsystem>())
			{
				DirectorSubsystem->OnDramaticMoment.AddDynamic(this, &AMGPlayerController::OnDramaticMoment);
				DirectorSubsystem->OnLeadChange.AddDynamic(this, &AMGPlayerController::OnLeadChange);
			}

			// Bind to license subsystem for license upgrades
			if (UMGLicenseSubsystem* LicenseSubsystem = GI->GetSubsystem<UMGLicenseSubsystem>())
			{
				LicenseSubsystem->OnLicenseUpgraded.AddDynamic(this, &AMGPlayerController::OnLicenseUpgraded);
				LicenseSubsystem->OnTestCompleted.AddDynamic(this, &AMGPlayerController::OnLicenseTestCompleted);
			}

			// Bind to contract subsystem for mission progress
			if (UMGContractSubsystem* ContractSubsystem = GI->GetSubsystem<UMGContractSubsystem>())
			{
				ContractSubsystem->OnContractCompleted.AddDynamic(this, &AMGPlayerController::OnContractCompleted);
				ContractSubsystem->OnObjectiveCompleted.AddDynamic(this, &AMGPlayerController::OnContractObjectiveCompleted);
				ContractSubsystem->OnSponsorLevelUp.AddDynamic(this, &AMGPlayerController::OnSponsorLevelUp);
			}

			// Bind to challenge subsystem for challenge notifications
			if (UMGChallengeSubsystem* ChallengeSubsystem = GI->GetSubsystem<UMGChallengeSubsystem>())
			{
				ChallengeSubsystem->OnChallengeCompleted.AddDynamic(this, &AMGPlayerController::OnChallengeCompleted);
			}

			// Bind to currency subsystem for reward notifications
			if (UMGCurrencySubsystem* CurrencySubsystem = GI->GetSubsystem<UMGCurrencySubsystem>())
			{
				CurrencySubsystem->OnCurrencyChanged.AddDynamic(this, &AMGPlayerController::OnCurrencyChanged);
				CurrencySubsystem->OnMultiplierActivated.AddDynamic(this, &AMGPlayerController::OnMultiplierActivated);
			}

			// Bind to daily rewards subsystem for login bonuses
			if (UMGDailyRewardsSubsystem* DailyRewardsSubsystem = GI->GetSubsystem<UMGDailyRewardsSubsystem>())
			{
				DailyRewardsSubsystem->OnDailyRewardClaimed.AddDynamic(this, &AMGPlayerController::OnDailyRewardClaimed);
				DailyRewardsSubsystem->OnMilestoneReached.AddDynamic(this, &AMGPlayerController::OnStreakMilestoneReached);
			}

			// Bind to reputation subsystem for tier unlocks
			if (UMGReputationSubsystem* RepSubsystem = GI->GetSubsystem<UMGReputationSubsystem>())
			{
				RepSubsystem->OnTierReached.AddDynamic(this, &AMGPlayerController::OnReputationTierReached);
				RepSubsystem->OnUnlockEarned.AddDynamic(this, &AMGPlayerController::OnReputationUnlockEarned);
			}

			// Bind to ghost subsystem for personal bests
			if (UMGGhostSubsystem* GhostSubsystem = GI->GetSubsystem<UMGGhostSubsystem>())
			{
				GhostSubsystem->OnNewPersonalBest.AddDynamic(this, &AMGPlayerController::OnGhostNewPersonalBest);
				GhostSubsystem->OnGhostComparison.AddDynamic(this, &AMGPlayerController::OnGhostComparison);
			}

			// Bind to shortcut subsystem for shortcut feedback
			if (UMGShortcutSubsystem* ShortcutSubsystem = GI->GetSubsystem<UMGShortcutSubsystem>())
			{
				ShortcutSubsystem->OnShortcutDiscovered.AddDynamic(this, &AMGPlayerController::OnShortcutDiscovered);
				ShortcutSubsystem->OnShortcutCompleted.AddDynamic(this, &AMGPlayerController::OnShortcutCompleted);
				ShortcutSubsystem->OnShortcutMastered.AddDynamic(this, &AMGPlayerController::OnShortcutMastered);
				ShortcutSubsystem->OnSecretShortcutFound.AddDynamic(this, &AMGPlayerController::OnSecretShortcutFound);
			}

			// Bind to career subsystem for story progression
			if (UMGCareerSubsystem* CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>())
			{
				CareerSubsystem->OnChapterAdvanced.AddDynamic(this, &AMGPlayerController::OnCareerChapterAdvanced);
				CareerSubsystem->OnMilestoneReached.AddDynamic(this, &AMGPlayerController::OnCareerMilestoneReached);
				CareerSubsystem->OnObjectiveCompleted.AddDynamic(this, &AMGPlayerController::OnCareerObjectiveCompleted);
			}

			// Bind to rivals subsystem for rivalry events
			if (UMGRivalsSubsystem* RivalsSubsystem = GI->GetSubsystem<UMGRivalsSubsystem>())
			{
				RivalsSubsystem->OnNewRivalDiscovered.AddDynamic(this, &AMGPlayerController::OnNewRivalDiscovered);
				RivalsSubsystem->OnRivalDefeated.AddDynamic(this, &AMGPlayerController::OnRivalDefeated);
				RivalsSubsystem->OnNemesisDesignated.AddDynamic(this, &AMGPlayerController::OnNemesisDesignated);
			}
		}
	}
}

void AMGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from subsystem delegates
	if (UWorld* World = GetWorld())
	{
		if (UMGCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UMGCheckpointSubsystem>())
		{
			CheckpointSubsystem->OnWrongWay.RemoveDynamic(this, &AMGPlayerController::OnWrongWayDetected);
			CheckpointSubsystem->OnCheckpointPassed.RemoveDynamic(this, &AMGPlayerController::OnCheckpointPassed);
			CheckpointSubsystem->OnLapCompleted.RemoveDynamic(this, &AMGPlayerController::OnLapCompleted);
		}

		if (UMGNearMissSubsystem* NearMissSubsystem = World->GetSubsystem<UMGNearMissSubsystem>())
		{
			NearMissSubsystem->OnNearMissOccurred.RemoveDynamic(this, &AMGPlayerController::OnNearMissDetected);
		}

		if (UMGVehicleWearSubsystem* WearSubsystem = World->GetSubsystem<UMGVehicleWearSubsystem>())
		{
			WearSubsystem->OnEngineOverheat.RemoveDynamic(this, &AMGPlayerController::OnEngineOverheat);
		}

		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			WeatherSubsystem->OnWeatherTransitionStarted.RemoveDynamic(this, &AMGPlayerController::OnWeatherTransitionStarted);
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGDriftSubsystem* DriftSubsystem = GI->GetSubsystem<UMGDriftSubsystem>())
		{
			DriftSubsystem->OnDriftEnded.RemoveDynamic(this, &AMGPlayerController::OnDriftEnded);
		}

		if (UMGAirtimeSubsystem* AirtimeSubsystem = GI->GetSubsystem<UMGAirtimeSubsystem>())
		{
			AirtimeSubsystem->OnJumpEnded.RemoveDynamic(this, &AMGPlayerController::OnJumpEnded);
			AirtimeSubsystem->OnTrickCompleted.RemoveDynamic(this, &AMGPlayerController::OnTrickCompleted);
		}

		if (UMGFuelSubsystem* FuelSubsystem = GI->GetSubsystem<UMGFuelSubsystem>())
		{
			FuelSubsystem->OnFuelAlert.RemoveDynamic(this, &AMGPlayerController::OnFuelAlert);
			FuelSubsystem->OnFuelEmpty.RemoveDynamic(this, &AMGPlayerController::OnFuelEmpty);
		}

		if (UMGTireSubsystem* TireSubsystem = GI->GetSubsystem<UMGTireSubsystem>())
		{
			TireSubsystem->OnTirePunctured.RemoveDynamic(this, &AMGPlayerController::OnTirePunctured);
			TireSubsystem->OnTireConditionChanged.RemoveDynamic(this, &AMGPlayerController::OnTireConditionChanged);
		}

		if (UMGCollisionSubsystem* CollisionSubsystem = GI->GetSubsystem<UMGCollisionSubsystem>())
		{
			CollisionSubsystem->OnTakedownDealt.RemoveDynamic(this, &AMGPlayerController::OnTakedownDealt);
			CollisionSubsystem->OnTakedownChain.RemoveDynamic(this, &AMGPlayerController::OnTakedownChain);
			CollisionSubsystem->OnRevengeComplete.RemoveDynamic(this, &AMGPlayerController::OnRevengeComplete);
			CollisionSubsystem->OnDamageStateChanged.RemoveDynamic(this, &AMGPlayerController::OnDamageStateChanged);
		}

		if (UMGPitStopSubsystem* PitStopSubsystem = GI->GetSubsystem<UMGPitStopSubsystem>())
		{
			PitStopSubsystem->OnPitStopCompleted.RemoveDynamic(this, &AMGPlayerController::OnPitStopCompleted);
			PitStopSubsystem->OnPitLaneViolation.RemoveDynamic(this, &AMGPlayerController::OnPitLaneViolation);
		}

		if (UMGBonusSubsystem* BonusSubsystem = GI->GetSubsystem<UMGBonusSubsystem>())
		{
			BonusSubsystem->OnBonusCollected.RemoveDynamic(this, &AMGPlayerController::OnBonusCollected);
			BonusSubsystem->OnComboBonusTriggered.RemoveDynamic(this, &AMGPlayerController::OnComboBonusTriggered);
			BonusSubsystem->OnSecretBonusFound.RemoveDynamic(this, &AMGPlayerController::OnSecretBonusFound);
		}

		if (UMGPursuitSubsystem* PursuitSubsystem = GI->GetSubsystem<UMGPursuitSubsystem>())
		{
			PursuitSubsystem->OnPursuitStarted.RemoveDynamic(this, &AMGPlayerController::OnPursuitStarted);
			PursuitSubsystem->OnPursuitEnded.RemoveDynamic(this, &AMGPlayerController::OnPursuitEnded);
			PursuitSubsystem->OnUnitDisabled.RemoveDynamic(this, &AMGPlayerController::OnUnitDisabled);
			PursuitSubsystem->OnRoadblockEvaded.RemoveDynamic(this, &AMGPlayerController::OnRoadblockEvaded);
		}

		if (UMGSpeedtrapSubsystem* SpeedtrapSubsystem = GI->GetSubsystem<UMGSpeedtrapSubsystem>())
		{
			SpeedtrapSubsystem->OnSpeedtrapRecorded.RemoveDynamic(this, &AMGPlayerController::OnSpeedtrapRecorded);
			SpeedtrapSubsystem->OnSpeedtrapNewPersonalBest.RemoveDynamic(this, &AMGPlayerController::OnSpeedtrapNewPersonalBest);
			SpeedtrapSubsystem->OnSpeedtrapDiscovered.RemoveDynamic(this, &AMGPlayerController::OnSpeedtrapDiscovered);
		}

		if (UMGDestructionSubsystem* DestructionSubsystem = GI->GetSubsystem<UMGDestructionSubsystem>())
		{
			DestructionSubsystem->OnDestructibleDestroyed.RemoveDynamic(this, &AMGPlayerController::OnDestructibleDestroyed);
			DestructionSubsystem->OnDestructionComboUpdated.RemoveDynamic(this, &AMGPlayerController::OnDestructionComboUpdated);
			DestructionSubsystem->OnSpectacularDestruction.RemoveDynamic(this, &AMGPlayerController::OnSpectacularDestruction);
		}

		if (UMGAerodynamicsSubsystem* AeroSubsystem = GI->GetSubsystem<UMGAerodynamicsSubsystem>())
		{
			AeroSubsystem->OnSlipstreamEntered.RemoveDynamic(this, &AMGPlayerController::OnSlipstreamEntered);
			AeroSubsystem->OnSlingshotReady.RemoveDynamic(this, &AMGPlayerController::OnSlingshotReady);
			AeroSubsystem->OnSlingshotUsed.RemoveDynamic(this, &AMGPlayerController::OnSlingshotUsed);
		}

		if (UMGScoringSubsystem* ScoringSubsystem = GI->GetSubsystem<UMGScoringSubsystem>())
		{
			ScoringSubsystem->OnScoreEvent.RemoveDynamic(this, &AMGPlayerController::OnScoreEvent);
			ScoringSubsystem->OnChainExtended.RemoveDynamic(this, &AMGPlayerController::OnChainExtended);
		}

		if (UMGAchievementSubsystem* AchievementSubsystem = GI->GetSubsystem<UMGAchievementSubsystem>())
		{
			AchievementSubsystem->OnAchievementUnlocked.RemoveDynamic(this, &AMGPlayerController::OnAchievementUnlocked);
		}

		if (UMGStreakSubsystem* StreakSubsystem = GI->GetSubsystem<UMGStreakSubsystem>())
		{
			StreakSubsystem->OnStreakTierUp.RemoveDynamic(this, &AMGPlayerController::OnStreakTierUp);
			StreakSubsystem->OnNewStreakRecord.RemoveDynamic(this, &AMGPlayerController::OnNewStreakRecord);
		}

		if (UMGPrestigeSubsystem* PrestigeSubsystem = GI->GetSubsystem<UMGPrestigeSubsystem>())
		{
			PrestigeSubsystem->OnPrestigeRankUp.RemoveDynamic(this, &AMGPlayerController::OnPrestigeRankUp);
			PrestigeSubsystem->OnPrestigeLevelUp.RemoveDynamic(this, &AMGPlayerController::OnPrestigeLevelUp);
		}

		if (UMGNitroBoostSubsystem* NitroSubsystem = GI->GetSubsystem<UMGNitroBoostSubsystem>())
		{
			NitroSubsystem->OnNitroDepleted.RemoveDynamic(this, &AMGPlayerController::OnNitroDepleted);
			NitroSubsystem->OnNitroOverheat.RemoveDynamic(this, &AMGPlayerController::OnNitroOverheat);
		}

		if (UMGStuntSubsystem* StuntSubsystem = GI->GetSubsystem<UMGStuntSubsystem>())
		{
			StuntSubsystem->OnStuntCompleted.RemoveDynamic(this, &AMGPlayerController::OnStuntCompleted);
		}

		if (UMGTakedownSubsystem* TakedownSubsystem = GI->GetSubsystem<UMGTakedownSubsystem>())
		{
			TakedownSubsystem->OnRampageActivated.RemoveDynamic(this, &AMGPlayerController::OnRampageActivated);
		}

		if (UMGPowerupSubsystem* PowerupSubsystem = GI->GetSubsystem<UMGPowerupSubsystem>())
		{
			PowerupSubsystem->OnPowerupCollected.RemoveDynamic(this, &AMGPlayerController::OnPowerupCollected);
			PowerupSubsystem->OnPowerupHit.RemoveDynamic(this, &AMGPlayerController::OnPowerupHit);
		}

		if (UMGCautionSubsystem* CautionSubsystem = GI->GetSubsystem<UMGCautionSubsystem>())
		{
			CautionSubsystem->OnCautionDeployed.RemoveDynamic(this, &AMGPlayerController::OnCautionDeployed);
			CautionSubsystem->OnCautionEnded.RemoveDynamic(this, &AMGPlayerController::OnCautionEnded);
			CautionSubsystem->OnSafetyCarDeployed.RemoveDynamic(this, &AMGPlayerController::OnSafetyCarDeployed);
			CautionSubsystem->OnSafetyCarIn.RemoveDynamic(this, &AMGPlayerController::OnSafetyCarIn);
		}

		if (UMGPenaltySubsystem* PenaltySubsystem = GI->GetSubsystem<UMGPenaltySubsystem>())
		{
			PenaltySubsystem->OnPenaltyIssued.RemoveDynamic(this, &AMGPlayerController::OnPenaltyIssued);
			PenaltySubsystem->OnPenaltyServed.RemoveDynamic(this, &AMGPlayerController::OnPenaltyServed);
		}

		if (UMGHeatLevelSubsystem* HeatSubsystem = GI->GetSubsystem<UMGHeatLevelSubsystem>())
		{
			HeatSubsystem->OnHeatLevelChanged.RemoveDynamic(this, &AMGPlayerController::OnHeatLevelChanged);
			HeatSubsystem->OnPursuitEvaded.RemoveDynamic(this, &AMGPlayerController::OnPursuitEvaded);
			HeatSubsystem->OnPlayerBusted.RemoveDynamic(this, &AMGPlayerController::OnPlayerBusted);
			HeatSubsystem->OnHelicopterDeployed.RemoveDynamic(this, &AMGPlayerController::OnHelicopterDeployed);
		}

		if (UMGBountySubsystem* BountySubsystem = GI->GetSubsystem<UMGBountySubsystem>())
		{
			BountySubsystem->OnBountyCompleted.RemoveDynamic(this, &AMGPlayerController::OnBountyCompleted);
			BountySubsystem->OnBountyFailed.RemoveDynamic(this, &AMGPlayerController::OnBountyFailed);
			BountySubsystem->OnBountyObjectiveCompleted.RemoveDynamic(this, &AMGPlayerController::OnBountyObjectiveCompleted);
		}

		if (UMGRaceDirectorSubsystem* DirectorSubsystem = GI->GetSubsystem<UMGRaceDirectorSubsystem>())
		{
			DirectorSubsystem->OnDramaticMoment.RemoveDynamic(this, &AMGPlayerController::OnDramaticMoment);
			DirectorSubsystem->OnLeadChange.RemoveDynamic(this, &AMGPlayerController::OnLeadChange);
		}

		if (UMGLicenseSubsystem* LicenseSubsystem = GI->GetSubsystem<UMGLicenseSubsystem>())
		{
			LicenseSubsystem->OnLicenseUpgraded.RemoveDynamic(this, &AMGPlayerController::OnLicenseUpgraded);
			LicenseSubsystem->OnTestCompleted.RemoveDynamic(this, &AMGPlayerController::OnLicenseTestCompleted);
		}

		if (UMGContractSubsystem* ContractSubsystem = GI->GetSubsystem<UMGContractSubsystem>())
		{
			ContractSubsystem->OnContractCompleted.RemoveDynamic(this, &AMGPlayerController::OnContractCompleted);
			ContractSubsystem->OnObjectiveCompleted.RemoveDynamic(this, &AMGPlayerController::OnContractObjectiveCompleted);
			ContractSubsystem->OnSponsorLevelUp.RemoveDynamic(this, &AMGPlayerController::OnSponsorLevelUp);
		}

		if (UMGChallengeSubsystem* ChallengeSubsystem = GI->GetSubsystem<UMGChallengeSubsystem>())
		{
			ChallengeSubsystem->OnChallengeCompleted.RemoveDynamic(this, &AMGPlayerController::OnChallengeCompleted);
		}

		if (UMGCurrencySubsystem* CurrencySubsystem = GI->GetSubsystem<UMGCurrencySubsystem>())
		{
			CurrencySubsystem->OnCurrencyChanged.RemoveDynamic(this, &AMGPlayerController::OnCurrencyChanged);
			CurrencySubsystem->OnMultiplierActivated.RemoveDynamic(this, &AMGPlayerController::OnMultiplierActivated);
		}

		if (UMGDailyRewardsSubsystem* DailyRewardsSubsystem = GI->GetSubsystem<UMGDailyRewardsSubsystem>())
		{
			DailyRewardsSubsystem->OnDailyRewardClaimed.RemoveDynamic(this, &AMGPlayerController::OnDailyRewardClaimed);
			DailyRewardsSubsystem->OnMilestoneReached.RemoveDynamic(this, &AMGPlayerController::OnStreakMilestoneReached);
		}

		if (UMGReputationSubsystem* RepSubsystem = GI->GetSubsystem<UMGReputationSubsystem>())
		{
			RepSubsystem->OnTierReached.RemoveDynamic(this, &AMGPlayerController::OnReputationTierReached);
			RepSubsystem->OnUnlockEarned.RemoveDynamic(this, &AMGPlayerController::OnReputationUnlockEarned);
		}

		if (UMGGhostSubsystem* GhostSubsystem = GI->GetSubsystem<UMGGhostSubsystem>())
		{
			GhostSubsystem->OnNewPersonalBest.RemoveDynamic(this, &AMGPlayerController::OnGhostNewPersonalBest);
			GhostSubsystem->OnGhostComparison.RemoveDynamic(this, &AMGPlayerController::OnGhostComparison);
		}

		if (UMGShortcutSubsystem* ShortcutSubsystem = GI->GetSubsystem<UMGShortcutSubsystem>())
		{
			ShortcutSubsystem->OnShortcutDiscovered.RemoveDynamic(this, &AMGPlayerController::OnShortcutDiscovered);
			ShortcutSubsystem->OnShortcutCompleted.RemoveDynamic(this, &AMGPlayerController::OnShortcutCompleted);
			ShortcutSubsystem->OnShortcutMastered.RemoveDynamic(this, &AMGPlayerController::OnShortcutMastered);
			ShortcutSubsystem->OnSecretShortcutFound.RemoveDynamic(this, &AMGPlayerController::OnSecretShortcutFound);
		}

		if (UMGCareerSubsystem* CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>())
		{
			CareerSubsystem->OnChapterAdvanced.RemoveDynamic(this, &AMGPlayerController::OnCareerChapterAdvanced);
			CareerSubsystem->OnMilestoneReached.RemoveDynamic(this, &AMGPlayerController::OnCareerMilestoneReached);
			CareerSubsystem->OnObjectiveCompleted.RemoveDynamic(this, &AMGPlayerController::OnCareerObjectiveCompleted);
		}

		if (UMGRivalsSubsystem* RivalsSubsystem = GI->GetSubsystem<UMGRivalsSubsystem>())
		{
			RivalsSubsystem->OnNewRivalDiscovered.RemoveDynamic(this, &AMGPlayerController::OnNewRivalDiscovered);
			RivalsSubsystem->OnRivalDefeated.RemoveDynamic(this, &AMGPlayerController::OnRivalDefeated);
			RivalsSubsystem->OnNemesisDesignated.RemoveDynamic(this, &AMGPlayerController::OnNemesisDesignated);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Driving inputs
		if (AccelerateAction)
		{
			EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnAccelerate);
			EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnAccelerateReleased);
		}

		if (BrakeAction)
		{
			EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnBrake);
			EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnBrakeReleased);
		}

		if (SteerAction)
		{
			EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnSteer);
			EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnSteer);
		}

		if (HandbrakeAction)
		{
			EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AMGPlayerController::OnHandbrake);
			EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnHandbrakeReleased);
		}

		if (NitroAction)
		{
			EnhancedInputComponent->BindAction(NitroAction, ETriggerEvent::Started, this, &AMGPlayerController::OnNitro);
			EnhancedInputComponent->BindAction(NitroAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnNitroReleased);
		}

		if (ShiftUpAction)
		{
			EnhancedInputComponent->BindAction(ShiftUpAction, ETriggerEvent::Started, this, &AMGPlayerController::OnShiftUp);
		}

		if (ShiftDownAction)
		{
			EnhancedInputComponent->BindAction(ShiftDownAction, ETriggerEvent::Started, this, &AMGPlayerController::OnShiftDown);
		}

		if (LookBackAction)
		{
			EnhancedInputComponent->BindAction(LookBackAction, ETriggerEvent::Started, this, &AMGPlayerController::OnLookBack);
			EnhancedInputComponent->BindAction(LookBackAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnLookBackReleased);
		}

		if (HornAction)
		{
			EnhancedInputComponent->BindAction(HornAction, ETriggerEvent::Started, this, &AMGPlayerController::OnHorn);
			EnhancedInputComponent->BindAction(HornAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnHornReleased);
		}

		if (ResetVehicleAction)
		{
			EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Started, this, &AMGPlayerController::OnResetVehicle);
		}

		if (CycleCameraAction)
		{
			EnhancedInputComponent->BindAction(CycleCameraAction, ETriggerEvent::Started, this, &AMGPlayerController::OnCycleCamera);
		}

		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AMGPlayerController::OnPause);
		}

		if (MapAction)
		{
			EnhancedInputComponent->BindAction(MapAction, ETriggerEvent::Started, this, &AMGPlayerController::OnMap);
		}

		// Quick chat
		if (QuickChat1Action)
		{
			EnhancedInputComponent->BindAction(QuickChat1Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat1);
		}
		if (QuickChat2Action)
		{
			EnhancedInputComponent->BindAction(QuickChat2Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat2);
		}
		if (QuickChat3Action)
		{
			EnhancedInputComponent->BindAction(QuickChat3Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat3);
		}
		if (QuickChat4Action)
		{
			EnhancedInputComponent->BindAction(QuickChat4Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat4);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnLook);
		}

		if (RewindAction)
		{
			EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Started, this, &AMGPlayerController::OnRewind);
		}
	}
}

void AMGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Check if it's a vehicle
	if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(InPawn))
	{
		ControlledVehicle = Vehicle;
		SetInputMode(EMGInputMode::Driving);
		OnVehiclePossessed.Broadcast(Vehicle);
	}
}

void AMGPlayerController::OnUnPossess()
{
	if (ControlledVehicle)
	{
		ControlledVehicle = nullptr;
		OnVehicleUnpossessed.Broadcast();
	}

	Super::OnUnPossess();
}

void AMGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController() && CanDrive())
	{
		ApplyVehicleInput();

		// Send input to server
		if (!HasAuthority())
		{
			ServerUpdateVehicleInput(VehicleInput);
		}
	}
}

void AMGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGPlayerController, ControlledVehicle);
	DOREPLIFETIME(AMGPlayerController, VehicleInput);
	DOREPLIFETIME(AMGPlayerController, bRaceStarted);
}

void AMGPlayerController::SetInputMode(EMGInputMode NewMode)
{
	if (CurrentInputMode != NewMode)
	{
		CurrentInputMode = NewMode;
		UpdateInputMappingContext();
		OnInputModeChanged.Broadcast(NewMode);
	}
}

bool AMGPlayerController::CanDrive() const
{
	return CurrentInputMode == EMGInputMode::Driving &&
	       bRaceStarted &&
	       ControlledVehicle != nullptr;
}

void AMGPlayerController::RequestVehicleReset()
{
	if (HasAuthority())
	{
		OnResetVehicleRequested.Broadcast();
	}
	else
	{
		ServerRequestVehicleReset();
	}
}

void AMGPlayerController::CycleCamera()
{
	CurrentCameraIndex = (CurrentCameraIndex + 1) % NumCameras;
	// Camera switching is handled by the vehicle pawn
}

void AMGPlayerController::EnterSpectatorMode()
{
	SetInputMode(EMGInputMode::Spectating);

	// Find first available spectate target
	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() > 0)
	{
		SpectateTarget = Targets[0];
	}
}

void AMGPlayerController::ExitSpectatorMode()
{
	SpectateTarget = nullptr;
	SetInputMode(EMGInputMode::Driving);
}

void AMGPlayerController::SpectateNextPlayer()
{
	if (CurrentInputMode != EMGInputMode::Spectating)
	{
		return;
	}

	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = SpectateTarget ? Targets.IndexOfByKey(SpectateTarget) : -1;
	int32 NextIndex = (CurrentIndex + 1) % Targets.Num();
	SpectateTarget = Targets[NextIndex];
}

void AMGPlayerController::SpectatePreviousPlayer()
{
	if (CurrentInputMode != EMGInputMode::Spectating)
	{
		return;
	}

	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = SpectateTarget ? Targets.IndexOfByKey(SpectateTarget) : 0;
	int32 PrevIndex = (CurrentIndex - 1 + Targets.Num()) % Targets.Num();
	SpectateTarget = Targets[PrevIndex];
}

void AMGPlayerController::SendQuickChat(int32 Index)
{
	if (Index >= 1 && Index <= 4)
	{
		ServerSendQuickChat(Index);
		OnQuickChatSent.Broadcast(Index);
	}
}

void AMGPlayerController::ServerSendQuickChat_Implementation(int32 Index)
{
	// Broadcast to all players via game state or chat subsystem
	// Implementation depends on chat system
}

void AMGPlayerController::TogglePauseMenu()
{
	bPauseMenuOpen = !bPauseMenuOpen;

	if (bPauseMenuOpen)
	{
		SetInputMode(EMGInputMode::Menu);
	}
	else
	{
		SetInputMode(EMGInputMode::Driving);
	}
}

void AMGPlayerController::OpenMap()
{
	// Open map UI - handled by UI subsystem
}

float AMGPlayerController::GetNetworkLatency() const
{
	if (PlayerState)
	{
		return PlayerState->GetPingInMilliseconds();
	}
	return 0.0f;
}

// ==========================================
// INPUT HANDLERS
// ==========================================

void AMGPlayerController::OnAccelerate(const FInputActionValue& Value)
{
	VehicleInput.Throttle = Value.Get<float>();
}

void AMGPlayerController::OnAccelerateReleased(const FInputActionValue& Value)
{
	VehicleInput.Throttle = 0.0f;
}

void AMGPlayerController::OnBrake(const FInputActionValue& Value)
{
	VehicleInput.Brake = Value.Get<float>();
}

void AMGPlayerController::OnBrakeReleased(const FInputActionValue& Value)
{
	VehicleInput.Brake = 0.0f;
}

void AMGPlayerController::OnSteer(const FInputActionValue& Value)
{
	VehicleInput.Steering = Value.Get<float>();
}

void AMGPlayerController::OnHandbrake(const FInputActionValue& Value)
{
	VehicleInput.bHandbrake = true;
}

void AMGPlayerController::OnHandbrakeReleased(const FInputActionValue& Value)
{
	VehicleInput.bHandbrake = false;
}

void AMGPlayerController::OnNitro(const FInputActionValue& Value)
{
	VehicleInput.bNitro = true;
}

void AMGPlayerController::OnNitroReleased(const FInputActionValue& Value)
{
	VehicleInput.bNitro = false;
}

void AMGPlayerController::OnShiftUp(const FInputActionValue& Value)
{
	VehicleInput.GearShift = 1;
}

void AMGPlayerController::OnShiftDown(const FInputActionValue& Value)
{
	VehicleInput.GearShift = -1;
}

void AMGPlayerController::OnLookBack(const FInputActionValue& Value)
{
	VehicleInput.bLookBack = true;
}

void AMGPlayerController::OnLookBackReleased(const FInputActionValue& Value)
{
	VehicleInput.bLookBack = false;
}

void AMGPlayerController::OnHorn(const FInputActionValue& Value)
{
	VehicleInput.bHorn = true;
}

void AMGPlayerController::OnHornReleased(const FInputActionValue& Value)
{
	VehicleInput.bHorn = false;
}

void AMGPlayerController::OnResetVehicle(const FInputActionValue& Value)
{
	RequestVehicleReset();
}

void AMGPlayerController::OnCycleCamera(const FInputActionValue& Value)
{
	CycleCamera();
}

void AMGPlayerController::OnPause(const FInputActionValue& Value)
{
	TogglePauseMenu();
}

void AMGPlayerController::OnMap(const FInputActionValue& Value)
{
	OpenMap();
}

void AMGPlayerController::OnQuickChat1(const FInputActionValue& Value)
{
	SendQuickChat(1);
}

void AMGPlayerController::OnQuickChat2(const FInputActionValue& Value)
{
	SendQuickChat(2);
}

void AMGPlayerController::OnQuickChat3(const FInputActionValue& Value)
{
	SendQuickChat(3);
}

void AMGPlayerController::OnQuickChat4(const FInputActionValue& Value)
{
	SendQuickChat(4);
}

void AMGPlayerController::OnLook(const FInputActionValue& Value)
{
	FVector2D LookValue = Value.Get<FVector2D>();
	VehicleInput.LookDirection = FVector(LookValue.X, LookValue.Y, 0.0f).GetSafeNormal();
}

void AMGPlayerController::OnRewind(const FInputActionValue& Value)
{
	// Rewind functionality - integrated with replay subsystem
}

// ==========================================
// INTERNAL
// ==========================================

void AMGPlayerController::ApplyVehicleInput()
{
	// Input application to vehicle pawn is handled in the pawn's tick
	// VehicleInput struct is already populated by input handlers

	// Clear one-shot inputs
	VehicleInput.GearShift = 0;
}

void AMGPlayerController::UpdateInputMappingContext()
{
	if (!IsLocalController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Remove current contexts
		if (DrivingMappingContext)
		{
			Subsystem->RemoveMappingContext(DrivingMappingContext);
		}
		if (MenuMappingContext)
		{
			Subsystem->RemoveMappingContext(MenuMappingContext);
		}

		// Add appropriate context
		switch (CurrentInputMode)
		{
		case EMGInputMode::Driving:
			if (DrivingMappingContext)
			{
				Subsystem->AddMappingContext(DrivingMappingContext, 1);
			}
			break;

		case EMGInputMode::Menu:
		case EMGInputMode::PhotoMode:
		case EMGInputMode::Chat:
			if (MenuMappingContext)
			{
				Subsystem->AddMappingContext(MenuMappingContext, 1);
			}
			break;

		case EMGInputMode::Spectating:
		case EMGInputMode::Replay:
			// Spectating uses a subset of driving controls
			if (DrivingMappingContext)
			{
				Subsystem->AddMappingContext(DrivingMappingContext, 1);
			}
			break;
		}
	}
}

TArray<APlayerState*> AMGPlayerController::GetSpectateTargets() const
{
	TArray<APlayerState*> Targets;

	UWorld* World = GetWorld();
	if (!World)
	{
		return Targets;
	}

	if (AGameStateBase* GS = World->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			// Don't include ourselves
			if (PS && PS != PlayerState)
			{
				Targets.Add(PS);
			}
		}
	}

	return Targets;
}

void AMGPlayerController::ServerUpdateVehicleInput_Implementation(const FMGVehicleInputState& Input)
{
	VehicleInput = Input;
}

void AMGPlayerController::ServerRequestVehicleReset_Implementation()
{
	OnResetVehicleRequested.Broadcast();
}

void AMGPlayerController::ClientOnRaceStarted_Implementation()
{
	bRaceStarted = true;
	SetInputMode(EMGInputMode::Driving);
}

void AMGPlayerController::ClientOnRaceEnded_Implementation()
{
	bRaceStarted = false;
	SetInputMode(EMGInputMode::Menu);
}

void AMGPlayerController::OnWrongWayDetected(bool bIsWrongWay)
{
	// Forward wrong-way status to HUD subsystem
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->ShowWrongWayWarning(bIsWrongWay);
		}
	}
}

void AMGPlayerController::OnNearMissDetected(const FMGNearMissEvent& Event, int32 TotalPoints)
{
	// Forward near miss to HUD subsystem
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->ShowNearMissBonus(Event.BasePoints);
		}
	}
}

void AMGPlayerController::OnDriftEnded(const FMGDriftResult& Result)
{
	// Forward drift score to HUD subsystem
	if (Result.TotalPoints > 0 && !Result.bFailed)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				HUDSubsystem->ShowDriftScorePopup(Result.TotalPoints, Result.Multiplier);
			}
		}
	}
}

void AMGPlayerController::OnJumpEnded(const FString& PlayerId, const FMGJumpResult& Result)
{
	// Forward jump result to HUD subsystem
	if (Result.TotalScore > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				HUDSubsystem->ShowAirtimePopup(Result.AirtimeDuration, Result.TotalScore);
			}
		}
	}
}

void AMGPlayerController::OnTrickCompleted(const FString& PlayerId, EMGTrickType Trick, int32 Score)
{
	// Forward trick to HUD subsystem
	if (Score > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				// Convert trick type to display name
				FText TrickName;
				switch (Trick)
				{
					case EMGTrickType::Flip: TrickName = FText::FromString(TEXT("FLIP")); break;
					case EMGTrickType::Barrel: TrickName = FText::FromString(TEXT("BARREL ROLL")); break;
					case EMGTrickType::Spin: TrickName = FText::FromString(TEXT("SPIN")); break;
					case EMGTrickType::Corkscrew: TrickName = FText::FromString(TEXT("CORKSCREW")); break;
					default: TrickName = FText::FromString(TEXT("TRICK")); break;
				}
				HUDSubsystem->ShowTrickPopup(TrickName, Score);
			}
		}
	}
}

void AMGPlayerController::OnFuelAlert(FName VehicleID, EMGFuelAlert Alert)
{
	// Only show alerts for the player's own vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText AlertMessage;
			FLinearColor AlertColor;

			switch (Alert)
			{
				case EMGFuelAlert::LowFuel:
					AlertMessage = FText::FromString(TEXT("LOW FUEL"));
					AlertColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
					break;
				case EMGFuelAlert::CriticalFuel:
					AlertMessage = FText::FromString(TEXT("CRITICAL FUEL!"));
					AlertColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f); // Red
					break;
				default:
					return;
			}

			HUDSubsystem->ShowNotification(AlertMessage, 3.0f, AlertColor);
		}
	}
}

void AMGPlayerController::OnFuelEmpty(FName VehicleID)
{
	// Only show alerts for the player's own vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText AlertMessage = FText::FromString(TEXT("OUT OF FUEL!"));
			FLinearColor AlertColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
			HUDSubsystem->ShowNotification(AlertMessage, 5.0f, AlertColor);
		}
	}
}

void AMGPlayerController::OnTirePunctured(FName VehicleID, EMGTirePosition Position)
{
	// Only show alerts for the player's own vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString PositionStr;
			switch (Position)
			{
				case EMGTirePosition::FrontLeft: PositionStr = TEXT("FRONT LEFT"); break;
				case EMGTirePosition::FrontRight: PositionStr = TEXT("FRONT RIGHT"); break;
				case EMGTirePosition::RearLeft: PositionStr = TEXT("REAR LEFT"); break;
				case EMGTirePosition::RearRight: PositionStr = TEXT("REAR RIGHT"); break;
				default: PositionStr = TEXT(""); break;
			}

			FText AlertMessage = FText::FromString(FString::Printf(TEXT("PUNCTURE! %s TIRE"), *PositionStr));
			FLinearColor AlertColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f);
			HUDSubsystem->ShowNotification(AlertMessage, 4.0f, AlertColor);
		}
	}
}

void AMGPlayerController::OnTireConditionChanged(FName VehicleID, EMGTirePosition Position, EMGTireCondition NewCondition)
{
	// Only show alerts for the player's own vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	// Only warn on critical condition
	if (NewCondition != EMGTireCondition::Critical)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText AlertMessage = FText::FromString(TEXT("TIRE WEAR CRITICAL!"));
			FLinearColor AlertColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
			HUDSubsystem->ShowNotification(AlertMessage, 3.0f, AlertColor);
		}
	}
}

void AMGPlayerController::OnTakedownDealt(const FString& AttackerId, const FMGTakedownEvent& Takedown)
{
	// Only show for the player's own takedowns
	if (!ControlledVehicle || AttackerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText TakedownMessage;
			FLinearColor TakedownColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange

			switch (Takedown.Type)
			{
				case EMGTakedownType::Shunt:
					TakedownMessage = FText::FromString(TEXT("SHUNT TAKEDOWN!"));
					break;
				case EMGTakedownType::Slam:
					TakedownMessage = FText::FromString(TEXT("SLAM TAKEDOWN!"));
					break;
				case EMGTakedownType::PIT:
					TakedownMessage = FText::FromString(TEXT("PIT MANEUVER!"));
					break;
				case EMGTakedownType::Vertical:
					TakedownMessage = FText::FromString(TEXT("VERTICAL TAKEDOWN!"));
					TakedownColor = FLinearColor(1.0f, 0.2f, 0.8f, 1.0f); // Purple
					break;
				case EMGTakedownType::Traffic:
					TakedownMessage = FText::FromString(TEXT("TRAFFIC TAKEDOWN!"));
					break;
				case EMGTakedownType::Aftertouch:
					TakedownMessage = FText::FromString(TEXT("AFTERTOUCH TAKEDOWN!"));
					TakedownColor = FLinearColor(0.2f, 1.0f, 0.8f, 1.0f); // Cyan
					break;
				default:
					TakedownMessage = FText::FromString(TEXT("TAKEDOWN!"));
					break;
			}

			HUDSubsystem->ShowNotification(TakedownMessage, 3.0f, TakedownColor);
		}
	}
}

void AMGPlayerController::OnTakedownChain(const FString& PlayerId, int32 ChainCount, float ChainMultiplier)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (ChainCount < 2)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ChainMessage = FText::FromString(FString::Printf(TEXT("TAKEDOWN x%d! (%.1fx)"), ChainCount, ChainMultiplier));
			FLinearColor ChainColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(ChainMessage, 2.5f, ChainColor);
		}
	}
}

void AMGPlayerController::OnRevengeComplete(const FString& AttackerId, const FString& OriginalAttackerId)
{
	// Only show for the player
	if (!ControlledVehicle || AttackerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText RevengeMessage = FText::FromString(TEXT("REVENGE!"));
			FLinearColor RevengeColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(RevengeMessage, 3.0f, RevengeColor);
		}
	}
}

void AMGPlayerController::OnPitStopCompleted(FName VehicleID, const FMGPitStopResult& Result)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Format pit stop time
			float TotalTime = Result.TotalTime;
			FString TimeStr = FString::Printf(TEXT("PIT STOP: %.2fs"), TotalTime);

			FText PitMessage = FText::FromString(TimeStr);
			FLinearColor PitColor = FLinearColor(0.2f, 0.8f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(PitMessage, 4.0f, PitColor);
		}
	}
}

void AMGPlayerController::OnPitLaneViolation(FName VehicleID, EMGPitLaneViolation Violation)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle || VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ViolationMessage;
			switch (Violation)
			{
				case EMGPitLaneViolation::Speeding:
					ViolationMessage = FText::FromString(TEXT("PIT LANE SPEEDING PENALTY!"));
					break;
				case EMGPitLaneViolation::UnsafeRelease:
					ViolationMessage = FText::FromString(TEXT("UNSAFE RELEASE PENALTY!"));
					break;
				case EMGPitLaneViolation::CrossingLine:
					ViolationMessage = FText::FromString(TEXT("PIT LINE CROSSING PENALTY!"));
					break;
				case EMGPitLaneViolation::WrongBox:
					ViolationMessage = FText::FromString(TEXT("WRONG PIT BOX!"));
					break;
				default:
					ViolationMessage = FText::FromString(TEXT("PIT LANE VIOLATION!"));
					break;
			}

			FLinearColor ViolationColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(ViolationMessage, 4.0f, ViolationColor);
		}
	}
}

void AMGPlayerController::OnBonusCollected(const FString& PlayerId, const FMGBonusDefinition& Bonus, int32 PointsAwarded)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (PointsAwarded <= 0)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText BonusMessage = FText::FromString(FString::Printf(TEXT("+%d %s"), PointsAwarded, *Bonus.DisplayName.ToString()));
			FLinearColor BonusColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Green-cyan
			HUDSubsystem->ShowNotification(BonusMessage, 2.0f, BonusColor);
		}
	}
}

void AMGPlayerController::OnComboBonusTriggered(const FString& PlayerId, int32 ComboLevel, int32 BonusPoints)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ComboMessage = FText::FromString(FString::Printf(TEXT("COMBO x%d! +%d"), ComboLevel, BonusPoints));
			FLinearColor ComboColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(ComboMessage, 2.5f, ComboColor);
		}
	}
}

void AMGPlayerController::OnSecretBonusFound(const FString& PlayerId, const FString& SecretId)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SecretMessage = FText::FromString(TEXT("SECRET FOUND!"));
			FLinearColor SecretColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(SecretMessage, 3.0f, SecretColor);
		}
	}
}

void AMGPlayerController::OnPursuitStarted(const FString& PlayerId, EMGPursuitIntensity Intensity)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText PursuitMessage = FText::FromString(TEXT("PURSUIT INITIATED!"));
			FLinearColor PursuitColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(PursuitMessage, 3.0f, PursuitColor);
		}
	}
}

void AMGPlayerController::OnPursuitEnded(const FString& PlayerId, bool bEscaped, int32 FinalBounty)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText EndMessage;
			FLinearColor EndColor;

			if (bEscaped)
			{
				EndMessage = FText::FromString(FString::Printf(TEXT("ESCAPED! +$%d"), FinalBounty));
				EndColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
			}
			else
			{
				EndMessage = FText::FromString(TEXT("BUSTED!"));
				EndColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			}

			HUDSubsystem->ShowNotification(EndMessage, 4.0f, EndColor);
		}
	}
}

void AMGPlayerController::OnUnitDisabled(const FString& PlayerId, const FMGPursuitUnit& Unit)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText DisabledMessage = FText::FromString(TEXT("UNIT DISABLED!"));
			FLinearColor DisabledColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
			HUDSubsystem->ShowNotification(DisabledMessage, 2.0f, DisabledColor);
		}
	}
}

void AMGPlayerController::OnRoadblockEvaded(const FString& PlayerId, const FString& RoadblockId)
{
	// Only show for the player
	if (!ControlledVehicle || PlayerId != ControlledVehicle->GetName())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText EvadedMessage = FText::FromString(TEXT("ROADBLOCK EVADED!"));
			FLinearColor EvadedColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(EvadedMessage, 2.5f, EvadedColor);
		}
	}
}

void AMGPlayerController::OnSpeedtrapRecorded(const FString& SpeedtrapId, float RecordedValue, EMGSpeedtrapRating Rating)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Format speed (assume KPH)
			int32 SpeedInt = FMath::RoundToInt(RecordedValue);

			FString RatingStr;
			FLinearColor RatingColor;

			switch (Rating)
			{
				case EMGSpeedtrapRating::Bronze:
					RatingStr = TEXT("BRONZE");
					RatingColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);
					break;
				case EMGSpeedtrapRating::Silver:
					RatingStr = TEXT("SILVER");
					RatingColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
					break;
				case EMGSpeedtrapRating::Gold:
					RatingStr = TEXT("GOLD");
					RatingColor = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
					break;
				case EMGSpeedtrapRating::Platinum:
					RatingStr = TEXT("PLATINUM");
					RatingColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
					break;
				default:
					RatingStr = TEXT("");
					RatingColor = FLinearColor::White;
					break;
			}

			FText SpeedMessage;
			if (!RatingStr.IsEmpty())
			{
				SpeedMessage = FText::FromString(FString::Printf(TEXT("SPEED: %d KPH - %s"), SpeedInt, *RatingStr));
			}
			else
			{
				SpeedMessage = FText::FromString(FString::Printf(TEXT("SPEED: %d KPH"), SpeedInt));
			}

			HUDSubsystem->ShowNotification(SpeedMessage, 3.0f, RatingColor);
		}
	}
}

void AMGPlayerController::OnSpeedtrapNewPersonalBest(const FString& SpeedtrapId, float OldBest, float NewBest)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			int32 NewBestInt = FMath::RoundToInt(NewBest);
			int32 ImprovementInt = FMath::RoundToInt(NewBest - OldBest);

			FText PBMessage = FText::FromString(FString::Printf(TEXT("NEW PERSONAL BEST! %d KPH (+%d)"), NewBestInt, ImprovementInt));
			FLinearColor PBColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
			HUDSubsystem->ShowNotification(PBMessage, 4.0f, PBColor);
		}
	}
}

void AMGPlayerController::OnSpeedtrapDiscovered(const FString& SpeedtrapId, int32 TotalDiscovered)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText DiscoveredMessage = FText::FromString(FString::Printf(TEXT("SPEEDTRAP DISCOVERED! (%d found)"), TotalDiscovered));
			FLinearColor DiscoveredColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(DiscoveredMessage, 3.0f, DiscoveredColor);
		}
	}
}

void AMGPlayerController::OnDestructibleDestroyed(const FString& PlayerId, const FMGDestructionEvent& Event)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Show points popup for destruction
			FText DestroyedMessage = FText::FromString(FString::Printf(TEXT("SMASH! +%d"), Event.PointsEarned));
			FLinearColor SmashColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange
			HUDSubsystem->ShowNotification(DestroyedMessage, 1.5f, SmashColor);
		}
	}
}

void AMGPlayerController::OnDestructionComboUpdated(const FString& PlayerId, int32 ComboCount, float Multiplier)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Show combo counter
			FText ComboMessage = FText::FromString(FString::Printf(TEXT("COMBO x%d (%.1fx)"), ComboCount, Multiplier));
			FLinearColor ComboColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow-orange
			HUDSubsystem->ShowNotification(ComboMessage, 2.0f, ComboColor);
		}
	}
}

void AMGPlayerController::OnSpectacularDestruction(const FString& PlayerId, int32 BonusPoints)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SpectacularMessage = FText::FromString(FString::Printf(TEXT("SPECTACULAR! +%d BONUS"), BonusPoints));
			FLinearColor SpectacularColor = FLinearColor(1.0f, 0.2f, 0.8f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(SpectacularMessage, 3.0f, SpectacularColor);
		}
	}
}

void AMGPlayerController::OnSlipstreamEntered(const FString& FollowerId, const FString& LeaderId, float Distance)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (FollowerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SlipstreamMessage = FText::FromString(TEXT("SLIPSTREAM!"));
			FLinearColor SlipstreamColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(SlipstreamMessage, 2.0f, SlipstreamColor);
		}
	}
}

void AMGPlayerController::OnSlingshotReady(const FString& VehicleId, float BoostAmount, float Duration)
{
	// Only show for local player's vehicle
	if (!ControlledVehicle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SlingshotMessage = FText::FromString(TEXT("SLINGSHOT READY!"));
			FLinearColor SlingshotColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-green
			HUDSubsystem->ShowNotification(SlingshotMessage, 2.0f, SlingshotColor);
		}
	}
}

void AMGPlayerController::OnSlingshotUsed(const FString& VehicleId, float SpeedGained)
{
	// Only show for local player's vehicle
	if (!ControlledVehicle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			int32 SpeedGainedInt = FMath::RoundToInt(SpeedGained);
			FText SlingshotMessage = FText::FromString(FString::Printf(TEXT("SLINGSHOT! +%d KPH"), SpeedGainedInt));
			FLinearColor SlingshotColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(SlingshotMessage, 2.5f, SlingshotColor);
		}
	}
}

void AMGPlayerController::OnScoreEvent(const FString& PlayerId, const FMGScoreEvent& Event, int32 NewTotal)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	// Don't show small score events, they clutter the HUD
	if (Event.FinalPoints < 100)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ScoreMessage = FText::FromString(FString::Printf(TEXT("+%d"), Event.FinalPoints));
			FLinearColor ScoreColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
			HUDSubsystem->ShowNotification(ScoreMessage, 1.5f, ScoreColor);
		}
	}
}

void AMGPlayerController::OnChainExtended(const FString& PlayerId, int32 ChainLength, float Multiplier, int32 ChainPoints)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	// Only show significant chains
	if (ChainLength < 3)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ChainMessage = FText::FromString(FString::Printf(TEXT("CHAIN x%d! (%.1fx)"), ChainLength, Multiplier));
			FLinearColor ChainColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
			HUDSubsystem->ShowNotification(ChainMessage, 2.0f, ChainColor);
		}
	}
}

FString AMGPlayerController::GetLocalPlayerId() const
{
	if (PlayerState)
	{
		return PlayerState->GetPlayerName();
	}
	return FString();
}

void AMGPlayerController::OnAchievementUnlocked(const FMGAchievementDefinition& Achievement, int32 TierUnlocked)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString TierStr;
			if (TierUnlocked > 1)
			{
				TierStr = FString::Printf(TEXT(" (Tier %d)"), TierUnlocked);
			}

			FText AchievementMessage = FText::FromString(FString::Printf(TEXT("ACHIEVEMENT UNLOCKED: %s%s"), *Achievement.DisplayName.ToString(), *TierStr));
			FLinearColor AchievementColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(AchievementMessage, 5.0f, AchievementColor);
		}
	}
}

void AMGPlayerController::OnStreakTierUp(const FString& PlayerId, EMGStreakType Type, EMGStreakTier NewTier)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString TierStr;
			FLinearColor TierColor;

			switch (NewTier)
			{
				case EMGStreakTier::Bronze:
					TierStr = TEXT("BRONZE");
					TierColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);
					break;
				case EMGStreakTier::Silver:
					TierStr = TEXT("SILVER");
					TierColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
					break;
				case EMGStreakTier::Gold:
					TierStr = TEXT("GOLD");
					TierColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
					break;
				case EMGStreakTier::Platinum:
					TierStr = TEXT("PLATINUM");
					TierColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
					break;
				case EMGStreakTier::Diamond:
					TierStr = TEXT("DIAMOND");
					TierColor = FLinearColor(0.6f, 0.85f, 1.0f, 1.0f);
					break;
				case EMGStreakTier::Champion:
					TierStr = TEXT("CHAMPION");
					TierColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
					break;
				case EMGStreakTier::Legend:
					TierStr = TEXT("LEGEND");
					TierColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
					break;
				default:
					TierStr = TEXT("");
					TierColor = FLinearColor::White;
					break;
			}

			if (!TierStr.IsEmpty())
			{
				FText TierMessage = FText::FromString(FString::Printf(TEXT("STREAK TIER UP! %s"), *TierStr));
				HUDSubsystem->ShowNotification(TierMessage, 3.0f, TierColor);
			}
		}
	}
}

void AMGPlayerController::OnNewStreakRecord(const FString& PlayerId, EMGStreakType Type)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText RecordMessage = FText::FromString(TEXT("NEW PERSONAL BEST STREAK!"));
			FLinearColor RecordColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Green
			HUDSubsystem->ShowNotification(RecordMessage, 3.0f, RecordColor);
		}
	}
}

void AMGPlayerController::OnPrestigeRankUp(const FString& PlayerId, EMGPrestigeRank OldRank, EMGPrestigeRank NewRank)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText RankUpMessage = FText::FromString(TEXT("PRESTIGE RANK UP!"));
			FLinearColor RankUpColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(RankUpMessage, 5.0f, RankUpColor);
		}
	}
}

void AMGPlayerController::OnPrestigeLevelUp(const FString& PlayerId, int32 OldLevel, int32 NewLevel)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText LevelUpMessage = FText::FromString(FString::Printf(TEXT("LEVEL UP! %d"), NewLevel));
			FLinearColor LevelUpColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(LevelUpMessage, 3.0f, LevelUpColor);
		}
	}
}

void AMGPlayerController::OnDamageStateChanged(const FString& VehicleId, EMGDamageState OldState, EMGDamageState NewState)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle || VehicleId != ControlledVehicle->GetName())
	{
		return;
	}

	// Only show warnings for significant damage transitions
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText DamageMessage;
			FLinearColor DamageColor;
			float Duration = 3.0f;

			switch (NewState)
			{
				case EMGDamageState::Damaged:
					DamageMessage = FText::FromString(TEXT("VEHICLE DAMAGED"));
					DamageColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
					Duration = 2.0f;
					break;

				case EMGDamageState::HeavyDamage:
					DamageMessage = FText::FromString(TEXT("HEAVY DAMAGE!"));
					DamageColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					Duration = 2.5f;
					break;

				case EMGDamageState::Critical:
					DamageMessage = FText::FromString(TEXT("CRITICAL DAMAGE! FIND A PIT STOP!"));
					DamageColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f); // Red-orange
					Duration = 4.0f;
					break;

				case EMGDamageState::Wrecked:
					DamageMessage = FText::FromString(TEXT("WRECKED!"));
					DamageColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					Duration = 5.0f;
					break;

				default:
					// Don't show notifications for minor states (Pristine, Scratched, Dented)
					return;
			}

			HUDSubsystem->ShowNotification(DamageMessage, Duration, DamageColor);
		}
	}
}

void AMGPlayerController::OnCheckpointPassed(const FMGCheckpointPassage& Passage, int32 CheckpointsRemaining, float DeltaTime)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Show split time delta
			FText SplitMessage;
			FLinearColor SplitColor;

			if (DeltaTime < 0.0f)
			{
				// Ahead of target
				SplitMessage = FText::FromString(FString::Printf(TEXT("SPLIT: -%.2fs"), FMath::Abs(DeltaTime)));
				SplitColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
			}
			else if (DeltaTime > 0.0f)
			{
				// Behind target
				SplitMessage = FText::FromString(FString::Printf(TEXT("SPLIT: +%.2fs"), DeltaTime));
				SplitColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
			}
			else
			{
				// Exactly on pace
				SplitMessage = FText::FromString(TEXT("SPLIT: 0.00s"));
				SplitColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
			}

			HUDSubsystem->ShowNotification(SplitMessage, 2.0f, SplitColor);
		}
	}
}

void AMGPlayerController::OnLapCompleted(const FMGLapData& LapData, int32 LapsRemaining, bool bIsBestLap)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Format lap time as MM:SS.mmm
			int32 Minutes = FMath::FloorToInt(LapData.LapTime / 60.0f);
			float Seconds = FMath::Fmod(LapData.LapTime, 60.0f);

			FText LapMessage;
			FLinearColor LapColor;

			if (bIsBestLap)
			{
				LapMessage = FText::FromString(FString::Printf(TEXT("BEST LAP! %d:%05.2f"), Minutes, Seconds));
				LapColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
			}
			else
			{
				LapMessage = FText::FromString(FString::Printf(TEXT("LAP %d: %d:%05.2f"), LapData.LapNumber, Minutes, Seconds));

				// Color based on delta from best
				if (LapData.DeltaFromBest < 0.5f)
				{
					LapColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-green (very close)
				}
				else if (LapData.DeltaFromBest < 2.0f)
				{
					LapColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow (decent)
				}
				else
				{
					LapColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange (behind)
				}
			}

			HUDSubsystem->ShowNotification(LapMessage, 4.0f, LapColor);

			// If there are laps remaining, show them
			if (LapsRemaining > 0)
			{
				FText RemainingMessage = FText::FromString(FString::Printf(TEXT("%d LAPS TO GO"), LapsRemaining));
				HUDSubsystem->ShowNotification(RemainingMessage, 3.0f, FLinearColor::White);
			}
			else
			{
				// Race finished
				FText FinishMessage = FText::FromString(TEXT("RACE COMPLETE!"));
				HUDSubsystem->ShowNotification(FinishMessage, 5.0f, FLinearColor(1.0f, 0.84f, 0.0f, 1.0f)); // Gold
			}
		}
	}
}

void AMGPlayerController::OnNitroDepleted()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText NitroMessage = FText::FromString(TEXT("NITRO DEPLETED"));
			FLinearColor NitroColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray
			HUDSubsystem->ShowNotification(NitroMessage, 1.5f, NitroColor);
		}
	}
}

void AMGPlayerController::OnNitroOverheat()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText OverheatMessage = FText::FromString(TEXT("NITRO OVERHEAT!"));
			FLinearColor OverheatColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
			HUDSubsystem->ShowNotification(OverheatMessage, 2.0f, OverheatColor);
		}
	}
}

void AMGPlayerController::OnStuntCompleted(const FMGStuntEvent& Event, int32 TotalPoints)
{
	if (TotalPoints <= 0)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText StuntMessage = FText::FromString(FString::Printf(TEXT("STUNT! +%d"), TotalPoints));
			FLinearColor StuntColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(StuntMessage, 2.5f, StuntColor);
		}
	}
}

void AMGPlayerController::OnRampageActivated(float Duration, float Multiplier)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText RampageMessage = FText::FromString(FString::Printf(TEXT("RAMPAGE! %.1fx MULTIPLIER"), Multiplier));
			FLinearColor RampageColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(RampageMessage, 3.0f, RampageColor);
		}
	}
}

void AMGPlayerController::OnPowerupCollected(const FString& PlayerId, EMGPowerupType PowerupType, int32 SlotIndex)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString PowerupName;
			switch (PowerupType)
			{
				case EMGPowerupType::SpeedBoost: PowerupName = TEXT("SPEED BOOST"); break;
				case EMGPowerupType::Shield: PowerupName = TEXT("SHIELD"); break;
				case EMGPowerupType::Nitro: PowerupName = TEXT("NITRO"); break;
				case EMGPowerupType::Missile: PowerupName = TEXT("MISSILE"); break;
				case EMGPowerupType::EMPBlast: PowerupName = TEXT("EMP BLAST"); break;
				case EMGPowerupType::OilSlick: PowerupName = TEXT("OIL SLICK"); break;
				case EMGPowerupType::SpikeStrip: PowerupName = TEXT("SPIKE STRIP"); break;
				case EMGPowerupType::Shockwave: PowerupName = TEXT("SHOCKWAVE"); break;
				case EMGPowerupType::Repair: PowerupName = TEXT("REPAIR"); break;
				case EMGPowerupType::RocketBoost: PowerupName = TEXT("ROCKET BOOST"); break;
				default: PowerupName = TEXT("POWERUP"); break;
			}

			FText PowerupMessage = FText::FromString(FString::Printf(TEXT("COLLECTED: %s"), *PowerupName));
			FLinearColor PowerupColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(PowerupMessage, 1.5f, PowerupColor);
		}
	}
}

void AMGPlayerController::OnPowerupHit(const FString& SourceId, const FString& TargetId, EMGPowerupType PowerupType)
{
	FString LocalPlayerId = GetLocalPlayerId();

	// Show when we hit someone
	if (SourceId == LocalPlayerId && TargetId != LocalPlayerId)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				FText HitMessage = FText::FromString(TEXT("HIT!"));
				FLinearColor HitColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
				HUDSubsystem->ShowNotification(HitMessage, 1.5f, HitColor);
			}
		}
	}
	// Show when we get hit
	else if (TargetId == LocalPlayerId && SourceId != LocalPlayerId)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				FText HitMessage = FText::FromString(TEXT("INCOMING!"));
				FLinearColor HitColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
				HUDSubsystem->ShowNotification(HitMessage, 1.5f, HitColor);
			}
		}
	}
}

void AMGPlayerController::OnEngineOverheat(FGuid VehicleID)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText OverheatMessage = FText::FromString(TEXT("ENGINE OVERHEATING!"));
			FLinearColor OverheatColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(OverheatMessage, 3.0f, OverheatColor);
		}
	}
}

void AMGPlayerController::OnWeatherTransitionStarted(EMGWeatherType FromType, EMGWeatherType ToType)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString WeatherName;
			FLinearColor WeatherColor = FLinearColor::White;

			switch (ToType)
			{
				case EMGWeatherType::Clear:
					WeatherName = TEXT("CLEAR SKIES");
					WeatherColor = FLinearColor(1.0f, 0.95f, 0.5f, 1.0f); // Sunny yellow
					break;
				case EMGWeatherType::PartlyCloudy:
					WeatherName = TEXT("PARTLY CLOUDY");
					WeatherColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f); // Light gray
					break;
				case EMGWeatherType::Overcast:
					WeatherName = TEXT("OVERCAST");
					WeatherColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f); // Gray
					break;
				case EMGWeatherType::LightRain:
					WeatherName = TEXT("LIGHT RAIN - SLIPPERY CONDITIONS");
					WeatherColor = FLinearColor(0.4f, 0.6f, 0.9f, 1.0f); // Blue
					break;
				case EMGWeatherType::HeavyRain:
					WeatherName = TEXT("HEAVY RAIN - REDUCED GRIP!");
					WeatherColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f); // Dark blue
					break;
				case EMGWeatherType::Thunderstorm:
					WeatherName = TEXT("THUNDERSTORM - CAUTION!");
					WeatherColor = FLinearColor(0.5f, 0.3f, 0.7f, 1.0f); // Purple
					break;
				case EMGWeatherType::Fog:
					WeatherName = TEXT("FOG - REDUCED VISIBILITY!");
					WeatherColor = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f); // Foggy gray
					break;
				default:
					WeatherName = TEXT("WEATHER CHANGING");
					break;
			}

			FText WeatherMessage = FText::FromString(WeatherName);
			HUDSubsystem->ShowNotification(WeatherMessage, 4.0f, WeatherColor);
		}
	}
}

void AMGPlayerController::OnCautionDeployed(EMGCautionType Type, EMGCautionReason Reason)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText CautionMessage;
			FLinearColor CautionColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

			switch (Type)
			{
				case EMGCautionType::LocalYellow:
					CautionMessage = FText::FromString(TEXT("LOCAL YELLOW FLAG"));
					break;
				case EMGCautionType::FullCourseYellow:
					CautionMessage = FText::FromString(TEXT("FULL COURSE YELLOW"));
					break;
				case EMGCautionType::SafetyCar:
					CautionMessage = FText::FromString(TEXT("SAFETY CAR DEPLOYED"));
					CautionColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					break;
				case EMGCautionType::VirtualSafetyCar:
					CautionMessage = FText::FromString(TEXT("VIRTUAL SAFETY CAR"));
					CautionColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					break;
				case EMGCautionType::RedFlag:
					CautionMessage = FText::FromString(TEXT("RED FLAG - RACE STOPPED"));
					CautionColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					break;
				case EMGCautionType::Code60:
					CautionMessage = FText::FromString(TEXT("CODE 60 - SLOW DOWN"));
					break;
				default:
					CautionMessage = FText::FromString(TEXT("CAUTION"));
					break;
			}

			HUDSubsystem->ShowNotification(CautionMessage, 5.0f, CautionColor);
		}
	}
}

void AMGPlayerController::OnCautionEnded(EMGCautionType Type)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText Message = FText::FromString(TEXT("GREEN FLAG - RACING RESUMES"));
			FLinearColor GreenColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
			HUDSubsystem->ShowNotification(Message, 4.0f, GreenColor);
		}
	}
}

void AMGPlayerController::OnSafetyCarDeployed(const FMGSafetyCarState& State)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText Message = FText::FromString(TEXT("SAFETY CAR - MAINTAIN POSITION"));
			FLinearColor OrangeColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
			HUDSubsystem->ShowNotification(Message, 5.0f, OrangeColor);
		}
	}
}

void AMGPlayerController::OnSafetyCarIn()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText Message = FText::FromString(TEXT("SAFETY CAR IN THIS LAP"));
			FLinearColor YellowColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
			HUDSubsystem->ShowNotification(Message, 4.0f, YellowColor);
		}
	}
}

void AMGPlayerController::OnPenaltyIssued(const FMGPenalty& Penalty)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle || Penalty.VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText PenaltyMessage;
			FLinearColor PenaltyColor;
			float Duration = 4.0f;

			switch (Penalty.Type)
			{
				case EMGPenaltyType::Warning:
					PenaltyMessage = FText::FromString(TEXT("WARNING ISSUED"));
					PenaltyColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
					Duration = 2.5f;
					break;

				case EMGPenaltyType::TimeAdded:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("+%.1f SEC PENALTY"), Penalty.TimeAmount));
					PenaltyColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					break;

				case EMGPenaltyType::DriveThrough:
					PenaltyMessage = FText::FromString(TEXT("DRIVE-THROUGH PENALTY"));
					PenaltyColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
					Duration = 5.0f;
					break;

				case EMGPenaltyType::StopAndGo:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("STOP & GO PENALTY (%ds)"), FMath::RoundToInt(Penalty.TimeAmount)));
					PenaltyColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f); // Red-orange
					Duration = 5.0f;
					break;

				case EMGPenaltyType::PositionPenalty:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("-%d POSITION PENALTY"), Penalty.PositionAmount));
					PenaltyColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					Duration = 5.0f;
					break;

				case EMGPenaltyType::GridPenalty:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("-%d GRID POSITIONS (next race)"), Penalty.PositionAmount));
					PenaltyColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					Duration = 5.0f;
					break;

				case EMGPenaltyType::Disqualification:
					PenaltyMessage = FText::FromString(TEXT("DISQUALIFIED"));
					PenaltyColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					Duration = 6.0f;
					break;

				case EMGPenaltyType::Exclusion:
					PenaltyMessage = FText::FromString(TEXT("EXCLUDED FROM SESSION"));
					PenaltyColor = FLinearColor(0.5f, 0.0f, 0.0f, 1.0f); // Dark red
					Duration = 6.0f;
					break;

				case EMGPenaltyType::PointsDeduction:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("-%d CHAMPIONSHIP POINTS"), Penalty.PointsAmount));
					PenaltyColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f); // Magenta
					Duration = 5.0f;
					break;

				case EMGPenaltyType::FinePenalty:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("$%d FINE"), Penalty.FineAmount));
					PenaltyColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
					Duration = 3.0f;
					break;

				case EMGPenaltyType::LicensePoints:
					PenaltyMessage = FText::FromString(FString::Printf(TEXT("+%d LICENSE POINTS"), Penalty.LicensePointsAmount));
					PenaltyColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					Duration = 3.0f;
					break;

				default:
					PenaltyMessage = FText::FromString(TEXT("PENALTY ISSUED"));
					PenaltyColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					break;
			}

			HUDSubsystem->ShowNotification(PenaltyMessage, Duration, PenaltyColor);
		}
	}
}

void AMGPlayerController::OnPenaltyServed(const FMGPenalty& Penalty)
{
	// Only show for the player's vehicle
	if (!ControlledVehicle || Penalty.VehicleID != FName(*ControlledVehicle->GetName()))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ServedMessage = FText::FromString(TEXT("PENALTY SERVED"));
			FLinearColor ServedColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f); // Green
			HUDSubsystem->ShowNotification(ServedMessage, 2.5f, ServedColor);
		}
	}
}

void AMGPlayerController::OnHeatLevelChanged(EMGHeatLevel OldLevel, EMGHeatLevel NewLevel)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText HeatMessage;
			FLinearColor HeatColor;
			float Duration = 3.0f;

			switch (NewLevel)
			{
				case EMGHeatLevel::None:
					// Going from heat to no heat
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL CLEARED"));
					HeatColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
					break;

				case EMGHeatLevel::Low:
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL 1 - PATROL ALERT"));
					HeatColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
					break;

				case EMGHeatLevel::Medium:
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL 2 - UNITS DISPATCHED"));
					HeatColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange
					break;

				case EMGHeatLevel::High:
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL 3 - AGGRESSIVE PURSUIT"));
					HeatColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
					Duration = 4.0f;
					break;

				case EMGHeatLevel::Critical:
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL 4 - FEDERAL RESPONSE"));
					HeatColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
					Duration = 5.0f;
					break;

				case EMGHeatLevel::Maximum:
					HeatMessage = FText::FromString(TEXT("HEAT LEVEL 5 - MOST WANTED"));
					HeatColor = FLinearColor(0.8f, 0.0f, 0.8f, 1.0f); // Purple
					Duration = 5.0f;
					break;

				default:
					return;
			}

			HUDSubsystem->ShowNotification(HeatMessage, Duration, HeatColor);
		}
	}
}

void AMGPlayerController::OnPursuitEvaded(float Duration, int32 BountyEarned)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			int32 Minutes = FMath::FloorToInt(Duration / 60.0f);
			int32 Seconds = FMath::FloorToInt(FMath::Fmod(Duration, 60.0f));

			FText EvadeMessage;
			if (BountyEarned > 0)
			{
				EvadeMessage = FText::FromString(FString::Printf(TEXT("EVADED! %d:%02d - $%d BOUNTY"), Minutes, Seconds, BountyEarned));
			}
			else
			{
				EvadeMessage = FText::FromString(FString::Printf(TEXT("EVADED! %d:%02d"), Minutes, Seconds));
			}

			FLinearColor EvadeColor = FLinearColor(0.0f, 1.0f, 0.3f, 1.0f); // Green
			HUDSubsystem->ShowNotification(EvadeMessage, 5.0f, EvadeColor);
		}
	}
}

void AMGPlayerController::OnPlayerBusted(int32 TotalCost, float PursuitDuration)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText BustedMessage = FText::FromString(FString::Printf(TEXT("BUSTED! -$%d"), TotalCost));
			FLinearColor BustedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(BustedMessage, 6.0f, BustedColor);
		}
	}
}

void AMGPlayerController::OnHelicopterDeployed()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText HelicopterMessage = FText::FromString(TEXT("HELICOPTER DEPLOYED!"));
			FLinearColor HelicopterColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(HelicopterMessage, 4.0f, HelicopterColor);
		}
	}
}

void AMGPlayerController::OnBountyCompleted(const FString& PlayerId, const FMGBountyCompletionResult& Result)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText CompleteMessage = FText::FromString(FString::Printf(TEXT("BOUNTY COMPLETE! +$%d"), Result.RewardAmount));
			FLinearColor CompleteColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(CompleteMessage, 5.0f, CompleteColor);
		}
	}
}

void AMGPlayerController::OnBountyFailed(const FString& PlayerId, const FString& BountyId, const FString& Reason)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText FailedMessage = FText::FromString(TEXT("BOUNTY FAILED"));
			FLinearColor FailedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
			HUDSubsystem->ShowNotification(FailedMessage, 4.0f, FailedColor);
		}
	}
}

void AMGPlayerController::OnBountyObjectiveCompleted(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId)
{
	// Only show for local player
	FString LocalPlayerId = GetLocalPlayerId();
	if (PlayerId != LocalPlayerId)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ObjectiveMessage = FText::FromString(TEXT("OBJECTIVE COMPLETE"));
			FLinearColor ObjectiveColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-green
			HUDSubsystem->ShowNotification(ObjectiveMessage, 2.5f, ObjectiveColor);
		}
	}
}

void AMGPlayerController::OnDramaticMoment(const FMGRaceEvent& Event)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText DramaMessage;
			FLinearColor DramaColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

			switch (Event.Type)
			{
				case EMGRaceEventType::PhotoFinish:
					DramaMessage = FText::FromString(TEXT("PHOTO FINISH!"));
					DramaColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
					break;

				case EMGRaceEventType::CloseBattle:
					DramaMessage = FText::FromString(TEXT("CLOSE BATTLE!"));
					DramaColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
					break;

				case EMGRaceEventType::MajorOvertake:
					DramaMessage = FText::FromString(TEXT("MAJOR OVERTAKE!"));
					DramaColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-green
					break;

				case EMGRaceEventType::LastLapDrama:
					DramaMessage = FText::FromString(TEXT("FINAL LAP - IT'S CLOSE!"));
					DramaColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f); // Magenta
					break;

				case EMGRaceEventType::UnexpectedFinish:
					DramaMessage = FText::FromString(TEXT("WHAT A FINISH!"));
					DramaColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
					break;

				default:
					return;
			}

			HUDSubsystem->ShowNotification(DramaMessage, 4.0f, DramaColor);
		}
	}
}

void AMGPlayerController::OnLeadChange(const FGuid& NewLeaderId, int32 TotalChanges)
{
	// Only show if there have been multiple lead changes
	if (TotalChanges < 2)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText LeadMessage = FText::FromString(FString::Printf(TEXT("LEAD CHANGE! (%d total)"), TotalChanges));
			FLinearColor LeadColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
			HUDSubsystem->ShowNotification(LeadMessage, 2.5f, LeadColor);
		}
	}
}

void AMGPlayerController::OnLicenseUpgraded(EMGLicenseCategory Category, EMGLicenseTier NewTier)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString TierStr;
			FLinearColor TierColor;

			switch (NewTier)
			{
				case EMGLicenseTier::Novice:
					TierStr = TEXT("NOVICE");
					TierColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f); // Gray
					break;
				case EMGLicenseTier::National:
					TierStr = TEXT("NATIONAL");
					TierColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f); // Bronze
					break;
				case EMGLicenseTier::International:
					TierStr = TEXT("INTERNATIONAL");
					TierColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f); // Silver
					break;
				case EMGLicenseTier::Super:
					TierStr = TEXT("SUPER");
					TierColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
					break;
				case EMGLicenseTier::Professional:
					TierStr = TEXT("PROFESSIONAL");
					TierColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Platinum
					break;
				case EMGLicenseTier::Elite:
					TierStr = TEXT("ELITE");
					TierColor = FLinearColor(0.6f, 0.85f, 1.0f, 1.0f); // Diamond
					break;
				default:
					TierStr = TEXT("UPGRADED");
					TierColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);
					break;
			}

			FText LicenseMessage = FText::FromString(FString::Printf(TEXT("LICENSE UPGRADED: %s"), *TierStr));
			HUDSubsystem->ShowNotification(LicenseMessage, 5.0f, TierColor);
		}
	}
}

void AMGPlayerController::OnLicenseTestCompleted(const FString& TestId, EMGTestGrade Grade, float Time)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString GradeStr;
			FLinearColor GradeColor;

			switch (Grade)
			{
				case EMGTestGrade::Gold:
					GradeStr = TEXT("GOLD MEDAL!");
					GradeColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
					break;
				case EMGTestGrade::Silver:
					GradeStr = TEXT("SILVER MEDAL");
					GradeColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
					break;
				case EMGTestGrade::Bronze:
					GradeStr = TEXT("BRONZE MEDAL");
					GradeColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);
					break;
				case EMGTestGrade::Pass:
					GradeStr = TEXT("PASSED");
					GradeColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);
					break;
				case EMGTestGrade::Fail:
					GradeStr = TEXT("FAILED");
					GradeColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
					break;
				default:
					return;
			}

			FText TestMessage = FText::FromString(FString::Printf(TEXT("TEST COMPLETE: %s"), *GradeStr));
			HUDSubsystem->ShowNotification(TestMessage, 4.0f, GradeColor);
		}
	}
}

void AMGPlayerController::OnContractCompleted(const FMGContract& Contract)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ContractMessage = FText::FromString(FString::Printf(TEXT("CONTRACT COMPLETE: %s"), *Contract.DisplayName.ToString()));
			FLinearColor ContractColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(ContractMessage, 5.0f, ContractColor);
		}
	}
}

void AMGPlayerController::OnContractObjectiveCompleted(FName ContractID, const FMGContractObjective& Objective)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ObjectiveMessage = FText::FromString(FString::Printf(TEXT("OBJECTIVE: %s"), *Objective.Description.ToString()));
			FLinearColor ObjectiveColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-green
			HUDSubsystem->ShowNotification(ObjectiveMessage, 3.0f, ObjectiveColor);
		}
	}
}

void AMGPlayerController::OnSponsorLevelUp(FName SponsorID, int32 NewLevel)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SponsorMessage = FText::FromString(FString::Printf(TEXT("SPONSOR LEVEL UP! Level %d"), NewLevel));
			FLinearColor SponsorColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // Light blue
			HUDSubsystem->ShowNotification(SponsorMessage, 4.0f, SponsorColor);
		}
	}
}

void AMGPlayerController::OnChallengeCompleted(const FMGChallenge& Challenge)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ChallengeMessage = FText::FromString(FString::Printf(TEXT("CHALLENGE COMPLETE: %s"), *Challenge.DisplayName.ToString()));
			FLinearColor ChallengeColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(ChallengeMessage, 4.0f, ChallengeColor);
		}
	}
}

void AMGPlayerController::OnCurrencyChanged(EMGCurrencyType Type, int64 NewBalance, int64 Delta)
{
	// Only show significant gains (not tiny increments)
	if (Delta <= 0 || Delta < 100)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString CurrencyStr;
			FLinearColor CurrencyColor;

			switch (Type)
			{
				case EMGCurrencyType::Cash:
					CurrencyStr = FString::Printf(TEXT("+$%lld"), Delta);
					CurrencyColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
					break;

				case EMGCurrencyType::Premium:
					CurrencyStr = FString::Printf(TEXT("+%lld GOLD"), Delta);
					CurrencyColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
					break;

				case EMGCurrencyType::RepPoints:
					CurrencyStr = FString::Printf(TEXT("+%lld REP"), Delta);
					CurrencyColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // Light blue
					break;

				case EMGCurrencyType::XP:
					CurrencyStr = FString::Printf(TEXT("+%lld XP"), Delta);
					CurrencyColor = FLinearColor(0.8f, 0.6f, 1.0f, 1.0f); // Purple
					break;

				default:
					return;
			}

			FText CurrencyMessage = FText::FromString(CurrencyStr);
			HUDSubsystem->ShowNotification(CurrencyMessage, 2.0f, CurrencyColor);
		}
	}
}

void AMGPlayerController::OnMultiplierActivated(const FMGEarningMultiplier& Multiplier)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText MultiplierMessage = FText::FromString(FString::Printf(TEXT("%.1fx MULTIPLIER ACTIVE!"), Multiplier.Multiplier));
			FLinearColor MultiplierColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
			HUDSubsystem->ShowNotification(MultiplierMessage, 3.0f, MultiplierColor);
		}
	}
}

void AMGPlayerController::OnDailyRewardClaimed(const FMGRewardClaimResult& Result)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText RewardMessage = FText::FromString(FString::Printf(TEXT("DAILY REWARD! Day %d"), Result.Day));
			FLinearColor RewardColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(RewardMessage, 4.0f, RewardColor);
		}
	}
}

void AMGPlayerController::OnStreakMilestoneReached(EMGStreakMilestone Milestone, const TArray<FMGDailyReward>& Rewards)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString MilestoneStr;
			FLinearColor MilestoneColor;

			switch (Milestone)
			{
				case EMGStreakMilestone::ThreeDay:
					MilestoneStr = TEXT("3-DAY STREAK MILESTONE!");
					MilestoneColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f); // Bronze
					break;
				case EMGStreakMilestone::SevenDay:
					MilestoneStr = TEXT("7-DAY STREAK MILESTONE!");
					MilestoneColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f); // Silver
					break;
				case EMGStreakMilestone::FourteenDay:
					MilestoneStr = TEXT("14-DAY STREAK MILESTONE!");
					MilestoneColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
					break;
				case EMGStreakMilestone::ThirtyDay:
					MilestoneStr = TEXT("30-DAY STREAK MILESTONE!");
					MilestoneColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Platinum
					break;
				default:
					MilestoneStr = TEXT("STREAK MILESTONE!");
					MilestoneColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
					break;
			}

			FText MilestoneMessage = FText::FromString(MilestoneStr);
			HUDSubsystem->ShowNotification(MilestoneMessage, 5.0f, MilestoneColor);
		}
	}
}

void AMGPlayerController::OnReputationTierReached(EMGReputationCategory Category, EMGReputationTier Tier)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FString CategoryStr;
			switch (Category)
			{
				case EMGReputationCategory::Street: CategoryStr = TEXT("STREET"); break;
				case EMGReputationCategory::Drift: CategoryStr = TEXT("DRIFT"); break;
				case EMGReputationCategory::Drag: CategoryStr = TEXT("DRAG"); break;
				case EMGReputationCategory::Circuit: CategoryStr = TEXT("CIRCUIT"); break;
				case EMGReputationCategory::Outlaw: CategoryStr = TEXT("OUTLAW"); break;
				default: CategoryStr = TEXT(""); break;
			}

			FString TierStr;
			FLinearColor TierColor;
			switch (Tier)
			{
				case EMGReputationTier::Rookie:
					TierStr = TEXT("ROOKIE");
					TierColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
					break;
				case EMGReputationTier::Amateur:
					TierStr = TEXT("AMATEUR");
					TierColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);
					break;
				case EMGReputationTier::Skilled:
					TierStr = TEXT("SKILLED");
					TierColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
					break;
				case EMGReputationTier::Expert:
					TierStr = TEXT("EXPERT");
					TierColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
					break;
				case EMGReputationTier::Master:
					TierStr = TEXT("MASTER");
					TierColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
					break;
				case EMGReputationTier::Legend:
					TierStr = TEXT("LEGEND");
					TierColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
					break;
				default:
					TierStr = TEXT("");
					TierColor = FLinearColor::White;
					break;
			}

			FText TierMessage = FText::FromString(FString::Printf(TEXT("%s REP: %s TIER!"), *CategoryStr, *TierStr));
			HUDSubsystem->ShowNotification(TierMessage, 5.0f, TierColor);
		}
	}
}

void AMGPlayerController::OnReputationUnlockEarned(const FMGReputationUnlock& Unlock)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText UnlockMessage = FText::FromString(FString::Printf(TEXT("UNLOCKED: %s"), *Unlock.DisplayName.ToString()));
			FLinearColor UnlockColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(UnlockMessage, 4.0f, UnlockColor);
		}
	}
}

void AMGPlayerController::OnGhostNewPersonalBest(FName TrackID, float NewTime)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			int32 Minutes = FMath::FloorToInt(NewTime / 60.0f);
			float Seconds = FMath::Fmod(NewTime, 60.0f);

			FText PBMessage = FText::FromString(FString::Printf(TEXT("NEW PERSONAL BEST! %d:%05.2f"), Minutes, Seconds));
			FLinearColor PBColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(PBMessage, 5.0f, PBColor);
		}
	}
}

void AMGPlayerController::OnGhostComparison(const FMGGhostComparator& Comparison, EMGGhostComparison Status)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText CompareMessage;
			FLinearColor CompareColor;

			switch (Status)
			{
				case EMGGhostComparison::Ahead:
					CompareMessage = FText::FromString(FString::Printf(TEXT("AHEAD: -%.2fs"), FMath::Abs(Comparison.TimeDelta)));
					CompareColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
					break;
				case EMGGhostComparison::Behind:
					CompareMessage = FText::FromString(FString::Printf(TEXT("BEHIND: +%.2fs"), Comparison.TimeDelta));
					CompareColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Red-orange
					break;
				case EMGGhostComparison::Even:
					CompareMessage = FText::FromString(TEXT("EVEN WITH GHOST"));
					CompareColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
					break;
				default:
					return;
			}

			HUDSubsystem->ShowNotification(CompareMessage, 2.0f, CompareColor);
		}
	}
}

void AMGPlayerController::OnShortcutDiscovered(const FString& ShortcutId, int32 DiscoveryPoints)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText DiscoverMessage = FText::FromString(FString::Printf(TEXT("SHORTCUT DISCOVERED! +%d"), DiscoveryPoints));
			FLinearColor DiscoverColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f); // Cyan
			HUDSubsystem->ShowNotification(DiscoverMessage, 3.0f, DiscoverColor);
		}
	}
}

void AMGPlayerController::OnShortcutCompleted(const FString& ShortcutId, float TimeTaken, float TimeSaved)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText ShortcutMessage;
			FLinearColor ShortcutColor;

			if (TimeSaved > 0.0f)
			{
				ShortcutMessage = FText::FromString(FString::Printf(TEXT("SHORTCUT! -%.1fs"), TimeSaved));
				ShortcutColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
			}
			else
			{
				ShortcutMessage = FText::FromString(FString::Printf(TEXT("SHORTCUT! +%.1fs"), FMath::Abs(TimeSaved)));
				ShortcutColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange (slower route)
			}

			HUDSubsystem->ShowNotification(ShortcutMessage, 2.5f, ShortcutColor);
		}
	}
}

void AMGPlayerController::OnShortcutMastered(const FString& ShortcutId, int32 BonusPoints)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText MasteredMessage = FText::FromString(FString::Printf(TEXT("SHORTCUT MASTERED! +%d"), BonusPoints));
			FLinearColor MasteredColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
			HUDSubsystem->ShowNotification(MasteredMessage, 4.0f, MasteredColor);
		}
	}
}

void AMGPlayerController::OnSecretShortcutFound(const FString& ShortcutId, int32 BonusPoints)
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			FText SecretMessage = FText::FromString(FString::Printf(TEXT("SECRET SHORTCUT! +%d"), BonusPoints));
			FLinearColor SecretColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
			HUDSubsystem->ShowNotification(SecretMessage, 4.0f, SecretColor);
		}
	}
}
