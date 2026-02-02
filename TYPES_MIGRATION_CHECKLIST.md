# MGTypes.h Migration Checklist

Use this checklist to track migration progress across the Midnight Grind codebase.

## Pre-Migration Setup

- [x] Create MGTypes.h with all common types
- [x] Create migration documentation
- [ ] Create feature branch: `feature/types-centralization`
- [ ] Generate project files (`GenerateProjectFiles.bat`)
- [ ] Verify clean build before starting migration
- [ ] Communicate migration plan to team

## Phase 1: Core Classes (High Priority)

### Vehicle System
- [ ] **AMGVehiclePawn.h**
  - [ ] Replace local forward declarations with MGTypes.h include
  - [ ] Remove duplicate enum definitions
  - [ ] Test compilation
  - [ ] Test runtime vehicle spawning

- [ ] **AMGVehiclePawn.cpp**
  - [ ] Include MGTypes.h
  - [ ] Include full headers for classes that need definitions
  - [ ] Test compilation

- [ ] **UMGVehicleMovementComponent.h**
  - [ ] Replace forward declarations
  - [ ] Remove duplicate types
  - [ ] Test compilation

- [ ] **UMGVehicleMovementComponent.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

### Race System
- [ ] **AMGRaceGameMode.h**
  - [ ] Replace EMGRaceType, EMGRaceState definitions with MGTypes.h
  - [ ] Remove duplicate forward declarations
  - [ ] Test compilation

- [ ] **AMGRaceGameMode.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

- [ ] **AMGCheckpoint.h / AMGCheckpointActor.h**
  - [ ] Replace EMGCheckpointType with MGTypes.h
  - [ ] Test compilation

- [ ] **AMGCheckpoint.cpp / AMGCheckpointActor.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

### Player System
- [ ] **AMGPlayerController.h**
  - [ ] Replace forward declarations with MGTypes.h
  - [ ] Remove EMGDamageState if duplicated
  - [ ] Test compilation

- [ ] **AMGPlayerController.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

- [ ] **AMGPlayerState.h**
  - [ ] Replace forward declarations
  - [ ] Test compilation

- [ ] **AMGPlayerState.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

### Game Instance
- [ ] **UMGGameInstance.h**
  - [ ] Include MGTypes.h
  - [ ] Remove duplicate types
  - [ ] Test compilation

- [ ] **UMGGameInstance.cpp**
  - [ ] Include MGTypes.h
  - [ ] Test compilation

## Phase 2: AI Subsystems

- [ ] **MGAIRaceManager.h / .cpp**
- [ ] **MGAIRacerSubsystem.h / .cpp**
- [ ] **MGRacingAIController.h / .cpp**
- [ ] **MGAIDriverProfile.h / .cpp**
- [ ] **MGRacingLineGenerator.h / .cpp**

## Phase 3: Economy Subsystems

- [ ] **MGEconomySubsystem.h / .cpp**
  - [ ] Replace EMGCurrencyType
  - [ ] Replace EMGRewardType
  - [ ] Replace EMGTransactionType

- [ ] **MGPlayerMarketSubsystem.h / .cpp**
  - [ ] Replace EMGListingType
  - [ ] Replace EMGListingStatus

- [ ] **MGShopSubsystem.h / .cpp**

## Phase 4: Race & Track Subsystems

- [ ] **MGRaceFlowSubsystem.h / .cpp**
  - [ ] Replace EMGRaceState
  - [ ] Replace FMGRaceResults struct usage

- [ ] **MGRaceModeSubsystem.h / .cpp**
  - [ ] Replace EMGRaceType

- [ ] **MGTrackSubsystem.h / .cpp**
- [ ] **MGCheckpointSubsystem.h / .cpp**
  - [ ] Replace EMGCheckpointType

- [ ] **MGRaceTypeHandler.h / .cpp**
- [ ] **MGDriftRaceHandler.h / .cpp**
- [ ] **MGWeatherRaceHandler.h / .cpp**

## Phase 5: Garage & Vehicle Subsystems

