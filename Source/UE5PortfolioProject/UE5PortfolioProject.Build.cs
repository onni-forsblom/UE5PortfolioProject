// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE5PortfolioProject : ModuleRules
{
	public UE5PortfolioProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
