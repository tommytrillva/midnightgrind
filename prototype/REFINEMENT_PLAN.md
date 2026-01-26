# MIDNIGHT GRIND - Code Refinement Plan
## Based on PRD Review - Iteration 71+

**Date:** 2026-01-26
**Focus:** Refine & polish existing completed systems
**Goal:** Move from 80% systems code → 95% production-ready

---

## EXECUTIVE SUMMARY

After reviewing ROADMAP_PRD_2026.md, the project has:
- ✅ **Exceptional architecture** (183 subsystems)
- ✅ **Comprehensive systems** (80% complete)
- ⚠️ **Integration gaps** (20% complete) **← PRIMARY FOCUS**
- ⚠️ **Unverified compilation** **← CRITICAL**
- ⚠️ **Blueprint wiring incomplete** **← HIGH PRIORITY**

**My Mission (Iterations 71-100):**
Stop adding new features. **Refine, integrate, and verify** what exists.

---

## PHASE 1: COMPILATION & VERIFICATION (Iterations 71-75)

### Priority 0 - Critical Blockers

| Task | Status | Iterations | Impact |
|------|--------|------------|--------|
| Verify MGVehicleMovementComponent compiles | ⏸️ | 71 | Core gameplay |
| Verify MGAIRacerController compiles | ⏸️ | 71 | AI opponents |
| Verify MGAIDriverProfile compiles | ⏸️ | 71 | AI behavior |
| Check all recent edits for syntax errors | ⏸️ | 71-72 | Stability |
| Document compilation warnings | ⏸️ | 72 | Code quality |
| Fix critical compile errors | ⏸️ | 73-75 | Unblock development |

**Deliverable:** Clean compilation or documented blockers.

---

## PHASE 2: INTEGRATION GAPS (Iterations 76-85)

### 2.1 Blueprint Exposure Audit

**What's Missing:**
According to PRD Section 2.1, many systems are "code complete" but Blueprint integration is unverified.

| System | Code Status | Blueprint Status | Action Needed |
|--------|-------------|------------------|---------------|
| Vehicle Physics | ✅ Complete | ⚠️ Unknown | Add BlueprintCallable where missing |
| AI Racing | ✅ Complete | ⚠️ Unknown | Verify AI can be spawned from BP |
| Customization | ✅ Complete | ⚠️ Unknown | Test part install from BP |
| Economy | ✅ Complete | ⚠️ Unknown | Verify transaction calls work |
| Tuning | ✅ Complete | ⚠️ Unknown | Test slider value updates |

**Iterations 76-80:**
1. Audit all subsystems for `UFUNCTION(BlueprintCallable)` coverage
2. Add missing Blueprint exposure where needed
3. Add `UPROPERTY(BlueprintReadWrite)` for designer tuning
4. Document Blueprint API for each subsystem
5. Create example Blueprint usage patterns

### 2.2 Save System Integration

**PRD Section 6.1 - P1 Priority**

Current status from DEVELOPMENT_NOTES:
- MGSaveSubsystem exists (code)
- Integration with all subsystems: Unknown
- Actual save/load testing: Not done

**Iterations 81-82:**
1. Review MGSaveSubsystem implementation
2. Verify all critical data is serializable
3. Add save/load functions where missing
4. Test basic save → quit → load cycle
5. Document save format and versioning

### 2.3 Performance Baseline

**PRD Section 6.3 - Performance Budget Goals:**
- Garage: 60 FPS
- Free roam: 60 FPS
- 8-player race: 60 FPS

**Iterations 83-85:**
1. Identify performance-critical code paths
2. Add profiling markers to hot functions
3. Review surface detection (current: 4 line traces/frame)
4. Review AI updates (multiple AI updates/frame)
5. Suggest optimization opportunities
6. Document performance budget per system

---

## PHASE 3: SYSTEM REFINEMENT (Iterations 86-95)

### 3.1 Vehicle Physics Polish

**PRD Section 3.2 - Gap 2: Physics Tuning (P0)**

Current implementation (from Iterations 1-60):
- ✅ Basic Chaos physics
- ✅ Surface grip (10 types)
- ✅ Tire temperature
- ✅ Engine temperature
- ⚠️ Actual driving feel: Untuned

**Refinement Tasks:**
1. Review and document all physics parameters
2. Add designer-friendly tuning presets (Arcade, Sim, Balanced)
3. Validate suspension spring/damper values
4. Verify weight transfer calculations
5. Test edge cases (high speed, collisions, jumps)
6. Add physics debug visualization

### 3.2 AI System Refinement

**Current State:**
- ✅ Racing line following
- ✅ Overtake/defend strategies
- ✅ Mood system (NEW - Iteration 70)
- ✅ Learning system (NEW - Iteration 70)
- ⚠️ Integration with other recent additions: Untested

**Refinement Tasks:**
1. Verify new aggression response functions compile
2. Test mood transitions (Neutral → Frustrated → Vengeful)
3. Verify learning system updates player predictions
4. Add AI debug visualization (mood, target, strategy)
5. Balance aggression escalation timings
6. Document AI personality authoring guide

