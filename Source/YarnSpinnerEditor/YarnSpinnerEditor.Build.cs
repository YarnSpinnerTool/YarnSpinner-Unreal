// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YarnSpinnerEditor : ModuleRules
{
	public YarnSpinnerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		OptimizeCode = CodeOptimization.Never;

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"MainFrame",
//				"WorkspaceMenuStructure",
			});

		PrivateIncludePaths.AddRange(
			new string[] {
				"YarnSpinnerEditor/Private",
				// "TextAssetEditor/Private/AssetTools",
				// "TextAssetEditor/Private/Factories",
				// "TextAssetEditor/Private/Shared",
				// "TextAssetEditor/Private/Styles",
				// "TextAssetEditor/Private/Toolkits",
				// "TextAssetEditor/Private/Widgets",
			});
   
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "ContentBrowser",
                "Core",
                "CoreUObject",
                "DesktopWidgets",
                "EditorStyle",
                "Engine",
                "InputCore",
                "Projects",
                "Slate",
                "SlateCore",
                "UnrealEd",

                "YarnSpinner",
            });

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"ContentBrowser",
				"Core",
				"CoreUObject",
				"DesktopWidgets",
				"EditorStyle",
				"Engine",
				"InputCore",
				"Projects",
				"Slate",
				"SlateCore",
				"UnrealEd",

				"YarnSpinner",
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
               "Core",
                "CoreUObject",
                "DesktopWidgets",
                "EditorStyle",
                "Engine",
                "InputCore",
                "Projects",
				"AssetTools",
				"UnrealEd",
//				"WorkspaceMenuStructure",
			});
	}
}
