// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tactics : ModuleRules
{
	public Tactics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Tactics",
			"Tactics/Core",
			"Tactics/Characters/Player",
			"Tactics/Characters/Enemy",
			"Tactics/Components",
			"Tactics/Managers",
			"Tactics/UI",
			"Tactics/AI",
			"Tactics/Utils",
			"Tactics/GameModes/Strategy",
			"Tactics/GameModes/Strategy/UI",
			"Tactics/GameModes/TwinStick",
			"Tactics/GameModes/TwinStick/AI",
			"Tactics/GameModes/TwinStick/Gameplay",
			"Tactics/GameModes/TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
