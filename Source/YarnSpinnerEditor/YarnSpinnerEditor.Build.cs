// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class YarnSpinnerEditor : ModuleRules
{
	public YarnSpinnerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		OptimizeCode = CodeOptimization.Never;

		var protobufDir = Path.Combine(PluginDirectory,
            "ThirdParty",
            "protobuf_x64-osx");

        // PrivateIncludePaths.Add(Path.Combine(protobufDir, "include"));

		// PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI=1");

		// // The protobuf header files use '#if _MSC_VER', but this will
		// // trigger -Wundef. Disable unidentified compiler directive warnings.
        bEnableUndefinedIdentifierWarnings = false;

        var yscPath = Path.Combine("YarnSpinner/Tools/osx-x64/ysc");

        PublicDefinitions.Add($"YSC_PATH=TEXT(\"{yscPath}\")");

		if (Target.Platform == UnrealTargetPlatform.Mac) {
			PublicAdditionalLibraries.Add(Path.Combine(protobufDir, "lib", "libprotobufd.a"));
        } else {
            throw new System.PlatformNotSupportedException($"Platform {Target.Platform} is not currently supported.");
        }

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
