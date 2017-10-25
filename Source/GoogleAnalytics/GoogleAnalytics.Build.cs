// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA Ltd. All Rights Reserved.

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

			string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "ThirdParty");
			string ThirdPartyIOSPath = Path.Combine(ThirdPartyPath, "IOS");

			bool bHasGoogleAnalyticsSDK = false;

            // Get Project Path
            string ProjectPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../"));
            if (Target.ProjectFile != null)
            {
                ProjectPath = Path.GetDirectoryName(Target.ProjectFile.ToString());
            }

            // Get Settings from Config Cache
            var Ini = UnrealBuildTool.ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine, new DirectoryReference(ProjectPath), Target.Platform);
			string SettingsPath = "/Script/GoogleAnalytics.GoogleAnalyticsSettings";

			bool bEnableIDFACollection = false;

			if (!Ini.GetBool(SettingsPath, "bEnableIDFACollection", out bEnableIDFACollection))
				bEnableIDFACollection = false;

			// Additional Frameworks and Libraries for iOS
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PublicFrameworks.AddRange(
					new string[] {
						"CoreData",
						"SystemConfiguration"
					}
				);

				PublicAdditionalLibraries.Add("sqlite3");
				PublicAdditionalLibraries.Add("z");

				bHasGoogleAnalyticsSDK = (Directory.Exists(ThirdPartyPath) &&
										  Directory.Exists(ThirdPartyIOSPath) &&
										  File.Exists(Path.Combine(ThirdPartyIOSPath, "libAdIdAccess.a")) &&
										  File.Exists(Path.Combine(ThirdPartyIOSPath, "libGoogleAnalyticsServices.a")));

				if (bHasGoogleAnalyticsSDK)
				{
					PublicIncludePaths.Add(ThirdPartyIOSPath);
					PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyIOSPath, "libGoogleAnalyticsServices.a"));

					if (bEnableIDFACollection)
					{
						PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyIOSPath, "libAdIdAccess.a"));
						PublicFrameworks.AddRange(
							new string[] {
								"AdSupport"
							}
						);
					}
				}

				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add(new ReceiptProperty("IOSPlugin", Path.Combine(PluginPath, "GoogleAnalytics_UPL_IOS.xml")));
			}
			// Additional Frameworks and Libraries for Android
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				bHasGoogleAnalyticsSDK = true;
				PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(PluginPath, "GoogleAnalytics_UPL_Android.xml")));
			}
			// Other platforms
			else
			{
				bHasGoogleAnalyticsSDK = true;
			}

			if (bHasGoogleAnalyticsSDK)
			{
				Definitions.Add("WITH_GOOGLEANALYTICS=1");
			}
			else
			{
				Definitions.Add("WITH_GOOGLEANALYTICS=0");
				Log.TraceError("Google Analytics SDK not installed!");
			}
		}
	}
}
