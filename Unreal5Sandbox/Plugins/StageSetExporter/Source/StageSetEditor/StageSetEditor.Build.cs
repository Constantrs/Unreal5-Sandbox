using UnrealBuildTool;

public class StageSetEditor : ModuleRules
{
    public StageSetEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "StageSetRuntime", "Blutility", "UMG", "UnrealEd", "SlateCore"
        });
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Slate", "PropertyEditor", "AssetTools", "UMGEditor"
        });
    }
}
