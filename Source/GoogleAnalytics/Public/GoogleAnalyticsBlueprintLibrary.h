// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoogleAnalyticsDelegates.h"
#include "GoogleAnalyticsBlueprintLibrary.generated.h"

UCLASS()
class GOOGLEANALYTICS_API UGoogleAnalyticsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Records a screen (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics", meta = (AutoCreateRefTerm = "CustomDimensions, CustomMetrics"))
	static void RecordGoogleScreen(const FString& ScreenName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics);

	/** Records a screen (only for Google Analytics) */
	static void RecordGoogleScreen(const FString& ScreenName);

	/** Records an event with all attributes (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics", meta = (AutoCreateRefTerm = "CustomDimensions, CustomMetrics"))
	static void RecordGoogleEvent(const FString& EventCategory, const FString& EventAction, const FString& EventLabel, const int32 EventValue, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics);

	/** Records an event with all attributes (only for Google Analytics) */
	static void RecordGoogleEvent(const FString& EventCategory, const FString& EventAction, const FString& EventLabel, const int32 EventValue);

	/** Records a social interaction (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics", meta = (AutoCreateRefTerm = "CustomDimensions, CustomMetrics"))
	static void RecordGoogleSocialInteraction(const FString& SocialNetwork, const FString& SocialAction, const FString& SocialTarget, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics);

	/** Records a social interaction (only for Google Analytics) */
	static void RecordGoogleSocialInteraction(const FString& SocialNetwork, const FString& SocialAction, const FString& SocialTarget);

	/** Records an user timing (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics", meta = (AutoCreateRefTerm = "CustomDimensions, CustomMetrics"))
	static void RecordGoogleUserTiming(const FString& TimingCategory, const int32 TimingValue, const FString& TimingName, const TArray<FCustomDimension> CustomDimensions, const TArray<FCustomMetric> CustomMetrics);

	/** Records an user timing (only for Google Analytics) */
	static void RecordGoogleUserTiming(const FString& TimingCategory, const int32 TimingValue, const FString& TimingName);

	/** Set new Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void SetTrackingId(const FString& TrackingId);

	/** Get current Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static FString GetTrackingId();

	/** If true, the IP address of the sender will be anonymized - GDPR compliant (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void SetAnonymizeIP(const bool Anonymize);
};
