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

#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
#import "GAI.h"
#import "GAIFields.h"
#import "GAIDictionaryBuilder.h"
#include "IOSAppDelegate.h"
#endif
#elif PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include <android_native_app_glue.h>
#endif


DEFINE_LOG_CATEGORY(LogGoogleAnalytics);

#define LOCTEXT_NAMESPACE "GoogleAnalytics"

IMPLEMENT_MODULE(FAnalyticsGoogleAnalytics, GoogleAnalytics)

TSharedPtr<IAnalyticsProvider> FAnalyticsProviderGoogleAnalytics::Provider;

#if PLATFORM_ANDROID
jintArray BuildCustomDimensionsIndexArray(const TArray<FCustomDimension>& CustomDimensions)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = (jintArray)Env->NewIntArray(CustomDimensions.Num());
		jint* CustomDimensionIndexElements = Env->GetIntArrayElements(CustomDimensionIndex, 0);
		for (uint32 Param = 0; Param < CustomDimensions.Num(); Param++)
		{
			CustomDimensionIndexElements[Param] = CustomDimensions[Param].Index;
		}
		Env->ReleaseIntArrayElements(CustomDimensionIndex, CustomDimensionIndexElements, 0);
		return CustomDimensionIndex;
	}

	return NULL;
}

jobjectArray BuildCustomDimensionsValueArray(const TArray<FCustomDimension>& CustomDimensions)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jobjectArray CustomDimensionValue = (jobjectArray)Env->NewObjectArray(CustomDimensions.Num(), FJavaWrapper::JavaStringClass, NULL);
		for (uint32 Param = 0; Param < CustomDimensions.Num(); Param++)
		{
			jstring StringValue = Env->NewStringUTF(TCHAR_TO_UTF8(*CustomDimensions[Param].Value));
			Env->SetObjectArrayElement(CustomDimensionValue, Param, StringValue);
			Env->DeleteLocalRef(StringValue);
		}
		return CustomDimensionValue;
	}

	return NULL;
}

jintArray BuildCustomMetricsIndexArray(const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomMetricIndex = (jintArray)Env->NewIntArray(CustomMetrics.Num());
		jint* CustomMetricIndexElements = Env->GetIntArrayElements(CustomMetricIndex, 0);
		for (uint32 Param = 0; Param < CustomMetrics.Num(); Param++)
		{
			CustomMetricIndexElements[Param] = CustomMetrics[Param].Index;
		}
		Env->ReleaseIntArrayElements(CustomMetricIndex, CustomMetricIndexElements, 0);
		return CustomMetricIndex;
	}

	return NULL;
}

jfloatArray BuildCustomMetricsValueArray(const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jfloatArray CustomMetricValue = (jfloatArray)Env->NewFloatArray(CustomMetrics.Num());
		jfloat* CustomMetricValueElements = Env->GetFloatArrayElements(CustomMetricValue, 0);
		for (uint32 Param = 0; Param < CustomMetrics.Num(); Param++)
		{
			CustomMetricValueElements[Param] = CustomMetrics[Param].Value;
		}
		Env->ReleaseFloatArrayElements(CustomMetricValue, CustomMetricValueElements, 0);
		return CustomMetricValue;
	}

	return NULL;
}
#endif

#if PLATFORM_ANDROID
void AndroidThunkCpp_GoogleAnalyticsStartSession(FString& TrackingId, int32& DispatchInterval, bool AdvertisingIdCollection, bool AnonymizeIp)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring TrackingIdFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*TrackingId));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsStartSession", "(Ljava/lang/String;IZZ)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, TrackingIdFinal, DispatchInterval, AdvertisingIdCollection, AnonymizeIp);
		Env->DeleteLocalRef(TrackingIdFinal);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordScreen(const FString& ScreenName, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring ScreenNameFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*ScreenName));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordScreen", "(Ljava/lang/String;[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, ScreenNameFinal, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(ScreenNameFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordEvent(const FString& Category, const FString& Action, const FString& Label, const int32& Value, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring CategoryFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Category));
		jstring ActionFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Action));
		jstring LabelFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Label));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordEvent", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, CategoryFinal, ActionFinal, LabelFinal, Value, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(CategoryFinal);
		Env->DeleteLocalRef(ActionFinal);
		Env->DeleteLocalRef(LabelFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

void AndroidThunkCpp_GoogleAnalyticsFlushEvents()
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsFlushEvents", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
}

