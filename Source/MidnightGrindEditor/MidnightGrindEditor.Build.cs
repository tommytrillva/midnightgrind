// Copyright Midnight Grind. All Rights Reserved.

using UnrealBuildTool;

public class MidnightGrindEditor : ModuleRules
{
	public MidnightGrindEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IWYUSupport = IWYUSupport.Full;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"MidnightGrind", // Main game module
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Editor modules
			"UnrealEd",
			"LevelEditor",
			"PropertyEditor",
			"EditorStyle",
			"EditorFramework",
			"ToolMenus",
			"AssetTools",
			"AssetRegistry",

			// UI
			"Slate",
			"SlateCore",
			"InputCore",

			// Custom asset editing
			"BlueprintGraph",
			"Kismet",
			"GraphEditor",

			// Content browser
			"ContentBrowser",

			// Sequencer (for cinematics tools)
			"Sequencer",
			"MovieScene",
			"MovieSceneTools",
		});

		// This module is editor-only
		if (Target.bBuildEditor == false)
		{
			throw new BuildException("MidnightGrindEditor should only be built for editor targets!");
		}
	}
}
