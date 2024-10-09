using UnrealBuildTool;

public class MyCustomNodesEditor : ModuleRules
{
    public MyCustomNodesEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "MyCustomNodes",
                "UnrealEd",
                "AudioEditor", // for loading audio files
                "MetasoundEditor",
            }
        );
    }
}