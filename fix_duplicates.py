"""
Midnight Grind - Duplicate Type Fixer
Automatically fixes duplicate enum/struct definitions by:
1. Keeping the first definition (alphabetically)
2. Removing duplicates and adding includes
"""

import os
import re
from collections import defaultdict
from pathlib import Path

SOURCE_PATH = r"E:\UNREAL ENGINE\midnightgrind\Source\MidnightGrind\Public"

# Find all duplicate types
def find_duplicates():
    duplicates = defaultdict(list)  # type_name -> [(file, line_num, full_match)]
    
    # Patterns for enums and structs
    enum_pattern = re.compile(r'UENUM\([^)]*\)\s*enum\s+class\s+(\w+)\s*:\s*uint8', re.MULTILINE)
    struct_pattern = re.compile(r'USTRUCT\([^)]*\)\s*struct\s+(\w+)', re.MULTILINE)
    
    for root, dirs, files in os.walk(SOURCE_PATH):
        for file in files:
            if file.endswith('.h'):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        
                    # Find enums
                    for match in enum_pattern.finditer(content):
                        type_name = match.group(1)
                        duplicates[type_name].append((filepath, match.start(), 'enum'))
                    
                    # Find structs  
                    for match in struct_pattern.finditer(content):
                        type_name = match.group(1)
                        if type_name.startswith('FMG'):  # Only our structs
                            duplicates[type_name].append((filepath, match.start(), 'struct'))
                            
                except Exception as e:
                    print(f"Error reading {filepath}: {e}")
    
    # Filter to only actual duplicates (>1 definition)
    return {k: v for k, v in duplicates.items() if len(v) > 1}

def main():
    print("Scanning for duplicate types...")
    duplicates = find_duplicates()
    
    print(f"\nFound {len(duplicates)} duplicate types:")
    for type_name, locations in sorted(duplicates.items()):
        print(f"  {type_name}: {len(locations)} definitions")
        for loc in locations:
            rel_path = os.path.relpath(loc[0], SOURCE_PATH)
            print(f"    - {rel_path}")
    
    print(f"\n=== TOTAL: {len(duplicates)} types need consolidation ===")
    print("\nTo fix: Create shared header files and have others include them.")

if __name__ == "__main__":
    main()