- [ ] **MGGarageSubsystem.h / .cpp**
- [ ] **MGVehicleSpawnSubsystem.h / .cpp**
- [ ] **MGVehicleWearSubsystem.h / .cpp**
- [ ] **MGVehicleConfigApplicator.h / .cpp**
- [ ] **MGFuelConsumptionComponent.h / .cpp**

## Phase 6: Customization & Parts

- [ ] **MGCustomizationSubsystem.h / .cpp**
  - [ ] Replace EMGCustomizationCategory

- [ ] **MGTuningSubsystem.h / .cpp**
  - [ ] Replace EMGTuningCategory

- [ ] **MGPartInstallation.h / .cpp**
- [ ] **MGPartsCatalogSubsystem.h / .cpp**
- [ ] **MGVehicleCatalogSubsystem.h / .cpp**

## Phase 7: Police & Heat System

- [ ] **MGHeatLevelSubsystem.h / .cpp**
  - [ ] Replace EMGHeatLevel
  - [ ] Replace EMGPursuitState
  - [ ] Replace EMGPoliceUnitType

- [ ] **MGPoliceSubsystem.h / .cpp**
- [ ] **MGPoliceUnit.h / .cpp**

## Phase 8: Gameplay Mechanics

- [ ] **MGDriftSubsystem.h / .cpp**
  - [ ] Replace EMGDriftGrade

- [ ] **MGDriftComboSystem.h / .cpp**
- [ ] **MGGhostSubsystem.h / .cpp**
  - [ ] Replace EMGGhostType
  - [ ] Replace EMGGhostState

- [ ] **MGAirtimeSubsystem.h / .cpp**
- [ ] **MGSlipstreamSubsystem.h / .cpp**
- [ ] **MGDynoSubsystem.h / .cpp**

## Phase 9: Social & Multiplayer

- [ ] **MGMatchmakingSubsystem.h / .cpp**
  - [ ] Replace EMGMatchType
  - [ ] Replace EMGMatchmakingState

- [ ] **MGPartySubsystem.h / .cpp**
  - [ ] Replace EMGPartyState
  - [ ] Replace EMGPartyRole

- [ ] **MGCrewSubsystem.h / .cpp**
  - [ ] Replace EMGCrewRank

- [ ] **MGRivalsSubsystem.h / .cpp**
  - [ ] Replace EMGRivalryIntensity

- [ ] **MGSocialSubsystem.h / .cpp**
- [ ] **MGMeetSpotSubsystem.h / .cpp**

## Phase 10: Events & Progression

- [ ] **MGEventCalendarSubsystem.h / .cpp**
  - [ ] Replace EMGEventType
  - [ ] Replace EMGEventStatus

- [ ] **MGLiveEventSubsystem.h / .cpp**
- [ ] **MGChallengeSubsystem.h / .cpp**
  - [ ] Replace EMGChallengeType
  - [ ] Replace EMGChallengeDifficulty

- [ ] **MGAchievementSubsystem.h / .cpp**
  - [ ] Replace EMGAchievementRarity
  - [ ] Replace EMGAchievementCategory
  - [ ] Replace EMGAchievementStatType

- [ ] **MGPlayerProgression.h / .cpp**
- [ ] **MGContentGatingSubsystem.h / .cpp**
- [ ] **MGRaceRewardsProcessor.h / .cpp**

## Phase 11: Environment & Weather

- [ ] **MGWeatherSubsystem.h / .cpp**
  - [ ] Replace EMGWeatherType
  - [ ] Replace EMGTimeOfDay

- [ ] **MGWeatherRacingEffects.h / .cpp**

## Phase 12: Audio & VFX

- [ ] **MGAudioSubsystem.h / .cpp**
- [ ] **MGEngineAudioComponent.h / .cpp**
- [ ] **MGVehicleSFXComponent.h / .cpp**
  - [ ] Replace EMGSurfaceType

- [ ] **MGMusicManager.h / .cpp**
- [ ] **MGAudioDataAssets.h / .cpp**

## Phase 13: UI & HUD

- [ ] **MGNotificationManager.h / .cpp**
  - [ ] Replace EMGNotificationPriority

