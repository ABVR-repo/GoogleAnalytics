// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2016 gameDNA studio. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class GoogleAnalytics : ModuleRules
	{
		public GoogleAnalytics(TargetInfo Target)
		{
			Definitions.Add("WITH_GOOGLEANALYTICS=1");

			PrivateDependencyModuleNames.AddRange(new string[] { "Analytics" });
			PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
			PublicIncludePathModuleNames.Add("Analytics");

			// Third Party
			var ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/"));

			// Additional Frameworks and Libraries for iOS
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				var ThirdPartyIOS = Path.Combine(ThirdPartyPath, "iOS");

				PublicAdditionalFrameworks.Add(
					new UEBuildFramework(
						"GoogleAnalytics",
						Path.Combine(ThirdPartyIOS, "GoogleAnalytics.embeddedframework.zip")
					)
				);

				PublicFrameworks.AddRange(
					new string[] {
						"CoreTelephony",
						"SystemConfiguration",
						"UIKit",
						"Foundation",
						"CoreGraphics",
						"MobileCoreServices",
						"StoreKit",
						"CFNetwork",
						"CoreData",
						"Security",
						"CoreLocation",
						"iAd",
						"AdSupport",
						"SystemConfiguration"
					}
				);

				PublicAdditionalLibraries.Add("sqlite3");
				PublicAdditionalLibraries.Add("z");
			}
			// Additional Frameworks and Libraries for Android
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
				string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, BuildConfiguration.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(PluginPath, "GoogleAnalytics_APL.xml")));
			}
			// Other platforms
			else
			{
				PublicDependencyModuleNames.AddRange(new string[] { "HTTP", "Json" });
			}
		}
	}
}
