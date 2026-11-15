using UnrealBuildTool;

public class StageSelectHostEditorTarget : TargetRules
{
    public StageSelectHostEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("StageSelectHost");
    }
}
