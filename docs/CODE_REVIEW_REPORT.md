# Midnight Grind — Comprehensive Code Review Report

**Date:** 2026-02-06
**Scope:** Full codebase review — 317 .cpp files, 308 .h files, 320 JSON data files
**Engine:** Unreal Engine 5.7
**Reviewer:** Automated deep analysis across 9 system areas

---

## Executive Summary

| Severity | Count |
|----------|-------|
| **CRITICAL / ERROR** | 87 |
| **INCOMPLETE** | 42 |
| **OPTIMIZATION** | 28 |
| **Total Issues** | **157** |

The codebase is architecturally ambitious with 183 subsystems covering all major game systems. However, the review uncovered **critical compilation blockers** (duplicate type definitions, missing members), **physics calculation errors** that will produce incorrect gameplay, **security vulnerabilities** in the economy system, **data inconsistencies** across JSON files that will break cross-system references, and **configuration errors** that prevent core systems from initializing.

### Top 5 Blockers (Fix First)

1. **GameInstance never instantiated** — `DefaultEngine.ini` points to `Engine.GameInstance` instead of `MGGameInstance` (all 183 subsystems dead)
2. **14+ duplicate UENUM/USTRUCT definitions** — Will cause UHT/compilation failures across Economy, Input, Rewards, and Reputation systems
3. **Vehicle input never applied** — `ApplyInputsToVehicle()` is entirely commented out
4. **PinkSlip races map to wrong handler** — Factory maps `PinkSlip` to `CircuitRaceHandler`, breaking the signature game mode
5. **Module dependency errors** — Invalid `"Chaos"` module, editor-only `MetasoundFrontend` in runtime deps

---

## 1. Build Configuration & Project Setup

### ERRORS

**1.1. GameInstanceClass points to base Engine.GameInstance**
- `Config/DefaultEngine.ini:15` — `GameInstanceClass=/Script/Engine.GameInstance`
- `MGGameInstance` is never instantiated. All subsystem initialization, session management, and save bootstrapping is dead code.

**1.2. Invalid module "Chaos" in public dependencies**
- `Source/MidnightGrind/MidnightGrind.Build.cs:28` — `"Chaos"` is not a valid UE5 module name. Will cause UnrealBuildTool failure.

**1.3. Editor-only MetasoundFrontend in runtime dependencies**
- `MidnightGrind.Build.cs:51` — `MetasoundFrontend` is editor-only. Will cause linker failures when packaging.

**1.4. Deprecated EditorStyle module**
- `MidnightGrind.Build.cs:101` and `MidnightGrindEditor.Build.cs:28` — `EditorStyle` was removed in UE 5.x. Use `FAppStyle` from `SlateCore`.

**1.5. Game name redirectors use wrong casing**
- `DefaultEngine.ini:98-99` — Redirects target `/Script/midnightgrind` (lowercase) but module is `MidnightGrind` (PascalCase). Blueprint references through redirectors will fail.

**1.6. GlobalDefaultGameMode points to template Blueprint**
- `DefaultEngine.ini:18` — Still references `BP_VehicleAdvGameMode` from the vehicle template, not the project's custom game mode.

**1.7. OnlineSubsystemSteam plugin not enabled**
- `MidnightGrind.uproject` — Steam plugin not enabled despite 172 lines of Steam config in `DefaultSteam.ini`. All Steam features are inert.

**1.8. MCP plugin buffer overflow**
- `Plugins/UnrealMCP/Private/MCPServerRunnable.cpp:72` — `Buffer[BytesRead] = '\0'` when `BytesRead == 8192` writes past array bounds.

**1.9. MCP socket Send uses character count instead of byte count**
- `MCPServerRunnable.cpp:94,317` — `Response.Len()` returns TCHAR count, not UTF-8 byte count. Non-ASCII responses will be truncated.

### INCOMPLETE

**1.10.** GameName still `VehicleTemplateNew` (`DefaultEngine.ini:2`)
**1.11.** ProjectName still `Advanced Vehicle Template BP` (`DefaultGame.ini:3`)
**1.12.** `DefaultGame.ini` missing shipping metadata (version, company, copyright)
**1.13.** Default maps still point to `VehicleTemplate/Maps/Lvl_VehicleBasic`
**1.14.** `GlobalDefaultServerGameMode` set to None — dedicated server has no game mode

