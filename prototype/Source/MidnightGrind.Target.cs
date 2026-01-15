// Copyright Midnight Grind. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MidnightGrindTarget : TargetRules
{
	public MidnightGrindTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[] { "MidnightGrind" });

		// Enable Steam on supported platforms
		if (Target.Platform == UnrealTargetPlatform.Win64 ||
		    Target.Platform == UnrealTargetPlatform.Mac ||
		    Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Steam configuration is handled by plugin
		}
	}
}
