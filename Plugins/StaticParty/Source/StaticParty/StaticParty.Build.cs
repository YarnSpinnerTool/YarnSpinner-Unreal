// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class StaticParty : ModuleRules
{
	public StaticParty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
  
        var protobufDir = Path.Combine("$(ProjectDir)",
            "Plugins",
            "ThirdParty",
            "protobuf_x64-osx");
  
        PublicIncludePaths.Add(Path.Combine(protobufDir, "include"));
        
        PublicAdditionalLibraries.Add(Path.Combine(protobufDir, "lib", "libprotobuf.a"));
        
        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // The protobuf header files use '#if _MSC_VER', but this will
            // trigger -Wundef. Disable unidentified compiler directive warnings.
            bEnableUndefinedIdentifierWarnings = false;
        }
		
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
