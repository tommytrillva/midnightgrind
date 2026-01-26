#!/bin/bash

# MIDNIGHT GRIND - Autonomous Ralph Loop Starter
# Run this script to start 500 iterations of autonomous development

cd /Users/trill/Documents/GitHub/midnightgrind/prototype

claude '/ralph-loop "🏁 MIDNIGHT GRIND - 500 ITERATION AUTONOMOUS DEV SESSION 🏁

PROJECT: Unreal Engine 5 street racing game with deep car culture simulation
CURRENT STATE: MVP Stage 59 - 183 subsystems, 292 cpp files, 293 headers

📚 DOCUMENTATION TO READ FIRST:
- docs/design/GDD.md (Game Design Document - full vision)
- docs/design/VehicleSystems.md (Deep vehicle tech specs)
- docs/design/MultiplayerSystems.md (Social/economy/multiplayer)
- docs/technical/TechnicalDesign.md (Architecture and implementation)

🎯 PRIMARY MISSION:
Work through the codebase systematically enhancing systems based on the GDD specifications.

PHASE 1 (Iterations 1-100): EXPLORATION & PLANNING
- Read all design docs thoroughly
- Map out subsystem architecture (Source/MidnightGrind/Public/)
- Identify gaps between GDD vision and current implementation
- Create priority list of enhancements
- Document findings in DEVELOPMENT_NOTES.md

PHASE 2 (Iterations 101-250): CORE ENHANCEMENTS
- Vehicle physics refinement (handling, drift, grip)
- Customization system depth (200+ parts interaction)
- Pink slip race mechanics (permanent loss system)
- Economy balancing (cash flow, part pricing)
- AI opponent intelligence (racing lines, rubber-banding)
- Police/heat system (escalation, escape mechanics)

PHASE 3 (Iterations 251-400): FEATURE ADDITIONS
- Crew system implementation
- Meet spot social features
- Player-driven marketplace
- Tournament system
- Reputation/prestige mechanics
- Career mode progression
- Dynamic weather effects on physics
- Traffic AI improvements

PHASE 4 (Iterations 401-500): POLISH & OPTIMIZATION
- Performance profiling and optimization
- Memory leak detection
- Code cleanup and refactoring
- Comprehensive inline documentation
- Unit test coverage
- Build validation
- Final integration testing

🛠️ TECHNICAL REQUIREMENTS:

Code Standards:
- Follow Unreal Engine C++ coding standards
- Use UPROPERTY/UFUNCTION macros properly
- Const correctness everywhere
- Smart pointers (TSharedPtr, TWeakPtr)
- UE_LOG for all important events
- Null checks before dereferencing
- FORCEINLINE for hot paths

Documentation:
- Add /** Doxygen */ comments for all public functions
- Explain WHY not just WHAT in complex logic
- Document assumptions and constraints
- Note performance considerations
- Add usage examples for APIs

Git Commits:
- Clear, descriptive commit messages
- Format: \"feat: add X\" or \"fix: resolve Y\" or \"perf: optimize Z\"
- Atomic commits (one logical change per commit)
- Reference GDD sections when implementing features

🚨 CRITICAL RULES:

1. ALWAYS compile-test after major changes
2. NEVER break existing functionality
3. READ the design docs before implementing features
4. DOCUMENT everything you add or change
5. OPTIMIZE hot paths (physics, rendering, AI)
6. PRESERVE the PS1/PS2 aesthetic vision
7. RESPECT the \"high stakes\" design pillar (pink slips, permanence)

📊 SUCCESS METRICS:

Minimum Requirements:
✓ 50+ meaningful code enhancements
✓ All changes compile without errors
✓ Comprehensive documentation added
✓ Performance maintained or improved
✓ 20+ git commits with clear messages
✓ No regression in existing features
✓ GDD alignment validated

Stretch Goals:
✓ 100+ improvements across all subsystems
✓ New features from GDD fully implemented
✓ Unit test coverage >30%
✓ Performance improvements >10%
✓ Complete API documentation

🎮 IMPLEMENTATION PRIORITIES:

HIGH PRIORITY (Do First):
1. Vehicle physics enhancements (core gameplay)
2. Pink slip race mechanics (unique selling point)
3. Customization depth (200+ parts working correctly)
4. AI racing intelligence (makes or breaks experience)
5. Economy balance (player progression pacing)

MEDIUM PRIORITY:
6. Police/heat system polish
7. Crew/social features
8. Meet spot implementation
9. Weather effects on handling
10. Traffic AI behaviors

LOW PRIORITY (If Time Permits):
11. Additional race types
12. Cosmetic customization expansion
13. Photo mode
14. Replay system
15. Advanced telemetry

💡 WHEN STUCK OR BLOCKED:

If you encounter errors:
- Document the issue in BLOCKERS.md
- Try alternative approaches
- Add TODO comments for manual review
- Continue with other improvements

If unclear about design intent:
- Reference the GDD for vision alignment
- Default to \"authentic PS1/PS2 arcade racer\" vibe
- Prioritize \"meaningful stakes\" and \"real ownership\"

If performance concerns:
- Profile before optimizing
- Document performance-critical paths
- Use UE5 profiling tools (Insights, stat commands)

📝 REPORTING:

Every 50 iterations, create a checkpoint:
- PROGRESS_ITERATION_[N].md with summary
- What was accomplished
- What'\''s next
- Any blockers or concerns

After 450 iterations:
- Final comprehensive report
- Handoff documentation
- Recommended next steps
- Known issues/limitations

🏁 COMPLETION CONDITION:

Output <promise>MIDNIGHT_GRIND_LEGENDARY</promise> when:
- All success metrics met
- Code is production-quality
- Documentation is comprehensive
- No critical bugs or blockers
- Ready for senior dev review
- You'\''ve made something you'\''re proud of

THIS IS YOUR CHANCE TO BUILD SOMETHING LEGENDARY.
THE RACING SIM COMMUNITY IS WAITING.
GO ABSOLUTELY CRAZY AND MAKE MAGIC HAPPEN.

🏎️💨🌃✨ LET'\''S GRIND. 🔥🏁" --max-iterations 500 --completion-promise "MIDNIGHT_GRIND_LEGENDARY"'
