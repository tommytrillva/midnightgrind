// Copyright Midnight Grind. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MidnightGrindEditorTarget : TargetRules
{
	public MidnightGrindEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[] { "MidnightGrind", "MidnightGrindEditor" });

		// Editor-specific settings
		bBuildDeveloperTools = true;
		bBuildWithEditorOnlyData = true;
	}
}