---

## 2. Core Architecture

### ERRORS

**2.1. UFUNCTION overload collision: OnSlipstreamEntered (2 signatures)**
- `Public/Core/MGPlayerController.h:792,1051` — Two `UFUNCTION()` methods with same name but different signatures. UHT does not support UFUNCTION overloading — **compile error**.

**2.2. UFUNCTION overload collision: OnSlingshotReady (2 signatures)**
- `Public/Core/MGPlayerController.h:796,1059` — Same UHT overload problem.

**2.3. Double-binding to conflicting OnSlipstreamEntered delegates**
- `Private/Core/MGPlayerController.cpp:192,377` — `AddDynamic` resolves by name, so one binding is incorrect.

**2.4. Session password replicated to all clients**
- `Public/Core/MGGameState.h:198` — `SessionPassword` is inside a replicated struct. Any client can read it.

**2.5. TotalRacerCount excludes AI racers**
- `Private/Core/MGGameState.cpp:219` — `TotalRacerCount = PlayerArray.Num()` ignores AI. Race will never auto-transition to Results when AI is present.

**2.6. GearShift sent via Unreliable RPC, cleared same frame**
- `Private/Core/MGPlayerController.cpp:1085,799` — Gear shifts are single-frame impulses sent unreliably. Dropped packets = lost gear changes.

### INCOMPLETE

**2.7.** `OnRewind` handler empty — rewind button does nothing (`.cpp:1070`)
**2.8.** `OpenMap` handler empty — map button does nothing (`.cpp:931`)
**2.9.** `ServerSendQuickChat_Implementation` empty — quick chat silently discarded (`.cpp:911`)
**2.10.** `MGGameInstance` protected members missing UPROPERTY macros — invisible to serialization/GC (`.h:599-620`)

### OPTIMIZATION

**2.11.** `GetRaceSettings()` returns full struct by value — should return `const&` (`.h:384`)
**2.12.** `GetPositions()` returns TArray by value — copies array every HUD frame (`.h:424`)
**2.13.** `VehicleInput` replicated back to owner — use `COND_SkipOwner` (`.h:565`)

---

## 3. Vehicle & Physics Systems

### ERRORS

**3.1. Wind speed compounds exponentially every tick**
- `Private/Aerodynamics/MGAerodynamicsSubsystem.cpp:~985` — `Zone.WindSpeed *= (1.0f + GustWave * Zone.GustIntensity)` multiplies each tick instead of applying to base value.

**3.2. Collision unit mismatch: MPH conversion but km/h thresholds**
- `Private/Collision/MGCollisionSubsystem.cpp:199` — Converts to MPH but severity thresholds reference km/h. All collision severity underestimated by ~1.6x.

**3.3. Collision uses global ForwardVector instead of vehicle direction**
- `Private/Collision/MGCollisionSubsystem.cpp:~286` — `FVector::ForwardVector` (world +X) used instead of vehicle's actual forward. Collision type detection wrong for any non-+X-facing vehicle.

**3.4. Damage uses global directions for impact zone calculation**
- `Private/Damage/MGDamageSubsystem.cpp:103` — Same issue: world-space vectors instead of vehicle orientation.

**3.5. Air density goes negative at high altitude**
- `Private/Aerodynamics/MGAerodynamicsSubsystem.cpp:~1000` — `DensityReduction` can exceed 1.0, inverting all aero forces.

**3.6. FMath::Acos without clamping produces NaN**
- `Private/Slipstream/MGSlipstreamSubsystem.cpp:~587` — Dot product not clamped to [-1,1] before `Acos()`.

**3.7. const_cast in const function — undefined behavior**
- `Private/Vehicle/MGVehicleMovementComponent.cpp:933` — Modifies `EngineState` via `const_cast` in const method.

**3.8. Non-deterministic FMath::FRand for misfire simulation**
- `Private/Vehicle/MGVehicleMovementComponent.cpp:988` — Power output varies non-deterministically across network peers.

**3.9. PVehicleOutput not null-checked before array access**
- `Private/Vehicle/MGVehicleMovementComponent.cpp:338` — Potential crash if Chaos physics not yet initialized.

