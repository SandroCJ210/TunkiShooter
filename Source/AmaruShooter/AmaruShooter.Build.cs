// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AmaruShooter : ModuleRules
{
	public AmaruShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities",
			"GameplayTasks", "GameplayTags"
		]);
	}
}