- [ ] **MGRacingHUD.h / .cpp**
- [ ] **MGHUDDataProvider.h / .cpp**
- [ ] **MGRaceResultsWidget.h / .cpp**
- [ ] **MGCustomizationWidget.h / .cpp**
- [ ] **MGLeaderboardWidgets.h / .cpp**
- [ ] **MGLobbyWidget.h / .cpp**

## Phase 14: Input & Racing Wheel

- [ ] **MGVehicleInputHandler.h / .cpp**
  - [ ] Replace EMGInputAction

- [ ] **MGRacingWheelSubsystem.h / .cpp**
- [ ] **MGRacingWheelInputDevice.h / .cpp**
- [ ] **MGWheelFFBProcessor.h / .cpp**
- [ ] **MGInputBufferSubsystem.h / .cpp**
  - [ ] Replace EMGInputAction
  - [ ] Replace EMGComboType

## Phase 15: Replay & Photo Mode

- [ ] **MGReplaySubsystem.h / .cpp**
  - [ ] Replace EMGReplayState
  - [ ] Replace EMGReplayQuality

- [ ] **MGCasterToolsSubsystem.h / .cpp**
  - [ ] Replace EMGHighlightType

- [ ] **MGPhotoModeWidgets.h / .cpp**
- [ ] **MGSocialShareSubsystem.h / .cpp**
  - [ ] Replace EMGShareStatus

## Phase 16: Miscellaneous Subsystems

- [ ] **MGInventorySubsystem.h / .cpp**
  - [ ] Replace EMGItemRarity

- [ ] **MGTelemetrySubsystem.h / .cpp**
- [ ] **MGAnalyticsSubsystem.h / .cpp**
- [ ] **MGAntiCheatSubsystem.h / .cpp**
  - [ ] Replace EMGAntiCheatViolationType

- [ ] **MGCollisionSubsystem.h / .cpp**
  - [ ] Replace EMGDamageState
  - [ ] Replace EMGDamageZone

- [ ] **MGPinkSlipSubsystem.h / .cpp**
- [ ] **MGBalancingSubsystem.h / .cpp**
  - [ ] Replace EMGCatchUpMode

## Phase 17: Test Framework

- [ ] **MGSubsystemTests.h / .cpp**
- [ ] **MGDevCommands.h / .cpp**
- [ ] **MGTestRaceConfig.h / .cpp**

## Phase 18: Data Assets

- [ ] **MGVehicleContentAssets.h / .cpp**
- [ ] **MGVehicleDatabase.h / .cpp**
  - [ ] Replace EMGTransmissionType

- [ ] **MGPartQuality.h / .cpp**
- [ ] **MGTrackDataAssets.h / .cpp**
- [ ] **MGAudioDataAssets.h / .cpp**

## Phase 19: Cleanup & Verification

### Remove Deprecated Headers
- [ ] Check if MGSharedTypes.h can be removed
- [ ] Remove any backup files created during migration
- [ ] Clean up old forward declaration comments

### Update Includes
- [ ] Search for `#include "Core/MGSharedTypes.h"` and replace with `#include "MGTypes.h"`
- [ ] Remove redundant includes in .cpp files

### Documentation Updates
- [ ] Update coding standards wiki
- [ ] Update new developer onboarding guide
- [ ] Add MGTypes.h to essential headers list

## Phase 20: Testing

### Compilation Testing
- [ ] Clean solution
- [ ] Rebuild entire project
- [ ] Verify zero errors
- [ ] Verify zero warnings (or acceptable warnings)

### Runtime Testing
- [ ] Test all game modes
  - [ ] Circuit race
  - [ ] Sprint race
  - [ ] Drift competition
  - [ ] Drag race
  - [ ] Time attack
  - [ ] Free roam

- [ ] Test vehicle systems
  - [ ] Vehicle spawning
  - [ ] Vehicle physics
  - [ ] Damage system
  - [ ] Customization
  - [ ] Parts installation

- [ ] Test race systems
  - [ ] Checkpoint progression
  - [ ] Lap counting
  - [ ] Position tracking
  - [ ] Results calculation

- [ ] Test economy
  - [ ] Currency transactions
  - [ ] Shop purchases
  - [ ] Marketplace trading

