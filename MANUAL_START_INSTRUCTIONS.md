# 🏁 MIDNIGHT GRIND - Manual Ralph Loop Start Instructions

The Ralph Loop plugin requires an interactive Claude Code session. Here's how to start it:

## Quick Start (Copy & Paste)

### Step 1: Open Terminal and Navigate
```bash
cd /Users/trill/Documents/GitHub/midnightgrind/prototype
```

### Step 2: Start Claude Code
```bash
claude
```

### Step 3: Paste This Command
Once Claude Code is running, paste the entire command below:

```
/ralph-loop "🏁 MIDNIGHT GRIND - 500 ITERATION SESSION

Read ../docs/design/GDD.md, VehicleSystems.md, MultiplayerSystems.md
Stage 59 MVP: 183 subsystems, 292 cpp files

MISSION: Enhance UE5 racing game per GDD specs

PHASES:
1-100: Docs, explore, plan
101-250: Core (physics, customization, pink slips, AI, economy, police)
251-400: Features (crew, social, marketplace, career, weather, traffic)
401-500: Polish, optimize, test, document

PRIORITIES:
1. Vehicle physics depth
2. Pink slip mechanics
3. 200+ parts system
4. AI intelligence
5. Economy balance

STANDARDS: UE5 conventions, docs, performance, clean commits

SUCCESS: 50+ enhancements, compiles, documented, GDD-aligned

Checkpoint every 50 iterations: PROGRESS_ITERATION_[N].md
Output <promise>MIDNIGHT_GRIND_LEGENDARY</promise> when done

GO CRAZY 🏎️💨" --max-iterations 500 --completion-promise "MIDNIGHT_GRIND_LEGENDARY"
```

## Alternative: Use the Mission File

Instead of the command above, you can tell Claude:

```
Read RALPH_MISSION.txt and work on this codebase following those instructions for 500 iterations. Create progress checkpoints every 50 iterations. Output MIDNIGHT_GRIND_LEGENDARY when complete and all success criteria are met.
```

## Monitoring Progress

While it's running, you can monitor in another terminal:

```bash
# Watch for progress files
watch -n 30 'ls -lth /Users/trill/Documents/GitHub/midnightgrind/prototype/PROGRESS_*.md'

# Check git commits
cd /Users/trill/Documents/GitHub/midnightgrind && git log --oneline --graph

# See what files are being modified
cd /Users/trill/Documents/GitHub/midnightgrind && git status
```

## What to Expect

- **Duration:** This will run for HOURS (potentially 10-20+ hours depending on iteration speed)
- **Progress Files:** PROGRESS_ITERATION_50.md, PROGRESS_ITERATION_100.md, etc.
- **Git Commits:** Regular commits as features are implemented
- **Completion:** Look for "MIDNIGHT_GRIND_LEGENDARY" in the output

## If Something Goes Wrong

If Claude gets stuck or errors out:
1. Check the last PROGRESS file to see where it stopped
2. Review git log to see what was accomplished
3. Restart from that point with adjusted instructions

## Pro Tip

Run this in a tmux or screen session so you can detach and let it run in the background:

```bash
# Start tmux
tmux new -s midnight-grind

# Navigate and start Claude
cd /Users/trill/Documents/GitHub/midnightgrind/prototype
claude
# [paste the /ralph-loop command]

# Detach: Press Ctrl+B, then D
# Reattach later: tmux attach -t midnight-grind
```

---

**Ready to make history? Let's build something legendary! 🏁🔥**
