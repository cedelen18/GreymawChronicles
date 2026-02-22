using UnrealBuildTool;
using System.Collections.Generic;

public class GreymawChroniclesEditorTarget : TargetRules
{
    public GreymawChroniclesEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        bOverrideBuildEnvironment = true;
        ExtraModuleNames.Add("GreymawChronicles");
    }
}