- [ ] Test progression
  - [ ] XP earning
  - [ ] Reputation gains
  - [ ] Achievements unlock
  - [ ] Challenge completion

- [ ] Test multiplayer
  - [ ] Matchmaking
  - [ ] Party system
  - [ ] Crew features
  - [ ] Rivalry tracking

- [ ] Test save/load
  - [ ] Save game
  - [ ] Load game
  - [ ] Verify all enums serialize correctly
  - [ ] Check for data corruption

### Performance Testing
- [ ] Measure build times (target: 8-10 min incremental)
- [ ] Check runtime performance (should be identical)
- [ ] Profile memory usage (should be identical or better)

### Integration Testing
- [ ] Run automated test suite
- [ ] Verify all Blueprints still compile
- [ ] Check that editor tools still work
- [ ] Test all data assets load correctly

## Post-Migration

### Code Review
- [ ] Self-review all changed files
- [ ] Request peer review from team
- [ ] Address review feedback

### Merge & Deploy
- [ ] Squash commits if needed
- [ ] Write comprehensive commit message
- [ ] Merge to develop branch
- [ ] Monitor CI/CD pipeline
- [ ] Deploy to testing environment

### Communication
- [ ] Announce completion to team
- [ ] Share migration guide with team
- [ ] Hold knowledge sharing session
- [ ] Update project documentation

## Metrics Collection

Track these metrics before and after:

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Build Time (clean) | ___ min | ___ min | ___% |
| Build Time (incremental) | ___ min | ___ min | ___% |
| Duplicate Types | 984 | 0 | 100% |
| Header Files Changed | 0 | ___ | N/A |
| Lines of Code Removed | 0 | ___ | N/A |
| Build Errors Eliminated | N/A | ___ | N/A |

## Risk Mitigation

If issues arise during migration:

1. **Build Breaks**
   - [ ] Revert to last working commit
   - [ ] Fix issue in isolation
   - [ ] Re-apply migration incrementally

2. **Runtime Errors**
   - [ ] Check enum value mismatches
   - [ ] Verify struct layouts haven't changed
   - [ ] Test save game compatibility

3. **Performance Regression**
   - [ ] Profile to identify bottlenecks
   - [ ] Check for unnecessary includes
   - [ ] Optimize include order

## Notes

Use this space to track issues, insights, or important decisions made during migration:

```
[Date] - [Your Name]
- Issue: ...
- Resolution: ...

[Date] - [Your Name]
- Found that X subsystem requires Y ...
- Solution: ...
```

---

**Progress Tracker:**

- [ ] Phase 1: Core Classes (0/5 complete)
- [ ] Phase 2: AI Subsystems (0/5 complete)
- [ ] Phase 3: Economy Subsystems (0/3 complete)
- [ ] Phase 4: Race & Track Subsystems (0/7 complete)
- [ ] Phase 5: Garage & Vehicle Subsystems (0/5 complete)
- [ ] Phase 6: Customization & Parts (0/4 complete)
- [ ] Phase 7: Police & Heat System (0/3 complete)
- [ ] Phase 8: Gameplay Mechanics (0/5 complete)
- [ ] Phase 9: Social & Multiplayer (0/5 complete)
- [ ] Phase 10: Events & Progression (0/7 complete)
- [ ] Phase 11: Environment & Weather (0/2 complete)
- [ ] Phase 12: Audio & VFX (0/5 complete)
- [ ] Phase 13: UI & HUD (0/7 complete)
- [ ] Phase 14: Input & Racing Wheel (0/5 complete)
- [ ] Phase 15: Replay & Photo Mode (0/4 complete)
- [ ] Phase 16: Miscellaneous Subsystems (0/6 complete)
- [ ] Phase 17: Test Framework (0/3 complete)
- [ ] Phase 18: Data Assets (0/4 complete)
- [ ] Phase 19: Cleanup & Verification (0/3 sections complete)
- [ ] Phase 20: Testing (0/5 sections complete)

**Overall Progress: 0/20 phases complete (0%)**

Start Date: ___________  
Target Completion: ___________  
Actual Completion: ___________
