# MIDNIGHT GRIND - Setup Requirements

## CRITICAL: Before Building

### 1. Xcode Configuration (Required for macOS)

Your Mac is using CommandLineTools instead of full Xcode. Run this command:

```bash
sudo xcode-select -s /Applications/Xcode.app
```

Enter your Mac password when prompted.

To verify:
```bash
xcode-select -p
# Should output: /Applications/Xcode.app/Contents/Developer
```

### 2. Engine Version

The project is now configured for **Unreal Engine 5.2** (changed from 5.7).

UE 5.2 is installed at: `/Users/Shared/Epic Games/UE_5.2`

---

## Quick Build Steps

After fixing xcode-select:

### Option 1: Generate Project Files (Command Line)
```bash
cd /Users/trill/Documents/GitHub/midnightgrind/prototype
"/Users/Shared/Epic Games/UE_5.2/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh" -project="$(pwd)/MidnightGrind.uproject"
```

### Option 2: Open in Unreal Editor (Recommended)
1. Open Epic Games Launcher
2. Go to Library
3. Find UE 5.2
4. Click "Launch"
5. File → Open Project → Navigate to `MidnightGrind.uproject`
6. Let it compile shaders and build

---

## Potential UE 5.2 Compatibility Notes

The following modules are used and should be available in 5.2:

| Module | Status | Notes |
|--------|--------|-------|
| ChaosVehicles | Should work | Core vehicle physics |
| MetasoundEngine | Should work | Available since UE 5.0 |
| MetasoundFrontend | Should work | Available since UE 5.0 |
| CommonUI | Should work | May need plugin enabled |
| EnhancedInput | Should work | Core UE5 feature |
| GameplayAbilities | Should work | Plugin available |
| Niagara | Should work | Core UE5 feature |

If compilation fails, check:
1. All required plugins are enabled in MidnightGrind.uproject
2. Module names match UE 5.2 API

---

## After Successful Build

1. **Run Editor Scripts**
   - Copy `EditorScripts/` to `Content/Python/`
   - Window → Developer Tools → Output Log
   - Change dropdown to "Python"
   - Type: `exec(open("00_RunAllSetup.py").read())`

2. **Configure Project Settings**
   - Edit → Project Settings
   - Maps & Modes → Default GameMode: `BP_MidnightGrindGameMode`
   - Maps & Modes → Game Instance: `BP_MidnightGrindGameInstance`

3. **Test Basic Functionality**
   - Open L_TestTrack map
   - Press Play
   - Verify basic systems work

---

## Troubleshooting

### "Module not found" error
- Some UE 5.7 modules may have different names in 5.2
- Check Engine/Source/Runtime folder for exact module names
- Update Build.cs if needed

### Shader compilation hangs
- First-time shader compilation can take 10-30 minutes
- Let it complete without interruption

### Plugin not found
- Open MidnightGrind.uproject in text editor
- Ensure required plugins have `"Enabled": true`

---

*Created: January 26, 2026*
*Project: MIDNIGHT GRIND*
*Engine: UE 5.2*
