// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Modulo_1_consega : ModuleRules
{
	public Modulo_1_consega(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "NavigationSystem", "GameplayTasks"});
	}
}
