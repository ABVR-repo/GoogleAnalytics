// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class GoogleAnalytics : ModuleRules
	{
		public GoogleAnalytics(TargetInfo Target)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "Analytics", "HTTP", "Json" });
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
			PublicIncludePathModuleNames.Add("Analytics");

			string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "ThirdParty");
			string ThirdPartyIOSPath = Path.Combine(ThirdPartyPath, "IOS");

			bool bHasGoogleAnalyticsSDK = false;

			// Additional Frameworks and Libraries for iOS
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PublicFrameworks.AddRange(
					new string[] {
						"CoreData",
						"SystemConfiguration",
						"AdSupport"
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
					PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyIOSPath, "libAdIdAccess.a"));
					PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyIOSPath, "libGoogleAnalyticsServices.a"));
				}
			}
			// Additional Frameworks and Libraries for Android
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				bHasGoogleAnalyticsSDK = true;
				PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, BuildConfiguration.RelativeEnginePath);
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
