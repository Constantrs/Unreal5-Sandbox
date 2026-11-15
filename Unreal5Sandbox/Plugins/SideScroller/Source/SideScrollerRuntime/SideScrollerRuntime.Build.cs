using UnrealBuildTool;

public class SideScrollerRuntime : ModuleRules
{
    public SideScrollerRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG" });
        PrivateDependencyModuleNames.AddRange(new[] { "InputCore", "Slate", "SlateCore" });
    }
}
