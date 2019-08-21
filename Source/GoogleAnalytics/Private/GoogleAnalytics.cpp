// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "GoogleAnalytics.h"
#include "GoogleAnalyticsProvider.h"
#include "Runtime/Core/Public/Misc/SecureHash.h"
#include "Runtime/Online/HTTP/Public/PlatformHttp.h"
#include "Interfaces/IAnalyticsProviderModule.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "Analytics.h"
#include "Engine.h"
#include "Core.h"
#include <string>
#include "ISettingsModule.h"
#include "GoogleAnalyticsSettings.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogGoogleAnalytics);

#define LOCTEXT_NAMESPACE "GoogleAnalytics"

IMPLEMENT_MODULE(FAnalyticsGoogleAnalytics, GoogleAnalytics)

TSharedPtr<IAnalyticsProvider> FAnalyticsProviderGoogleAnalytics::Provider;

void FAnalyticsGoogleAnalytics::StartupModule()
{
	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "GoogleAnalytics",
			LOCTEXT("RuntimeSettingsName", "Google Analytics"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Google Analytics plugin"),
			GetMutableDefault<UGoogleAnalyticsSettings>()
		);
	}
}

void FAnalyticsGoogleAnalytics::ShutdownModule()
{
	FAnalyticsProviderGoogleAnalytics::Destroy();

	// Unregister settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "GoogleAnalytics");
	}
}

TSharedPtr<IAnalyticsProvider> FAnalyticsGoogleAnalytics::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	if (GetConfigValue.IsBound())
	{
		FString TrackingId = GetConfigValue.Execute(TEXT("TrackingId"), true);

		const FString SendInterval = GetConfigValue.Execute(TEXT("SendInterval"), false);
		return FAnalyticsProviderGoogleAnalytics::Create(TrackingId, FCString::Atoi(*SendInterval));
	}
	else
	{
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("GoogleAnalytics::CreateAnalyticsProvider called with an unbound config delegate"));
	}
	return nullptr;
}

// Provider
FAnalyticsProviderGoogleAnalytics::FAnalyticsProviderGoogleAnalytics(const FString TrackingId, const int32 SendInterval) :
	ApiTrackingId(TrackingId),
	bHasSessionStarted(false),
	bSessionStartedSent(false),
	bAnonymizeIp(false),
	Interval(SendInterval)
{
}

FAnalyticsProviderGoogleAnalytics::~FAnalyticsProviderGoogleAnalytics()
{
	if (bHasSessionStarted)
	{
		EndSession();
	}
}

bool FAnalyticsProviderGoogleAnalytics::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (!bHasSessionStarted && ApiTrackingId.Len() > 0)
	{
		// Settings
		const UGoogleAnalyticsSettings* DefaultSettings = GetDefault<UGoogleAnalyticsSettings>();

		FString UniversalCidBuffer("");
		GConfig->GetString(TEXT("GoogleAnalytics"), TEXT("UniversalCid"), UniversalCidBuffer, GEngineIni);

		if (UniversalCidBuffer.Len() > 0)
		{
			UniversalCid = UniversalCidBuffer;
		}
		else
		{
			UniversalCid = FMD5::HashAnsiString(*("UniversalCid" + FDateTime::Now().ToString()));
		}

		GConfig->SetString(TEXT("GoogleAnalytics"), TEXT("UniversalCid"), *UniversalCid, GEngineIni);
		GConfig->Flush(false, GEngineIni);

		RecordScreen("Game Launched");

		if (Attributes.Num() > 0)
		{
			RecordEvent(TEXT("SessionAttributes"), Attributes);
		}
		bHasSessionStarted = true;
	}

	return bHasSessionStarted;
}

void FAnalyticsProviderGoogleAnalytics::EndSession()
{
	if (bHasSessionStarted)
	{
		bHasSessionStarted = false;
		bSessionStartedSent = false;
		bAnonymizeIp = false;
	}
}

void FAnalyticsProviderGoogleAnalytics::SetTrackingId(const FString& TrackingId)
{
	const TArray<FAnalyticsEventAttribute> Attributes;
	EndSession();
	ApiTrackingId = TrackingId;
	StartSession(Attributes);
}

FString FAnalyticsProviderGoogleAnalytics::GetTrackingId()
{
	return ApiTrackingId;
}

void FAnalyticsProviderGoogleAnalytics::SetAnonymizeIp(const bool Anonymize)
{
	bAnonymizeIp = Anonymize;
}

void FAnalyticsProviderGoogleAnalytics::FlushEvents()
{
	// Ignored
	UE_LOG(LogGoogleAnalytics, Display, TEXT("FAnalyticsProviderGoogleAnalytics::FlushEvents - ignoring call"));
}

