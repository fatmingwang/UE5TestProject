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

		if (Target.Type == TargetType.Editor)
		{
			// Editor-only: powers the in-editor maze parameter tool (UEditorUtilityWidget), which
			// lets the maze be adjusted and regenerated without entering Play mode. NOTE: this makes
			// MazeEditorUtilityWidget.h/.cpp compile only correctly for the Editor target - if the
			// MobileUETestProject (Game) target is ever built, these two files need to move out into
			// their own editor-only module first (that move needs an editor restart to register).
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "Blutility", "UMGEditor" });
		}

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
