using UnrealBuildTool;

public class BP2AI : ModuleRules
{
    public BP2AI(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new [] {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new [] {
            "InputCore",
            "Slate",
            "SlateCore",
            "ApplicationCore",
            "UnrealEd",
            "BlueprintGraph",
            "GraphEditor",
            "Kismet",
            "ToolMenus",
            "EditorFramework",
            "KismetCompiler",
            "Projects",
            "UMG",
            "ToolWidgets",
            "EditorStyle",
            "Blutility",
            "UMGEditor",
            "InputBlueprintNodes",
            "EnhancedInput",
            "HTTP",
            "WebBrowser",
            "Json",
            "JsonUtilities",
            "ContentBrowser",
            "AssetRegistry"
        });

        if (target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("CEF3Utils");
        }
    }
}