// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class KBSEditorTarget : TargetRules
{
	public KBSEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("KBS");
        ExtraModuleNames.AddRange(new string[] { "KBS", "KBSTests" });
        ExtraModuleNames.Add("KBSEditorTools");
        if (Configuration != UnrealTargetConfiguration.Shipping)
        {
            ExtraModuleNames.Add("KBSTests");
        }
    }
}