**3.10. Multiple divide-by-zero risks in Collision subsystem**
- `Private/Collision/MGCollisionSubsystem.cpp:~1230,~1262,~463` — Division by `DamageZones.Num()` without empty check.

**3.11. Divide-by-zero in Damage subsystem on MaxHealth/MaxDamage**
- `Private/Damage/MGDamageSubsystem.cpp:159,822,882`

**3.12. Static local TMap shared across all instances (4 locations)**
- `Private/Aerodynamics/MGAerodynamicsSubsystem.cpp:~391` — `static TMap<FString, float> LastDownforce`
- `Private/NitroBoost/MGNitroBoostSubsystem.cpp:~508` — `static TMap<FName, float> RespawnTimers`
- `Private/Vehicle/MGFuelConsumptionComponent.cpp:436` — `static float PreviousThrottle`
- `Private/Vehicle/MGFuelConsumptionComponent.cpp:618` — `static float LastBroadcastWeight`
- All produce data races with multiple vehicle instances.

---

## 4. Race Systems & Game Modes

### ERRORS

**4.1. PinkSlip race type maps to wrong handler**
- `Private/GameModes/RaceTypes/MGRaceTypeHandler.cpp` — `EMGRaceType::PinkSlip` falls through to `CircuitRaceHandler`. PinkSlip wagering, confirmation, and transfer logic never executes.

**4.2. Drag race shift quality has unreachable Late branch**
- `Private/GameModes/RaceTypes/MGDragRaceHandler.cpp:~233` — `Late` shift quality can never be reached due to condition ordering.

**4.3. Drift race grace period bug**
- `Private/GameModes/RaceTypes/MGDriftRaceHandler.cpp:~186` — `Duration` accumulates total drift time but is compared against grace period as if measuring grace elapsed.

**4.4. Ghost subsystem references undeclared DownloadedGhosts member**
- `Private/Ghost/MGGhostSubsystem.cpp:697` — Compilation error.

**4.5. Ghost OnLeaderboardFetched broadcast wrong parameter count**
- `Private/Ghost/MGGhostSubsystem.cpp:795` — 1 param passed, delegate expects 3. Compilation error.

**4.6. Ghost personal best replaced with zero-time record**
- `Private/Ghost/MGGhostSubsystem.cpp:292` — If no lap completed, `BestLapTime=0` beats any existing time.

**4.7. EMGGhostState enum redefinition**
- `Public/Replay/MGGhostRacerActor.h:115` vs `MGGhostSubsystem.h` — Different values, same name.

**4.8. GhostRacerActor IsAheadOfPosition meaningless comparison**
- `Private/Replay/MGGhostRacerActor.cpp:231` — Compares track distance with Euclidean distance from world origin.

**4.9. Checkpoint ResetRace captures stale time limit**
- `Private/Checkpoint/MGCheckpointSubsystem.cpp:249` — `StopRace()` modifies state before time limit capture.

**4.10. Checkpoint UpdateBestTimes completion check may never trigger**
- `Private/Checkpoint/MGCheckpointSubsystem.cpp:1054` — `CurrentLap > TotalLaps` checked after `StopRace()` prevents increment.

**4.11. Stunt distance calculation ignores actual landing position**
- `Private/Stunt/MGStuntSubsystem.cpp:991` — Uses trajectory estimate from launch velocity, not actual landing point.

**4.12. Speedtrap const_cast on AttemptCounter — undefined behavior**
- `Private/Speedtrap/MGSpeedtrapSubsystem.cpp:1096`

### INCOMPLETE

**4.13.** PinkSlip `ApplyCooldown()` — empty stub, no exploit protection
**4.14.** PinkSlip `ApplyTradeLock()` — empty stub, duplication exploit risk
**4.15.** PinkSlip `RecordTransfer()` — witness notifications not implemented
**4.16.** PinkSlip `RematchWindowRemaining` — never decremented, infinite rematch window
**4.17.** Circuit race `HasCrossedStart` set populated but never read — dead data
**4.18.** Checkpoint `StartRace` mutates registered layout data — corrupts reuse
**4.19.** NearMiss `LoadNearMissData` overwrites session stats with career stats

---

## 5. Economy, Progression & Save Systems

