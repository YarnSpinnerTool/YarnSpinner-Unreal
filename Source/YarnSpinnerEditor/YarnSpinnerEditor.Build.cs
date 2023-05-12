
using UnrealBuildTool;
using System.IO;

public class YarnSpinnerEditor : ModuleRules
{
	public YarnSpinnerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string YscPath;

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
			// The protobuf header files use '#if _MSC_VER', but this will
			// trigger -Wundef. Disable unidentified compiler directive warnings.
			bEnableUndefinedIdentifierWarnings = false;

	        YscPath = ToolPath(Target) + "ysc";
        }
        else if (Target.Platform == UnrealTargetPlatform.Win64)
        {
	        YscPath = ToolPath(Target) + "ysc.exe";
        }
        else
        {
            throw new System.PlatformNotSupportedException("Platform " + Target.Platform + " is not currently supported.");
        }

        PublicDefinitions.Add("YSC_PATH=TEXT(\"" + YscPath + "\")");
        
		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"MainFrame",
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
				"YSProtobuf",
			});
	}
	
	public string ToolPath(ReadOnlyTargetRules Target)
	{
		return "YarnSpinner-Unreal/Tools/" + Target.Platform + "/";
    }
}
