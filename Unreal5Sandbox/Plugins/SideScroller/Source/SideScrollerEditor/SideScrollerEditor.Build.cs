using UnrealBuildTool;

public class SideScrollerEditor : ModuleRules
{
    public SideScrollerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UnrealEd", "SideScrollerRuntime" });
        PrivateDependencyModuleNames.AddRange(new[] { "AssetTools", "UMG", "SlateCore" });
    }
}
