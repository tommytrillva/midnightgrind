// Copyright Midnight Grind. All Rights Reserved.

using UnrealBuildTool;

public class MidnightGrind : ModuleRules
{
	public MidnightGrind(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Enable IWYU (Include What You Use)
		bEnforceIWYU = true;

		// Enable exceptions for third-party library compatibility
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			// Core modules
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",

			// Chaos Physics (for vehicle physics)
			"ChaosVehicles",
			"PhysicsCore",
			"Chaos",

			// UI
			"UMG",
			"Slate",
			"SlateCore",
			"CommonUI",

			// Input
			"EnhancedInput",

			// Gameplay systems
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",

			// Networking
			"NetCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",

			// Audio
			"MetasoundEngine",
			"MetasoundFrontend",
			"AudioMixer",

			// Rendering
			"RenderCore",
			"RHI",

			// Animation
			"AnimGraphRuntime",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Additional private dependencies
			"Niagara",
			"HTTP",
			"Json",
			"JsonUtilities",
			"AIModule",
			"NavigationSystem",
			"GeometryCollectionEngine",
		});

		// Platform-specific dependencies
		// Steam disabled for local development
		// if (Target.Platform == UnrealTargetPlatform.Win64)
		// {
		// 	PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
		// }

		// Racing Wheel / DirectInput support (Windows only)
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add("dinput8.lib");
			PublicAdditionalLibraries.Add("dxguid.lib");
			PublicDefinitions.Add("WITH_DIRECTINPUT_WHEEL=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_DIRECTINPUT_WHEEL=0");
		}

		// Editor-only dependencies
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"LevelEditor",
				"PropertyEditor",
				"EditorStyle",
				"EditorFramework",
				"ToolMenus",
			});
		}

		// Include paths
		PublicIncludePaths.AddRange(new string[]
		{
			// Add public include paths here
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			// Add private include paths here
		});

		// Definitions
		PublicDefinitions.AddRange(new string[]
		{
			// "WITH_STEAM=1" - Handled by plugin
		});

		// Disable specific warnings if needed
		// bEnableUndefinedIdentifierWarnings = false;

		// Optimization settings
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