void FAnalyticsProviderGoogleAnalytics::SetUserID(const FString& InUserId)
{
	if (bHasSessionStarted)
	{
		UserId = InUserId;
	}
}

FString FAnalyticsProviderGoogleAnalytics::GetUserID() const
{
	if (bHasSessionStarted)
	{
		return UserId;
	}
	return FString("");
}

void FAnalyticsProviderGoogleAnalytics::SetGender(const FString& InGender)
{
	if (bHasSessionStarted)
	{
		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), TEXT("Gender")));
		RecordEvent(InGender, Params);
	}
}

void FAnalyticsProviderGoogleAnalytics::SetAge(const int32 InAge)
{
	if (bHasSessionStarted)
	{
		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), TEXT("Age")));
		Params.Add(FAnalyticsEventAttribute(TEXT("Value"), InAge));
		RecordEvent(FString::FromInt(InAge), Params);
	}
}

void FAnalyticsProviderGoogleAnalytics::SetLocation(const FString& InLocation)
{
	if (bHasSessionStarted)
	{
		Location = InLocation;
	}
}

FString FAnalyticsProviderGoogleAnalytics::GetSessionID() const
{
	// Ignored
	UE_LOG(LogGoogleAnalytics, Display, TEXT("FAnalyticsProviderGoogleAnalytics::GetSessionID - ignoring call"));

	return FString();
}

bool FAnalyticsProviderGoogleAnalytics::SetSessionID(const FString& InSessionID)
{
	// Ignored
	UE_LOG(LogGoogleAnalytics, Display, TEXT("FAnalyticsProviderGoogleAnalytics::SetSessionID - ignoring call"));

	return true;
}

