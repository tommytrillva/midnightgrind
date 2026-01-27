#!/bin/bash
# MIDNIGHT GRIND - Duplicate Class Cleanup Script
# Generated: 2026-01-26
#
# This script removes duplicate class definitions that prevent compilation.
# KEEP: The larger/more complete implementation in the semantically correct location
# DELETE: The smaller/duplicate implementations
#
# Run with: chmod +x cleanup_duplicates.sh && ./cleanup_duplicates.sh

set -e
cd "$(dirname "$0")/Source/MidnightGrind"

echo "========================================"
echo "MIDNIGHT GRIND - Duplicate Cleanup"
echo "========================================"
echo ""

# Create backup directory
BACKUP_DIR="../_backup_duplicates_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
echo "Backup directory: $BACKUP_DIR"
echo ""

# Function to safely remove a file (backs up first)
safe_remove() {
    local file="$1"
    if [ -f "$file" ]; then
        local dir=$(dirname "$file")
        mkdir -p "$BACKUP_DIR/$dir"
        cp "$file" "$BACKUP_DIR/$file"
        rm "$file"
        echo "  Removed: $file"
    else
        echo "  Skipped (not found): $file"
    fi
}

echo "1. MGRaceGameMode - KEEP: GameModes/, DELETE: Core/"
echo "   (GameModes/ is larger: 490+691 lines vs Core/: 430+586 lines)"
safe_remove "Public/Core/MGRaceGameMode.h"
safe_remove "Private/Core/MGRaceGameMode.cpp"
echo ""

echo "2. MGAchievementSubsystem - KEEP: Achievements/, DELETE: Core/, Progression/"
echo "   (Achievements/ is largest: 635+977 lines)"
safe_remove "Public/Core/MGAchievementSubsystem.h"
safe_remove "Private/Core/MGAchievementSubsystem.cpp"
safe_remove "Public/Progression/MGAchievementSubsystem.h"
safe_remove "Private/Progression/MGAchievementSubsystem.cpp"
echo ""

echo "3. MGLeaderboardSubsystem - KEEP: Social/, DELETE: Leaderboard/"
echo "   (Social/ is larger: 590+650 lines vs Leaderboard/: 458+456 lines)"
safe_remove "Public/Leaderboard/MGLeaderboardSubsystem.h"
safe_remove "Private/Leaderboard/MGLeaderboardSubsystem.cpp"
echo ""

echo "4. MGPoliceSubsystem - KEEP: Police/, DELETE: World/"
echo "   (Police/ is much larger: 1621+1671 lines vs World/: 487+508 lines)"
safe_remove "Public/World/MGPoliceSubsystem.h"
safe_remove "Private/World/MGPoliceSubsystem.cpp"
echo ""

echo "5. MGPostProcessSubsystem - KEEP: PostProcess/, DELETE: Visual/"
echo "   (PostProcess/ is larger: 778+737 lines vs Visual/: 411+453 lines)"
safe_remove "Public/Visual/MGPostProcessSubsystem.h"
safe_remove "Private/Visual/MGPostProcessSubsystem.cpp"
echo ""

echo "6. MGRivalSubsystem - KEEP: Social/, DELETE: Rival/"
echo "   (Social/ is larger: 632+1125 lines vs Rival/: 412+865 lines)"
safe_remove "Public/Rival/MGRivalSubsystem.h"
safe_remove "Private/Rival/MGRivalSubsystem.cpp"
echo ""

echo "7. MGTrafficSubsystem - KEEP: Traffic/, DELETE: World/"
echo "   (Traffic/ is larger: 1063+666 lines vs World/: 410+470 lines)"
safe_remove "Public/World/MGTrafficSubsystem.h"
safe_remove "Private/World/MGTrafficSubsystem.cpp"
echo ""

echo "8. MGWeatherSubsystem - KEEP: Weather/, DELETE: Environment/"
echo "   (Weather/ is slightly larger: 674+905 lines vs Environment/: 584+933 lines)"
safe_remove "Public/Environment/MGWeatherSubsystem.h"
safe_remove "Private/Environment/MGWeatherSubsystem.cpp"
echo ""

echo "========================================"
echo "MANUAL FIXES REQUIRED:"
echo "========================================"
echo ""
echo "9. MGLoadingScreenWidget - DEFINED IN TWO PLACES:"
echo "   - Public/UI/MGMenuWidgets.h (contains multiple widgets)"
echo "   - Public/UI/MGLoadingScreenWidget.h (standalone)"
echo "   ACTION: Remove UMGLoadingScreenWidget class from MGMenuWidgets.h"
echo ""
echo "10. MGTrackDataAsset - DEFINED IN TWO PLACES:"
echo "    - Public/Track/MGTrackSpline.h (contains multiple classes)"
echo "    - Public/Track/MGTrackDataAssets.h (standalone)"
echo "    ACTION: Remove UMGTrackDataAsset class from MGTrackSpline.h"
echo ""
echo "========================================"
echo "Cleanup complete!"
echo "Backup saved to: $BACKUP_DIR"
echo ""
echo "Next steps:"
echo "1. Fix MGMenuWidgets.h (remove UMGLoadingScreenWidget class)"
echo "2. Fix MGTrackSpline.h (remove UMGTrackDataAsset class)"
echo "3. Update any #include statements that reference deleted files"
echo "4. Run: sudo xcode-select -s /Applications/Xcode.app"
echo "5. Rebuild the project"
echo "========================================"