### ERRORS (Compilation / ODR Violations)

**5.1. Duplicate EMGTransactionType enum (2 definitions)**
- `Public/Economy/MGEconomySubsystem.h:195` vs `Public/Economy/MGTransactionPipeline.h:114` — Different values, same name. UHT rejection / compile failure.

**5.2. Duplicate EMGRewardType enum (2 definitions)**
- `Public/BattlePass/MGBattlePassSubsystem.h:198` vs `Public/DailyRewards/MGDailyRewardsSubsystem.h:82`

**5.3. Duplicate EMGReputationTier enum (2 definitions, different tier names)**
- `Public/Progression/MGPlayerProgression.h:86` (`Known/Feared`) vs `Public/Reputation/MGReputationSubsystem.h:230` (`Regular/Elite`)

**5.4. Duplicate FMGTransaction struct (2 completely different field layouts)**
- `Public/Economy/MGEconomySubsystem.h:228` vs `Public/Economy/MGTransactionPipeline.h:184`

**5.5. Duplicate FMGChallengeReward struct (2 different field layouts)**
- `Public/Economy/MGEconomySubsystem.h:318` vs `Public/Challenges/MGChallengeSubsystem.h:119`

**5.6. Duplicate FOnCreditsChanged delegate (different param names)**
- `Public/Economy/MGEconomySubsystem.h:351` vs `Public/Progression/MGPlayerProgression.h:405`

### ERRORS (Security / Logic)

**5.7. SetCredits() bypasses all validation and logging**
- `Public/Economy/MGEconomySubsystem.h:398` — Public `BlueprintCallable` that sets credits to any value. Currency manipulation vulnerability.

**5.8. No negative amount validation on EarnCurrency/SpendCurrency**
- `Public/Currency/MGCurrencySubsystem.h:477,487` — Negative amounts invert operations, enabling currency duplication.

**5.9. CanAfford() passes negative amounts**
- `Public/Economy/MGEconomySubsystem.h:386` — `Credits >= Amount` returns true for negative Amount, allowing spend of negative (= earn).

**5.10. Starting cash inconsistency: $7,500 vs $5,000**
- `Public/Economy/MGEconomySubsystem.h:556` (7500) vs `Public/Save/MGSaveGame.h:373` (5000) — Race condition on init order.

**5.11. int32/int64 type mismatches across economy pipeline**
- `Public/Economy/MGTransactionPipeline.h:210` — `PremiumCurrencyDelta` is `int32` while Currency subsystem uses `int64`. Overflow above ~2.1B.
- `Public/Inventory/MGInventorySubsystem.h:322` — `SellValue` is `int32`.
- `Public/Economy/MGMechanicSubsystem.h:364` — `Cost` is `int32`.

---

## 6. AI, Traffic & World Systems

### ARCHITECTURAL ERRORS

**6.1. Four overlapping Police/Pursuit/Heat systems**
- `MGPoliceSubsystem`, `MGHeatLevelSubsystem`, `MGPursuitSubsystem`, and `MGWorldEventsSubsystem` all maintain separate heat levels, unit tracking, and pursuit state. Contradictory game state.

**6.2. Time period boundary mismatch**
- Weather defines Morning as 7-10, TimeOfDay as 7-11. Weather uses `Sunset`, TimeOfDay uses `Dusk`. Weather lacks `LateNight`.

**6.3. Hardcoded DeltaTime in 7 subsystems**
- `MGTimeOfDaySubsystem` (0.1f), `MGTrafficSubsystem` (0.1f), `MGPoliceSubsystem`, `MGPursuitSubsystem` (0.033f), `MGDestructionSubsystem` (0.033f), `MGAmbientLifeSubsystem` (0.5f), `MGWorldEventsSubsystem` (1.0f) — Timer jitter causes inaccuracy.

**6.4. Frame-rate dependent random checks in 6 files**
- `FMath::FRand() < threshold` per tick in AI controllers, traffic vehicles, driver profiles. AI behavior differs at 30 vs 120 FPS.

### PER-SYSTEM ERRORS

**6.5. AI braking distance formula wrong**
- `Private/AI/MGRacingLineGenerator.cpp:304` — Uses `(v1-v2)^2/(2a)` instead of correct `(v1²-v2²)/(2a)`. Braking zones miscalculated.

