// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class KBSEditorTarget : TargetRules
{
	public KBSEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("KBS");
        ExtraModuleNames.AddRange(new string[] { "KBS", "KBSTests" });
        if (Configuration != UnrealTargetConfiguration.Shipping)
        {
            ExtraModuleNames.Add("KBSTests");
        }
    }
}
