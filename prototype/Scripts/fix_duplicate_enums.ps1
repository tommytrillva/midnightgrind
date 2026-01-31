# Fix duplicate enum definitions in Midnight Grind codebase
# Run from prototype folder

$basePath = "C:\Users\trill\Documents\GitHub\midnightgrind\prototype\Source\MidnightGrind\Public"

# Define which files to keep enums in (canonical) vs remove from
$enumFixes = @(
    @{
        Enum = "EMGDrivetrainType"
        KeepIn = "Content/MGVehicleContentAssets.h"
        RemoveFrom = "VehicleClass/MGVehicleClassSubsystem.h"
    },
    @{
        Enum = "EMGVehicleEra"
        KeepIn = "Data/MGVehicleDatabase.h"
        RemoveFrom = "VehicleClass/MGVehicleClassSubsystem.h"
    },
    @{
        Enum = "EMGTimeOfDay"
        KeepIn = "PostProcess/MGPostProcessSubsystem.h"
        RemoveFrom = @("VFX/MGEnvironmentVFXManager.h", "Weather/MGWeatherSubsystem.h")
    },
    @{
        Enum = "EMGMuteReason"
        KeepIn = "Report/MGReportSubsystem.h"
        RemoveFrom = "VoiceChat/MGVoiceChatSubsystem.h"
    },
    @{
        Enum = "EMGWeatherType"
        KeepIn = "Weather/MGWeatherSubsystem.h"
        RemoveFrom = "VFX/MGEnvironmentVFXManager.h"
    }
)

Write-Host "Fixing duplicate enum definitions..."
Write-Host ""

foreach ($fix in $enumFixes) {
    $enum = $fix.Enum
    Write-Host "Processing $enum..."
    
    $removeFromList = if ($fix.RemoveFrom -is [array]) { $fix.RemoveFrom } else { @($fix.RemoveFrom) }
    
    foreach ($removeFile in $removeFromList) {
        $fullPath = Join-Path $basePath $removeFile
        if (Test-Path $fullPath) {
            Write-Host "  Removing from: $removeFile"
            
            $content = Get-Content $fullPath -Raw
            
            # Match the UENUM block for this enum
            # Pattern: UENUM followed by enum class EMGxxx until the closing };
            $pattern = "(?s)/\*\*[\s\S]*?\*/\s*UENUM\([^)]*\)\s*enum\s+class\s+$enum\s*:\s*uint8\s*\{[^}]+\};"
            
            if ($content -match $pattern) {
                $content = $content -replace $pattern, "// $enum defined in $($fix.KeepIn)"
                Set-Content $fullPath $content -NoNewline
                Write-Host "    ✓ Removed duplicate enum"
            } else {
                Write-Host "    ⚠ Pattern not found, trying alternate..."
                # Try simpler pattern
                $pattern2 = "UENUM\([^)]*\)\s*enum\s+class\s+$enum\s*:\s*uint8\s*\{[^}]+\};"
                if ($content -match $pattern2) {
                    $content = $content -replace $pattern2, "// $enum defined in $($fix.KeepIn)"
                    Set-Content $fullPath $content -NoNewline
                    Write-Host "    ✓ Removed (alternate pattern)"
                } else {
                    Write-Host "    ✗ Could not find enum to remove"
                }
            }
        } else {
            Write-Host "  ✗ File not found: $fullPath"
        }
    }
}

Write-Host ""
Write-Host "Done! Now add includes for the canonical headers where needed."