**6.6. AI GetAllActorsOfClass every tick per instance**
- `Private/AI/MGAIRacerController.cpp:307` — `GetAllActorsOfClass<APawn>` called every tick per AI. Massive performance cost.

**6.7. AI static local shared learning timer**
- `Private/AI/MGAIRacerController.cpp:2026` — `static float LearningTimer` shared across all AI instances.

**6.8. AI racing line does not wrap for circuit tracks (2 locations)**
- `MGAIRacerController.cpp:1631` and `MGRacingLineGenerator.cpp:818` — Closest-point search and apex detection fail at start/finish boundary.

**6.9. AI grid position hardcoded to player=1**
- `Private/AI/MGAIRacerSubsystem.cpp:81` — `GridPosition = i + 2` assumes player always P1. Breaks in multiplayer.

**6.10. Division by zero: ReactionTime, DistanceToPoint**
- `MGRacingAIController.cpp:383` — `1.0f / ReactionTime` with no zero guard.
- `MGAIRacerController.cpp:958` — `BrakingDistance / DistanceToPoint` with no zero guard.
- `MGAIDriverProfile.cpp:71,78` — `Skill / Modifier` with no zero guard.

**6.11. NOS activation and mistake frequency frame-rate dependent**
- `MGRacingAIController.cpp:550,691` — Per-frame probability checks.

**6.12. Traffic: null spawn result still tracked**
- `Private/Traffic/MGTrafficSubsystem.cpp:121` — Vehicle struct added to map even when `SpawnActor` returns null. Later dereference crashes.

**6.13. Traffic: SetActorLocation without sweep**
- `Private/Traffic/MGTrafficVehicle.cpp:187` — Vehicles teleport through walls instead of detecting collisions.

**6.14. Traffic collision broadcasts nullptr as other actor**
- `Private/Traffic/MGTrafficSubsystem.cpp:374` — `OnTrafficCollision.Broadcast(VehicleID, nullptr)`.

**6.15. Traffic: bHasCollided never reset**
- `Private/Traffic/MGTrafficVehicle.cpp:274` — Vehicle permanently stuck in "collided" state.

**6.16. Police: null spawn still tracked, double-counting disabled cops**
- `Private/Police/MGPoliceSubsystem.cpp:544,639+1599`

---

## 7. Audio, UI, Rendering & Input

### ERRORS

**7.1. Duplicate EMGInputAction enum — ODR violation**
- `Public/Input/MGInputRemapSubsystem.h` vs `Public/Input/MGInputBufferSubsystem.h` — Different values, same name.

**7.2. MGAudioDataAssets.cpp references non-existent methods**
- `Private/Audio/MGAudioDataAssets.cpp` — References methods on `UMGMusicManager` that don't exist in the header. Will not compile.

**7.3. MGRetroRenderingSettings.cpp — wrong member variable name**
- `Private/Rendering/MGRetroRenderingSettings.cpp:293-297` — References `CurrentConfig` instead of `RenderConfig`. Compilation error.

**7.4. StartCameraShake called with nullptr class (2 locations)**
- `Private/PostProcess/MGPostProcessSubsystem.cpp:592` and `Private/Cinematic/MGCinematicSubsystem.cpp:296` — `StartCameraShake(nullptr, ...)` crashes at runtime.

**7.5. Customization widget generates new GUID every refresh**
- `Private/UI/MGCustomizationWidget.cpp:1133` — `FGuid::NewGuid()` regenerated per refresh. Purchased parts never show as owned.

**7.6. InstallPart() inverted ownership check**
- `Private/UI/MGCustomizationWidget.cpp:486` — Logic allows locked parts to be installed when owned.

**7.7. Race HUD identical if/else branches**
- `Private/UI/MGRaceHUDSubsystem.cpp:85-98` — Condition has no effect.

**7.8. FMath::VInterpTo with FVector2D arguments**
- `Private/ScreenEffects/MGScreenEffectSubsystem.cpp:801` — Type mismatch, should use `Vector2DInterpTo`.

**7.9. ShakeScreen permanently drifts vignette**
- `Private/PostProcess/MGPostProcessSubsystem.cpp:560` — Vignette boosted but never restored.

