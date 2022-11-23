// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class YarnSpinner : ModuleRules
{
	public YarnSpinner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		var protobufDir = Path.Combine(PluginDirectory,
            "ThirdParty",
            "protobuf_x64-osx");

        PublicIncludePaths.Add(Path.Combine(protobufDir, "include"));

		PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI=1");
        
		if (Target.Platform == UnrealTargetPlatform.Mac) {
			PublicAdditionalLibraries.Add(Path.Combine(protobufDir, "lib", "libprotobuf.a"));
        } else {
            throw new System.PlatformNotSupportedException($"Platform {Target.Platform} is not currently supported.");
        }

		// The protobuf header files use '#if _MSC_VER', but this will
		// trigger -Wundef. Disable unidentified compiler directive warnings.
        bEnableUndefinedIdentifierWarnings = false;

        OptimizeCode = CodeOptimization.Never;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                // ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}