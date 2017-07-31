// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA Ltd. All Rights Reserved.

#include "GoogleAnalyticsBlueprintLibrary.h"


UGoogleAnalyticsBlueprintLibrary::UGoogleAnalyticsBlueprintLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/** Record Google Screen */
void UGoogleAnalyticsBlueprintLibrary::RecordGoogleScreen(const FString& ScreenName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		Provider->RecordScreen(ScreenName, CustomDimensions, CustomMetrics);
	}
}

/** Record Google Event */
void UGoogleAnalyticsBlueprintLibrary::RecordGoogleEvent(const FString& EventCategory, const FString& EventAction, const FString& EventLabel, const int32 EventValue, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> Params;
		Params.Add(FAnalyticsEventAttribute(TEXT("Category"), EventCategory));
		Params.Add(FAnalyticsEventAttribute(TEXT("Label"), EventLabel));
		Params.Add(FAnalyticsEventAttribute(TEXT("Value"), EventValue));

		for (int i = 0; i < CustomDimensions.Num(); i++)
		{
			Params.Add(FAnalyticsEventAttribute(TEXT("CustomDimension") + FString::FromInt(CustomDimensions[i].Index), CustomDimensions[i].Value));
		}
		for (int i = 0; i < CustomMetrics.Num(); i++)
		{
			Params.Add(FAnalyticsEventAttribute(TEXT("CustomMetric") + FString::FromInt(CustomMetrics[i].Index), FString::SanitizeFloat(CustomMetrics[i].Value)));
		}

		Provider->RecordEvent(EventAction, Params);
	}
}

/** Set new Tracking Id (only for Google Analytics) */
void UGoogleAnalyticsBlueprintLibrary::SetTrackingId(const FString& TrackingId)
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		return Provider->SetTrackingId(TrackingId);
	}
}

/** Get current Tracking Id (only for Google Analytics) */
FString UGoogleAnalyticsBlueprintLibrary::GetTrackingId()
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		return Provider->GetTrackingId();
	}
	return FString("");
}

/** Record Google Social Interaction */
void UGoogleAnalyticsBlueprintLibrary::RecordGoogleSocialInteraction(const FString& SocialNetwork, const FString& SocialAction, const FString& SocialTarget, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		Provider->RecordSocialInteraction(SocialNetwork, SocialAction, SocialTarget, CustomDimensions, CustomMetrics);
	}
}

/** Record Google User Timing */ 
void UGoogleAnalyticsBlueprintLibrary::RecordGoogleUserTiming(const FString& TimingCategory, const int32 TimingValue, const FString& TimingName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics)
{
	TSharedPtr<FAnalyticsProviderGoogleAnalytics> Provider = FAnalyticsProviderGoogleAnalytics::GetProvider();
	if (Provider.IsValid())
	{
		Provider->RecordUserTiming(TimingCategory, TimingValue, TimingName, CustomDimensions, CustomMetrics);
	}
}