**7.10. Camera interpolation modifies source each frame**
- `Private/UI/MGCustomizationWidget.cpp:1264` — Lerp source overwritten, breaking ease curve.

### INCOMPLETE

**7.11.** `ApplyInputsToVehicle()` entirely commented out — no vehicle input works
**7.12.** `ThrottleAssist` and `BrakingAssist` — no-ops, return input unmodified
**7.13.** `UpdateForceFeedback` — empty stub
**7.14.** `DetectControllerType` — no keyboard/mouse detection path
**7.15.** `ApplyBindingsToPlayerInput`, `LoadSavedBindings`, `SaveBindings` — all empty stubs (input remapping doesn't persist)
**7.16.** Duplicate gamepad bindings — multiple actions on same buttons
**7.17.** `SetChannelValue`/`SetChannelValues` for racing wheel FFB — unimplemented
**7.18.** `ApplyHapticOutput` — empty stub, no haptic feedback
**7.19.** `MusicAudioComponent` never created — music subsystem cannot play audio

---

## 8. JSON Data Consistency

### ERRORS

**8.1. Vehicle ID format inconsistency across files**
- `DA_*.json` uses bare names (`KAZE_CIVIC`), `DB_VehicleDatabase.json` uses `VEH_` prefix, `DB_VehicleUnlocks.json` uses real car names (`MX5_NA`, `CivicEG`). Cross-references will fail.

**8.2. REP tier naming inconsistency (4 different conventions)**
- Economy: `NEWCOMER/KNOWN/RESPECTED/FEARED/LEGENDARY`
- Progression: `ROOKIE/STREET/PRO/ELITE/LEGEND`
- Unlocks: `TIER_NOTICED/TIER_KNOWN/TIER_RESPECTED/TIER_FEARED/TIER_LEGENDARY`
- Showroom: `REP_TIER_STREET/REP_TIER_ELITE/REP_TIER_PRO`

**8.3. Kaze Civic PI: 420 (DA file) vs 180 (DB file)**
- Cannot both be Class D. One value is wrong.

**8.4. Lightning 86 in Class C with PI 290 (Class D range)**
- `DB_VehicleDatabase.json:145` — PI 290 falls in Class D (100-299), not Class C (300-449).

**8.5. Loot table class assignments massively wrong**
- `DB_Rewards.json:253-272` — Most vehicles in each class pool are actually from different classes. Players receive wrong-class vehicles from loot.

**8.6. Duplicate JSON keys in daily rewards**
- `DB_RewardsLoot.json:28,42` — `{"Type":"Paint","Type":"Metallic"}` — second key silently overwrites first.

**8.7. Beater Sedan country: Japan (DA file) vs USA (DB file)**
- Same vehicle assigned to different countries.

**8.8. Dealer IDs have zero overlap between Unlocks and Showroom**
- `DB_VehicleUnlocks.json` uses `DEALER_MAIN/IMPORT/EXOTIC/CLASSIC/BLACK`
- `DB_VehicleShowroom.json` uses `DEALER_UNDERGROUND/RISING_SUN/VELOCITY_EXOTICS/EURO_PERFORMANCE/AMERICAN_THUNDER/BUDGET_WHEELS`

**8.9. Story chapter count and names mismatch**
- Unlocks: 5 chapters ("Street Beginnings" → "Midnight Legend")
- Achievements: 6 chapters ("Fresh Blood" → "Legend")

**8.10. Achievement count declared as 85, actual total differs**
- `DB_Achievements.json:4` — `TotalAchievements: 85` but counted categories don't sum to 85.

---

## 9. Cross-Cutting Patterns

### Recurring Anti-Patterns

| Pattern | Occurrences | Impact |
|---------|-------------|--------|
| `static` local variables shared across instances | 8+ | Data races, corrupted multi-vehicle state |
| `const_cast` in const methods | 5+ | Undefined behavior, thread unsafety |
| `FMath::FRand()` per tick without time-scaling | 10+ | Frame-rate dependent gameplay |
| Hardcoded DeltaTime instead of actual elapsed | 7 | Timer jitter causes simulation drift |
| `FVector::ForwardVector` instead of vehicle direction | 3 | Incorrect calculations for rotated vehicles |
| `int32` vs `int64` type mismatch in economy | 5+ | Overflow above ~2.1B currency |
| Duplicate UENUM/USTRUCT names | 14+ | UHT compile failures |

### Recommended Priority Order

1. **P0 — Compilation Blockers:** Fix duplicate type definitions, missing members, wrong variable names (items 5.1-5.6, 7.1-7.3, 4.4-4.5, 4.7)
2. **P0 — Configuration:** Fix GameInstanceClass, GameMode, module deps (items 1.1-1.7)
3. **P0 — Core Gameplay:** Fix PinkSlip handler mapping, vehicle input, race finish logic (items 4.1, 7.11, 2.5)
4. **P1 — Physics Accuracy:** Fix aero wind compounding, collision vectors, air density, NaN, divide-by-zero (items 3.1-3.11)
5. **P1 — Economy Security:** Fix SetCredits bypass, negative amount validation, type mismatches (items 5.7-5.11)
6. **P1 — Data Consistency:** Unify vehicle IDs, REP tiers, class assignments, dealer IDs (items 8.1-8.10)
7. **P2 — AI Quality:** Fix braking formula, frame-rate dependent checks, static locals, circuit wrapping (items 6.5-6.11)
8. **P2 — Polish:** Fix camera shake nullptr, vignette drift, interpolation, HUD branches (items 7.4-7.10)
9. **P3 — Stubs:** Implement empty handlers (rewind, map, quick chat, force feedback, haptics) (remaining INCOMPLETE items)
10. **P3 — Optimization:** Return by const ref, skip owner replication, use spatial queries (OPTIMIZATION items)

---

## Appendix: Files Reviewed

### C++ Source (sampled across all 183 subsystems)
- Core: MGGameInstance, MGGameState, MGPlayerController, MGPlayerState
- Vehicle: MGVehicleMovementComponent, MGVehicleDamageSystem, MGFuelConsumptionComponent, MGStatCalculator
- Physics: MGAerodynamicsSubsystem, MGCollisionSubsystem, MGSlipstreamSubsystem, MGNitroBoostSubsystem, MGDamageSubsystem, MGTireSubsystem, MGFuelSubsystem
- Race: MGRaceTypeHandler, MGDragRaceHandler, MGDriftRaceHandler, MGPinkSlipHandler, MGCircuitRaceHandler, MGCheckpointSubsystem, MGGhostSubsystem, MGGhostRacerActor, MGStuntSubsystem, MGSpeedtrapSubsystem, MGNearMissSubsystem
- Economy: MGEconomySubsystem, MGTransactionPipeline, MGCurrencySubsystem, MGInventorySubsystem, MGMechanicSubsystem, MGBattlePassSubsystem, MGDailyRewardsSubsystem, MGChallengeSubsystem, MGSaveGame, MGPlayerProgression, MGReputationSubsystem
- AI: MGAIRacerController, MGRacingAIController, MGRacingLineGenerator, MGAIRacerSubsystem, MGAIRaceManager, MGAIDriverProfile
- Traffic: MGTrafficSubsystem, MGTrafficVehicle
- Police: MGPoliceSubsystem, MGHeatLevelSubsystem, MGPursuitSubsystem
- World: MGWeatherSubsystem, MGTimeOfDaySubsystem, MGWorldEventsSubsystem, MGDestructionSubsystem, MGAmbientLifeSubsystem
- Audio: MGAudioDataAssets, MGMusicSubsystem, MGEngineSoundSubsystem
- UI: MGCustomizationWidget, MGRaceHUDSubsystem
- Rendering: MGRetroRenderingSettings, MGPostProcessSubsystem, MGScreenEffectSubsystem
- Input: MGVehicleInputHandler, MGInputRemapSubsystem, MGInputBufferSubsystem, MGRacingWheelInputDevice, MGHapticsSubsystem
- Cinematic: MGCinematicSubsystem
- Build: MidnightGrind.Build.cs, Target.cs files, MidnightGrind.uproject

### Configuration
- DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini, DefaultEditor.ini, DefaultSteam.ini

### JSON Data (320 files sampled)
- Vehicles/, Parts/, Tracks/, Economy/, Progression/, Audio/, AI/, Police/, Tuning/, Rewards/, Settings/