void FAnalyticsProviderGoogleAnalytics::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		if (EventName.Len() > 0)
		{
			FString Action = FString(EventName);
			FString Category = FString("Default Category");
			FString Label = FString("");
			float Value = 0;

			for (int i = 0; i < Attributes.Num(); i++)
			{
				if (Attributes[i].AttrName.Equals("Category") && Attributes[i].ToString().Len() > 0)
				{
					Category = Attributes[i].ToString();
				}
				else if (Attributes[i].AttrName.Equals("Label") && Attributes[i].ToString().Len() > 0)
				{
					Label = Attributes[i].ToString();
				}
				else if (Attributes[i].AttrName.Equals("Value"))
				{
					Value = FCString::Atof(*Attributes[i].ToString());
				}
			}

			const TArray<FCustomDimension> CustomDimensions = BuildCustomDimensionsFromAttributes(Attributes);
			const TArray<FCustomMetric> CustomMetrics = BuildCustomMetricsFromAttributes(Attributes);

			MakeOnlineRequest("event", "&ec=" + FPlatformHttp::UrlEncode(Category) + "&ea=" + FPlatformHttp::UrlEncode(Action) + "&el=" + FPlatformHttp::UrlEncode(Label) + "&ev=" + FString::FromInt(Value), CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordScreen(const FString& ScreenName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (ScreenName.Len() > 0)
		{
			MakeOnlineRequest("pageview", "&dp=" + FPlatformHttp::UrlEncode(ScreenName) + "&dt=" + FPlatformHttp::UrlEncode(ScreenName), CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordSocialInteraction(const FString& Network, const FString& Action, const FString& Target, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (Network.Len() > 0 && Action.Len() > 0)
		{
			MakeOnlineRequest("social", "&sn=" + FPlatformHttp::UrlEncode(Network) + "&sa=" + FPlatformHttp::UrlEncode(Action) + "&st=" + FPlatformHttp::UrlEncode(Target), CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordUserTiming(const FString& Category, const int32 Value, const FString& Name, const FString& Label, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (Category.Len() > 0)
		{
			MakeOnlineRequest("timing", "&utc=" + FPlatformHttp::UrlEncode(Category) + "&utv=" + FPlatformHttp::UrlEncode(Name) + "&utt=" + FString::FromInt(Value) + "&utl=" + FPlatformHttp::UrlEncode(Label), CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordItemPurchase(const FString& ItemId, int ItemQuantity, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
	if (bHasSessionStarted)
	{
		FString Currency = FString("");
		int32 PerItemCost = 0;

		for (int i = 0; i < EventAttrs.Num(); i++)
		{
			if (EventAttrs[i].AttrName.Equals("Currency") && EventAttrs[i].ToString().Len() > 0)
			{
				Currency = EventAttrs[i].ToString();
			}
			if (EventAttrs[i].AttrName.Equals("PerItemCost"))
			{
				PerItemCost = FCString::Atoi(*EventAttrs[i].ToString());
			}
		}

		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), TEXT("Item Purchase")));
		Params.Add(FAnalyticsEventAttribute(TEXT("Label"), FString::Printf(TEXT("Cost: %d %s"), PerItemCost, *Currency)));
		Params.Add(FAnalyticsEventAttribute(TEXT("Value"), ItemQuantity));
		RecordEvent(ItemId, Params);
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
	if (bHasSessionStarted)
	{
		if (GameCurrencyType.Len() > 0)
		{
			FString RealCurrencyType = FString("USD");
			float RealMoneyCost = 0;
			FString PaymentProvider = FString("Default Provider");

			for (int i = 0; i < EventAttrs.Num(); i++)
			{
				if (EventAttrs[i].AttrName.Equals("RealCurrencyType") && EventAttrs[i].ToString().Len() > 0)
				{
					RealCurrencyType = EventAttrs[i].ToString();
				}
				if (EventAttrs[i].AttrName.Equals("RealMoneyCost") && EventAttrs[i].ToString().Len() > 0)
				{
					RealMoneyCost = FCString::Atof(*EventAttrs[i].ToString());
				}
				if (EventAttrs[i].AttrName.Equals("PaymentProvider") && EventAttrs[i].ToString().Len() > 0)
				{
					PaymentProvider = EventAttrs[i].ToString();
				}
			}

			FString TransactionId = FMD5::HashAnsiString(*(GameCurrencyType + RealCurrencyType + PaymentProvider + FDateTime::Now().ToString()));

			const TArray<FCustomDimension> CustomDimensions = BuildCustomDimensionsFromAttributes(EventAttrs);
			const TArray<FCustomMetric> CustomMetrics = BuildCustomMetricsFromAttributes(EventAttrs);

			MakeOnlineRequest("transaction", "&ti=" + FPlatformHttp::UrlEncode(TransactionId) + "&ta=" + FPlatformHttp::UrlEncode(PaymentProvider) + "&tr=" + FString::SanitizeFloat(RealMoneyCost) + "&ts=0&tt=0&cu=" + FPlatformHttp::UrlEncode(RealCurrencyType), CustomDimensions, CustomMetrics);
			MakeOnlineRequest("item", "&ti=" + FPlatformHttp::UrlEncode(TransactionId) + "&in=" + FPlatformHttp::UrlEncode(GameCurrencyType) +"&ip=" + FString::SanitizeFloat(RealMoneyCost / GameCurrencyAmount) + "&iq=" + FString::FromInt(GameCurrencyAmount) + "&iv=" + FPlatformHttp::UrlEncode(PaymentProvider) + "&ic=" + FPlatformHttp::UrlEncode(GameCurrencyType) + "&cu=" + FPlatformHttp::UrlEncode(RealCurrencyType), CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
	if (bHasSessionStarted)
	{
		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), TEXT("Currency Given")));
		Params.Add(FAnalyticsEventAttribute(TEXT("Value"), GameCurrencyAmount));
		RecordEvent(GameCurrencyType, Params);
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
	if (bHasSessionStarted)
	{
		if (Error.Len() > 0)
		{
			const TArray<FCustomDimension> CustomDimensions = BuildCustomDimensionsFromAttributes(EventAttrs);
			const TArray<FCustomMetric> CustomMetrics = BuildCustomMetricsFromAttributes(EventAttrs);

			MakeOnlineRequest("exception", "&exd=" + FPlatformHttp::UrlEncode(Error) + "&exf=1", CustomDimensions, CustomMetrics);
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordProgress(const FString& ProgressType, const TArray<FString>& ProgressHierarchy, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
	if (bHasSessionStarted)
	{
		FString Hierarchy;

		for (int32 Index = 0; Index < ProgressHierarchy.Num(); Index++)
		{
			Hierarchy += ProgressHierarchy[Index];
			if (Index + 1 < ProgressHierarchy.Num())
			{
				Hierarchy += TEXT(".");
			}
		}

		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), TEXT("Progression")));
		Params.Add(FAnalyticsEventAttribute(TEXT("Label"), Hierarchy));
		RecordEvent(ProgressType, Params);
	}
}

FString FAnalyticsProviderGoogleAnalytics::BuildCustomDimensions(const TArray<FCustomDimension> CustomDimensions)
{
	FString Result = FString();

	for (auto& CustomDimension : CustomDimensions)
	{
		Result += "&cd" + FString::FromInt(CustomDimension.Index) + "=" + FPlatformHttp::UrlEncode(CustomDimension.Value);
	}

	return Result;
}

FString FAnalyticsProviderGoogleAnalytics::BuildCustomMetrics(const TArray<FCustomMetric> CustomMetrics)
{
	FString Result = FString();

	for (auto& CustomMetric : CustomMetrics)
	{
		Result += "&cm" + FString::FromInt(CustomMetric.Index) + "=" + FString::SanitizeFloat(CustomMetric.Value);
	}

	return Result;
}

const TArray<FCustomDimension> FAnalyticsProviderGoogleAnalytics::BuildCustomDimensionsFromAttributes(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	TArray<FCustomDimension> CustomDimensions;

	for (int i = 0; i < Attributes.Num(); i++)
	{
		FString AttributeName = Attributes[i].AttrName;
		if (AttributeName.Contains("CustomDimension") && Attributes[i].ToString().Len() > 0)
		{
			AttributeName = AttributeName.Replace(*FString("CustomDimension"), *FString(""));

			if (AttributeName.IsNumeric())
			{
				FCustomDimension CustomDimension;
				CustomDimension.Index = FCString::Atoi(*AttributeName);
				CustomDimension.Value = Attributes[i].ToString();

				CustomDimensions.Add(CustomDimension);
			}
		}
	}

	return CustomDimensions;
}

const TArray<FCustomMetric> FAnalyticsProviderGoogleAnalytics::BuildCustomMetricsFromAttributes(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	TArray<FCustomMetric> CustomMetrics;

	for (int i = 0; i < Attributes.Num(); i++)
	{
		FString AttributeName = Attributes[i].AttrName;
		if (AttributeName.Contains("CustomMetric") && Attributes[i].ToString().Len() > 0)
		{
			AttributeName = AttributeName.Replace(*FString("CustomMetric"), *FString(""));

			if (AttributeName.IsNumeric() && Attributes[i].ToString().IsNumeric())
			{
				FCustomMetric CustomMetric;
				CustomMetric.Index = FCString::Atoi(*AttributeName);
				CustomMetric.Value = FCString::Atof(*Attributes[i].ToString());

				CustomMetrics.Add(CustomMetric);
			}
		}
	}

	return CustomMetrics;
}

void FAnalyticsProviderGoogleAnalytics::MakeOnlineRequest(const FString& Type, const FString& Parameters, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	FString SystemInfo = FString("");

	// Application Name, Application ID
	FString ApplicationName = FString("");
	FString ApplicationID = FString("");
	FString ApplicationVersion = FString("");

#if PLATFORM_IOS
	GConfig->GetString(TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"), TEXT("BundleName"), ApplicationName, GEngineIni);
	GConfig->GetString(TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"), TEXT("BundleIdentifier"), ApplicationID, GEngineIni);
	GConfig->GetString(TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"), TEXT("VersionInfo"), ApplicationVersion, GEngineIni);
#elif PLATFORM_ANDROID
	GConfig->GetString(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("ProjectName"), ApplicationName, GEngineIni);
	GConfig->GetString(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("PackageName"), ApplicationID, GEngineIni);
	GConfig->GetString(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("VersionDisplayName"), ApplicationVersion, GEngineIni);
#else
	GConfig->GetString(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("ProjectVersion"), ApplicationVersion, GGameIni);
#endif

	if (ApplicationName.IsEmpty())
	{
		ApplicationName = UKismetSystemLibrary::GetGameName();
	}

	if(ApplicationVersion.IsEmpty())
	{
		ApplicationVersion = FString("1.0");
	}

	SystemInfo += FString("&an=") + FPlatformHttp::UrlEncode(ApplicationName) + FString("&aid=") + FPlatformHttp::UrlEncode(ApplicationID) + FString("&av=") + FPlatformHttp::UrlEncode(ApplicationVersion);

	// Resolution
	if (GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		SystemInfo += FString("&ul=" + FInternationalization::Get().GetCurrentCulture()->GetName() + "&ua=" + UGameplayStatics::GetPlatformName() + "&sr=" + FString::FromInt(ViewportSize.X) + "x" + FString::FromInt(ViewportSize.Y) + "&vp=" + FString::FromInt(ViewportSize.X) + "x" + FString::FromInt(ViewportSize.Y));
	}

	// Start session
	if (!bSessionStartedSent)
	{
		bSessionStartedSent = true;
		SystemInfo += "&sc=start";
	}

	// Anonymize IP - GDPR
	if (bAnonymizeIp)
	{
		SystemInfo += "&aip=1";
	}

	// Request URL
	FString RequestUrl = "https://www.google-analytics.com/collect?v=1&t=" + Type + "&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + FPlatformHttp::UrlEncode(Location) + "&ds=UE4&uid=" + UserId + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + SystemInfo + Parameters;

	// Send Request
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->SetVerb("GET");
	HttpRequest->ProcessRequest();

	//UE_LOG(LogGoogleAnalytics, Display, TEXT("Google Analytics Provider Request: %s"), *RequestUrl);
}