### 3.3 Economy System Refinement

**Current State (from Iteration 30):**
- ✅ Balance pass complete
- ✅ Risk/reward multipliers
- ⚠️ Integration with actual races: Untested

**Refinement Tasks:**
1. Verify transaction logging works
2. Test economy edge cases (negative cash, overflow)
3. Add economy debug console commands
4. Validate progression pacing (hours to milestones)
5. Test marketplace listing/sale flow

### 3.4 Tuning System Refinement

**Current State:**
- ✅ Comprehensive FMGVehicleTuning struct
- ⚠️ ECU maps: Not implemented
- ⚠️ Live parameter updates: Unknown

**Refinement Tasks:**
1. Verify slider value changes affect gameplay
2. Add real-time preview for tuning changes
3. Implement tuning presets (Drag, Drift, Grip)
4. Add undo/reset functionality
5. Test tuning save/load

---

## PHASE 4: MISSING IMPLEMENTATIONS (Iterations 96-100)

### 4.1 Functions Declared But Not Implemented

From my code review, I found several declared functions that may be stubs:

| System | Function | Status | Priority |
|--------|----------|--------|----------|
| AIRacerController | HandleContactResponse() | ✅ Impl (auto-added) | Verify |
| AIRacerController | ApplyAggressionModifiers() | ✅ Impl (auto-added) | Verify |
| AIRacerController | ShouldAttemptDirtyMove() | ✅ Impl (auto-added) | Verify |
| AIDriverProfile | RecordContact() | ⚠️ Unknown | Check |
| AIDriverProfile | GetContactResponse() | ⚠️ Unknown | Check |
| AIDriverProfile | EnterBattleMode() | ⚠️ Unknown | Check |
| AIDriverProfile | UpdateAggressionState() | ⚠️ Unknown | Check |

**Task:** Audit all recently modified files for missing implementations.

### 4.2 Data Validation

Systems exist but may have invalid default values:

**Iterations 96-98:**
1. Review all UPROPERTY default values
2. Ensure physics values are realistic (masses, forces)
3. Validate economy values (prices, rewards)
4. Check enum coverage (all cases handled in switches)
5. Add validation asserts for critical parameters

### 4.3 Error Handling

**Iterations 99-100:**
1. Add null checks for critical pointers
2. Add error logging for failure cases
3. Graceful degradation when subsystems unavailable
4. Network error handling (for multiplayer prep)
5. File I/O error handling (save/load failures)

---

## PHASE 5: DOCUMENTATION REFINEMENT (Continuous)

### Code Documentation

**Current State:**
- Headers have basic comments
- Function documentation: Minimal
- Usage examples: None

**Improvements Needed:**
1. Add detailed function comments (params, returns, examples)
2. Document subsystem initialization order
3. Create integration guides (how to wire new vehicles, tracks)
4. Document Blueprint API for each subsystem
5. Add troubleshooting guides

### Technical Debt Tracking

Create `TECHNICAL_DEBT.md` documenting:
1. Known limitations in current implementations
2. Temporary workarounds that need proper fixes
3. Performance optimization opportunities
4. Code cleanup opportunities (duplicated logic, magic numbers)
5. Future refactoring candidates

---

## SUCCESS CRITERIA

**By Iteration 100:**

✅ **Compilation:**
- [ ] All modified files compile without errors
- [ ] All compilation warnings documented
- [ ] No critical linker errors

✅ **Integration:**
- [ ] Blueprint API documented for core systems
- [ ] Save/load tested for critical data
- [ ] Performance baseline established

✅ **Refinement:**
- [ ] Physics parameters documented and reasonable
- [ ] AI system tested and balanced
- [ ] Economy validated through simulation

✅ **Quality:**
- [ ] Error handling added to critical paths
- [ ] Null checks in place
- [ ] Default values validated

✅ **Documentation:**
- [ ] Code comments added to complex functions
- [ ] Integration guides created
- [ ] Technical debt catalogued

---

## ANTI-GOALS (What NOT To Do)

❌ **Don't add new features** - Focus on refining existing
❌ **Don't create art assets** - That's Phase 0/1 work (per PRD)
❌ **Don't implement missing subsystems** - 183 exist, refine what's there
❌ **Don't redesign architecture** - It's 95% complete, respect it
❌ **Don't over-engineer** - Minimum changes for production-ready state

---

## NEXT ACTIONS (Iteration 71)

**Immediate:**
1. Check if recent AI changes compile
2. Review auto-added functions for completeness
3. Identify any missing function implementations
4. Create compilation verification script
5. Begin Blueprint exposure audit

**Checkpoint:**
- Create PROGRESS_ITERATION_100.md
- Update DEVELOPMENT_NOTES.md with refinement work
- Summarize production-readiness improvements

---

**Document Status:** DRAFT
**Alignment:** ROADMAP_PRD_2026.md Section 6.1 (Technical Priorities)
**Focus:** Move from "Code Complete" → "Production Ready"

---