void AndroidThunkCpp_GoogleAnalyticsSetUserId(const FString& UserId) {
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring UserIdFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*UserId));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsSetUserId", "(Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, UserIdFinal);
		Env->DeleteLocalRef(UserIdFinal);
	}
}

FString AndroidThunkCpp_GoogleAnalyticsGetUserId() {
	FString ResultUserId = FString("");
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsGetUserId", "()Ljava/lang/String;", false);
		jstring UserIdString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method);
		const char *nativeUserIdString = Env->GetStringUTFChars(UserIdString, 0);
		ResultUserId = FString(nativeUserIdString);
		Env->ReleaseStringUTFChars(UserIdString, nativeUserIdString);
		Env->DeleteLocalRef(UserIdString);
	}
	return ResultUserId;
}

void AndroidThunkCpp_GoogleAnalyticsSetLocation(const FString& Location) {
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring LocationFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Location));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsSetLocation", "(Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, LocationFinal);
		Env->DeleteLocalRef(LocationFinal);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordError(const FString& Description, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics) {
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring DescriptionFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Description));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordError", "(Ljava/lang/String;[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, DescriptionFinal, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(DescriptionFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordCurrencyPurchase(const FString& TransactionId, const FString& GameCurrencyType, const int32& GameCurrencyAmount, const FString& RealCurrencyType, const float& RealMoneyCost, const FString& PaymentProvider, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics) {
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring TransactionIdFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*TransactionId));
		jstring GameCurrencyTypeFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*GameCurrencyType));
		jstring RealCurrencyTypeFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*RealCurrencyType));
		jstring PaymentProviderFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*PaymentProvider));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordCurrencyPurchase", "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;FLjava/lang/String;[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, TransactionIdFinal, GameCurrencyTypeFinal, GameCurrencyAmount, RealCurrencyTypeFinal, RealMoneyCost, PaymentProviderFinal, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(TransactionIdFinal);
		Env->DeleteLocalRef(GameCurrencyTypeFinal);
		Env->DeleteLocalRef(RealCurrencyTypeFinal);
		Env->DeleteLocalRef(PaymentProviderFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordSocialInteraction(const FString& Network, const FString& Action, const FString& Target, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring NetworkFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Network));
		jstring ActionFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Action));
		jstring TargetFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Target));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordSocialInteraction", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, NetworkFinal, ActionFinal, TargetFinal, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(NetworkFinal);
		Env->DeleteLocalRef(ActionFinal);
		Env->DeleteLocalRef(TargetFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

void AndroidThunkCpp_GoogleAnalyticsRecordUserTiming(const FString& Category, const int32& Value, const FString& Name, const TArray<FCustomDimension>& CustomDimensions, const TArray<FCustomMetric>& CustomMetrics)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jintArray CustomDimensionIndex = BuildCustomDimensionsIndexArray(CustomDimensions);
		jobjectArray CustomDimensionValue = BuildCustomDimensionsValueArray(CustomDimensions);
		jintArray CustomMetricIndex = BuildCustomMetricsIndexArray(CustomMetrics);
		jfloatArray CustomMetricValue = BuildCustomMetricsValueArray(CustomMetrics);

		jstring CategoryFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Category));
		jstring NameFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*Name));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GoogleAnalyticsRecordUserTiming", "(Ljava/lang/String;ILjava/lang/String;[I[Ljava/lang/String;[I[F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, CategoryFinal, Value, NameFinal, CustomDimensionIndex, CustomDimensionValue, CustomMetricIndex, CustomMetricValue);
		Env->DeleteLocalRef(CategoryFinal);
		Env->DeleteLocalRef(NameFinal);

		Env->DeleteLocalRef(CustomDimensionIndex);
		Env->DeleteLocalRef(CustomDimensionValue);
		Env->DeleteLocalRef(CustomMetricIndex);
		Env->DeleteLocalRef(CustomMetricValue);
	}
}

#endif

#if PLATFORM_IOS && WITH_GOOGLEANALYTICS
static void ListenGoogleAnalyticsOpenURL(UIApplication* application, NSURL* url, NSString* sourceApplication, id annotation)
{
	if(url != nil)
	{
		TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
		if (Provider.IsValid())
		{
			Provider->SetOpenUrlIOS(FString([url absoluteString]));
			Provider->SetOpenUrlHostIOS(FString([url host]));
		}
	}
}
#endif

void FAnalyticsGoogleAnalytics::StartupModule()
{
#if PLATFORM_IOS && WITH_GOOGLEANALYTICS
	FIOSCoreDelegates::OnOpenURL.AddStatic(&ListenGoogleAnalyticsOpenURL);
#endif

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
		FString TrackingId = FString("");
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		TrackingId = GetConfigValue.Execute(TEXT("TrackingIdIOS"), true);
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		TrackingId = GetConfigValue.Execute(TEXT("TrackingIdAndroid"), true);
#else
		TrackingId = GetConfigValue.Execute(TEXT("TrackingIdUniversal"), true);
#endif
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

#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		if (Interval > 0)
		{
			[[GAI sharedInstance] setDispatchInterval:Interval];
		}
		[[GAI sharedInstance] setTrackUncaughtExceptions:true];
		id<GAITracker> tracker = [[GAI sharedInstance] trackerWithName:@"DefaultTracker" trackingId : ApiTrackingId.GetNSString()];

		if (DefaultSettings->bEnableIDFACollection && tracker != nil)
		{
			tracker.allowIDFACollection = YES;
		}

		if(bAnonymizeIp && tracker != nil)
		{
			[tracker set : @"ga_anonymizeIp" value : @"true"];
		}

		if (OpenUrlIOS.Len() > 0 && tracker != nil)
		{
			GAIDictionaryBuilder *hitParams = [[GAIDictionaryBuilder alloc] init];
			[hitParams setCampaignParametersFromUrl : OpenUrlIOS.GetNSString()];
			if (![hitParams get : kGAICampaignSource] && OpenUrlHostIOS.GetNSString().length != 0)
			{
				[hitParams set : @"referrer" forKey : kGAICampaignMedium];
				[hitParams set : OpenUrlHostIOS.GetNSString() forKey : kGAICampaignSource];
			}
			NSDictionary *hitParamsDict = [hitParams build];
			[tracker set : kGAIScreenName value : @"Game Launched"];
			[tracker send : [[[GAIDictionaryBuilder createScreenView] setAll:hitParamsDict] build]];
		}
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		AndroidThunkCpp_GoogleAnalyticsStartSession(ApiTrackingId, Interval, DefaultSettings->bEnableIDFACollection, bAnonymizeIp);
#else
		FString UniversalCidBufor("");
		GConfig->GetString(TEXT("GoogleAnalytics"), TEXT("UniversalCid"), UniversalCidBufor, GEngineIni);

		if (UniversalCidBufor.Len() > 0)
		{
			UniversalCid = UniversalCidBufor;
		}
		else
		{
			UniversalCid = FMD5::HashAnsiString(*("UniversalCid" + FDateTime::Now().ToString()));
		}

		GConfig->SetString(TEXT("GoogleAnalytics"), TEXT("UniversalCid"), *UniversalCid, GEngineIni);

		RecordScreen("Game Launched");
#endif
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
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		[[GAI sharedInstance] removeTrackerByName:@"DefaultTracker"];
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#endif
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

void FAnalyticsProviderGoogleAnalytics::SetOpenUrlIOS(const FString& OpenUrl)
{
	OpenUrlIOS = OpenUrl;
}

FString FAnalyticsProviderGoogleAnalytics::GetOpenUrlIOS()
{
	return OpenUrlIOS;
}

void FAnalyticsProviderGoogleAnalytics::SetOpenUrlHostIOS(const FString& OpenUrlHost)
{
	OpenUrlHostIOS = OpenUrlHost;
}

FString FAnalyticsProviderGoogleAnalytics::GetOpenUrlHostIOS()
{
	return OpenUrlHostIOS;
}

FString FAnalyticsProviderGoogleAnalytics::GetSystemInfo()
{
	FString SystemInfo = FString("");

	if (GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		SystemInfo += FString("&ul=" + FInternationalization::Get().GetCurrentCulture()->GetName() + "&ua=Windows&sr=" + FString::FromInt(ViewportSize.X) + "x" + FString::FromInt(ViewportSize.Y) + "&vp=" + FString::FromInt(ViewportSize.X) + "x" + FString::FromInt(ViewportSize.Y));
	}

	if (!bSessionStartedSent)
	{
		bSessionStartedSent = true;
		SystemInfo += "&sc=start";
	}

	if(bAnonymizeIp)
	{
		SystemInfo += "&aip=1";
	}

	return SystemInfo;
}

void FAnalyticsProviderGoogleAnalytics::FlushEvents()
{
	if (bHasSessionStarted)
	{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		[[GAI sharedInstance] dispatch];
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		AndroidThunkCpp_GoogleAnalyticsFlushEvents();
#endif
	}
}

void FAnalyticsProviderGoogleAnalytics::SetUserID(const FString& InUserId)
{
	if (bHasSessionStarted)
	{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

		if (tracker != nil)
		{
			[tracker set : kGAIUserId value : InUserId.GetNSString()];
		}
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		AndroidThunkCpp_GoogleAnalyticsSetUserId(InUserId);
#else
		UserId = InUserId;
#endif
	}
}

FString FAnalyticsProviderGoogleAnalytics::GetUserID() const
{
	if (bHasSessionStarted)
	{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

		if (tracker)
		{
			return FString([tracker get : kGAIUserId]);
		}
		else
		{
			return FString();
		}
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		return AndroidThunkCpp_GoogleAnalyticsGetUserId();
#else
		return UserId;
#endif
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
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
		id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

		if (tracker != nil)
		{
			[tracker set : kGAILocation value : InLocation.GetNSString()];
		}
#else
		UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
		AndroidThunkCpp_GoogleAnalyticsSetLocation(InLocation);
#else 
		Location = InLocation;
#endif
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

#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			NSString* EventLabel = Label.Len() > 0 ? Label.GetNSString() : nil;
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);

				[tracker send : [[GAIDictionaryBuilder createEventWithCategory : Category.GetNSString()
					action : EventName.GetNSString()
					label : EventLabel
					value : @(Value)] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordEvent(Category, EventName, Label, Value, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetURL("https://www.google-analytics.com/collect?v=1&t=event&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&ec=" + FPlatformHttp::UrlEncode(Category) + "&ea=" + FPlatformHttp::UrlEncode(Action) + "&el=" + FPlatformHttp::UrlEncode(Label) + "&ev=" + FString::FromInt(Value) + "&geoid=" + Location + "&uid=" + UserId + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequest->SetVerb("GET");
			HttpRequest->ProcessRequest();
#endif
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordScreen(const FString& ScreenName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (ScreenName.Len() > 0)
		{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);
				[tracker set : kGAIScreenName value : ScreenName.GetNSString()];
				[tracker send : [[GAIDictionaryBuilder createScreenView] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordScreen(ScreenName, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetURL("https://www.google-analytics.com/collect?v=1&t=pageview&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&dp=" + FPlatformHttp::UrlEncode(ScreenName) + "&dt=" + FPlatformHttp::UrlEncode(ScreenName) + "&geoid=" + Location + "&uid=" + UserId + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequest->SetVerb("GET");
			HttpRequest->ProcessRequest();
#endif
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordSocialInteraction(const FString& Network, const FString& Action, const FString& Target, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (Network.Len() > 0 && Action.Len() > 0)
		{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);

				NSString* EventTarget = Target.Len() > 0 ? Target.GetNSString() : nil;
				[tracker send : [[GAIDictionaryBuilder createSocialWithNetwork : Network.GetNSString() action : Action.GetNSString() target : EventTarget] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordSocialInteraction(Network, Action, Target, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetURL("https://www.google-analytics.com/collect?v=1&t=social&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + Location + "&uid=" + UserId + "&sn=" + FPlatformHttp::UrlEncode(Network) + "&sa=" + FPlatformHttp::UrlEncode(Action) + "&st=" + FPlatformHttp::UrlEncode(Target) + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequest->SetVerb("GET");
			HttpRequest->ProcessRequest();
#endif
		}
	}
}

void FAnalyticsProviderGoogleAnalytics::RecordUserTiming(const FString& Category, const int32 Value, const FString& Name, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	if (bHasSessionStarted)
	{
		if (Category.Len() > 0)
		{
#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);

				[tracker send : [[GAIDictionaryBuilder createTimingWithCategory : Category.GetNSString() interval : @((NSUInteger)(Value)) name:Name.GetNSString() label:nil] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordUserTiming(Category, Value, Name, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetURL("https://www.google-analytics.com/collect?v=1&t=timing&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + Location + "&uid=" + UserId + "&utc=" + FPlatformHttp::UrlEncode(Category) + "&utv=" + FPlatformHttp::UrlEncode(Name) + "&utt=" + FString::FromInt(Value) + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequest->SetVerb("GET");
			HttpRequest->ProcessRequest();
#endif
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

#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);

				[tracker send : [[GAIDictionaryBuilder createTransactionWithId : TransactionId.GetNSString()
					affiliation : PaymentProvider.GetNSString()
					revenue : @(RealMoneyCost)
							  tax : @0.0F
					shipping : @0
					currencyCode : RealCurrencyType.GetNSString()] build]];

				[tracker send : [[GAIDictionaryBuilder createItemWithTransactionId : TransactionId.GetNSString()
					name : GameCurrencyType.GetNSString()
					sku : GameCurrencyType.GetNSString()
					category : PaymentProvider.GetNSString()
					price : @((RealMoneyCost / GameCurrencyAmount))
							quantity:@(GameCurrencyAmount)
									 currencyCode : RealCurrencyType.GetNSString()] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordCurrencyPurchase(TransactionId, GameCurrencyType, GameCurrencyAmount, RealCurrencyType, RealMoneyCost, PaymentProvider, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequestTransaction = FHttpModule::Get().CreateRequest();
			HttpRequestTransaction->SetURL("https://www.google-analytics.com/collect?v=1&t=transaction&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + Location + "&uid=" + UserId + "&ti=" + TransactionId + "&ta=" + PaymentProvider + "&tr=" + FString::SanitizeFloat(RealMoneyCost) + "&ts=0&tt=0&cu=" + RealCurrencyType + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequestTransaction->SetVerb("GET");
			HttpRequestTransaction->ProcessRequest();

			TSharedRef<IHttpRequest> HttpRequestItem = FHttpModule::Get().CreateRequest();
			HttpRequestItem->SetURL("https://www.google-analytics.com/collect?v=1&t=item&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + Location + "&uid=" + UserId + "&ti=" + TransactionId + "&in=" + GameCurrencyType + "&ip=" + FString::SanitizeFloat(RealMoneyCost / GameCurrencyAmount) + "&iq=" + FString::FromInt(GameCurrencyAmount) + "&iv=" + PaymentProvider + "&ic=" + GameCurrencyType + "&cu=" + RealCurrencyType + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequestItem->SetVerb("GET");
			HttpRequestItem->ProcessRequest();
#endif
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

#if PLATFORM_IOS
#if WITH_GOOGLEANALYTICS
			id<GAITracker> tracker = [[GAI sharedInstance] defaultTracker];

			if (tracker != nil)
			{
				tracker = BuildCustomDimensionsAndMetrics(tracker, CustomDimensions, CustomMetrics);

				[tracker send : [[GAIDictionaryBuilder
					createExceptionWithDescription : Error.GetNSString()
					withFatal : @NO] build]];
			}
#else
			UE_LOG(LogGoogleAnalytics, Warning, TEXT("WITH_GOOGLEANALYTICS=0. Are you missing the SDK?"));
#endif
#elif PLATFORM_ANDROID
			AndroidThunkCpp_GoogleAnalyticsRecordError(Error, CustomDimensions, CustomMetrics);
#else
			TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetURL("https://www.google-analytics.com/collect?v=1&t=exception&tid=" + ApiTrackingId + "&cid=" + UniversalCid + "&geoid=" + Location + "&uid=" + UserId + "&exd=" + Error + "&exf=0" + BuildCustomDimensions(CustomDimensions) + BuildCustomMetrics(CustomMetrics) + GetSystemInfo());
			HttpRequest->SetVerb("GET");
			HttpRequest->ProcessRequest();
#endif
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

#if PLATFORM_IOS && WITH_GOOGLEANALYTICS
id<GAITracker> FAnalyticsProviderGoogleAnalytics::BuildCustomDimensionsAndMetrics(id<GAITracker> tracker, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	for (int i = 0; i < CustomDimensions.Num(); i++)
	{
		[tracker set : [GAIFields customDimensionForIndex : CustomDimensions[i].Index] value : CustomDimensions[i].Value.GetNSString() ];
	}

	for (int i = 0; i < CustomMetrics.Num(); i++)
	{
		[tracker set : [GAIFields customMetricForIndex : CustomMetrics[i].Index] value : FString::SanitizeFloat(CustomMetrics[i].Value).GetNSString() ];
	}

	return tracker;
}
#endif

FString FAnalyticsProviderGoogleAnalytics::BuildCustomDimensions(const TArray<FCustomDimension> CustomDimensions)
{
	FString Result = FString();

	for (auto& CustomDimension : CustomDimensions)
	{
		Result += "&cd" + FString::FromInt(CustomDimension.Index) + "=" + CustomDimension.Value;
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
