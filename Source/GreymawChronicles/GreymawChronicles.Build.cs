using UnrealBuildTool;

public class GreymawChronicles : ModuleRules
{
    public GreymawChronicles(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "HTTP",
            "Json",
            "JsonUtilities",
            "UMG",
            "Slate",
            "SlateCore",
            "SQLiteCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects",
            "EngineSettings"
        });
    }
}
