using UnrealBuildTool;

public class KBSEditorTools : ModuleRules
{
	public KBSEditorTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "Json" });
	}
}
