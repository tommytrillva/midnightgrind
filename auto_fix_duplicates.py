"""
Midnight Grind - Auto Duplicate Type Fixer
Fixes duplicates by commenting out secondary definitions
"""

import os
import re
from collections import defaultdict
from pathlib import Path

SOURCE_PATH = r"E:\UNREAL ENGINE\midnightgrind\Source\MidnightGrind\Public"

def find_all_types():
    """Find all enum and struct definitions with their locations"""
    types = defaultdict(list)
    
    enum_pattern = re.compile(
        r'((?:/\*\*[\s\S]*?\*/\s*)?UENUM\([^)]*\)\s*enum\s+class\s+(\w+)\s*:\s*uint8\s*\{[^}]+\};)',
        re.MULTILINE
    )
    struct_pattern = re.compile(
        r'((?:/\*\*[\s\S]*?\*/\s*)?USTRUCT\([^)]*\)\s*struct\s+(FMG\w+)\s*\{[\s\S]*?GENERATED_BODY\(\))',
        re.MULTILINE
    )
    
    for root, dirs, files in os.walk(SOURCE_PATH):
        for file in files:
            if not file.endswith('.h'):
                continue
            filepath = os.path.join(root, file)
            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                for match in enum_pattern.finditer(content):
                    full_match, type_name = match.groups()
                    types[type_name].append({
                        'file': filepath,
                        'match': full_match,
                        'start': match.start(),
                        'end': match.end(),
                        'kind': 'enum'
                    })
                    
            except Exception as e:
                print(f"Error: {filepath}: {e}")
    
    return {k: v for k, v in types.items() if len(v) > 1}

def fix_duplicates():
    print("Scanning for duplicates...")
    duplicates = find_all_types()
    print(f"Found {len(duplicates)} duplicate types\n")
    
    fixed_count = 0
    
    for type_name, locations in sorted(duplicates.items()):
        # Sort by filepath - keep first alphabetically
        locations.sort(key=lambda x: x['file'])
        canonical = locations[0]
        
        print(f"Fixing {type_name}...")
        print(f"  Keeping: {os.path.relpath(canonical['file'], SOURCE_PATH)}")
        
        # Comment out all others
        for loc in locations[1:]:
            filepath = loc['file']
            rel_path = os.path.relpath(filepath, SOURCE_PATH)
            print(f"  Removing from: {rel_path}")
            
            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                # Find and comment out this specific definition
                old_text = loc['match']
                if old_text in content:
                    canonical_rel = os.path.relpath(canonical['file'], SOURCE_PATH).replace('\\', '/')
                    new_text = f"// {type_name} - REMOVED (duplicate)\n// Canonical definition in: {canonical_rel}\n"
                    content = content.replace(old_text, new_text, 1)
                    
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write(content)
                    fixed_count += 1
                    
            except Exception as e:
                print(f"    Error fixing: {e}")
    
    print(f"\n=== Fixed {fixed_count} duplicate definitions ===")

if __name__ == "__main__":
    fix_duplicates()
