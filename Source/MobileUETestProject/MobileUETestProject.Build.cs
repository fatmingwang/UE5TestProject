// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MobileUETestProject : ModuleRules
{
	public MobileUETestProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Json",              // Required for JSON serialization
			"JsonUtilities",     // Required for JSON helper functions
			"UMG"                // Required for the maze control widget (UUserWidget, buttons, sliders, etc.)
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
