// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

using System;
using System.IO;
using System.Collections.Generic;
using Tools.DotNETCommon;

namespace UnrealBuildTool.Rules
{
	public class GoogleAnalytics : ModuleRules
	{
		public GoogleAnalytics(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateDependencyModuleNames.AddRange(new string[] { "Analytics", "HTTP", "Json" });
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
			PrivateIncludePathModuleNames.AddRange(new string[] { "Settings" });
			PublicIncludePathModuleNames.Add("Analytics");

            // Get Project Path
            string ProjectPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../"));
            if (Target.ProjectFile != null)
            {
                ProjectPath = Path.GetDirectoryName(Target.ProjectFile.ToString());
            }

            // Get Settings from Config Cache
            var Ini = UnrealBuildTool.ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine, new DirectoryReference(ProjectPath), Target.Platform);

			// Additional Frameworks and Libraries for iOS
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "GoogleAnalytics_UPL_IOS.xml"));
			}
			// Additional Frameworks and Libraries for Android
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "GoogleAnalytics_UPL_Android.xml"));
			}
		}
	}
}
