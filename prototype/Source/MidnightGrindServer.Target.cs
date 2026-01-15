// Copyright Midnight Grind. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MidnightGrindServerTarget : TargetRules
{
	public MidnightGrindServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[] { "MidnightGrind" });

		// Server-specific settings
		bUseChecksInShipping = true;
		bUseLoggingInShipping = true;

		// Disable unnecessary features for dedicated server
		bCompileAgainstEngine = true;
		bCompileAgainstCoreUObject = true;
		bCompileAgainstApplicationCore = true;

		// Server doesn't need these
		bBuildWithEditorOnlyData = false;
	}
}
