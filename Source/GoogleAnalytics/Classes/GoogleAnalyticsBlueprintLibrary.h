// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoogleAnalyticsBlueprintLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGoogleAnalytics, Log, All);

UCLASS()
class GOOGLEANALYTICS_API UGoogleAnalyticsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Records a screen (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void RecordGoogleScreen(const FString& ScreenName);

	/** Records an event with all attributes (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void RecordGoogleEvent(const FString& EventCategory, const FString& EventAction, const FString& EventLabel, const int32 EventValue);

	/** Records a social interaction (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void RecordGoogleSocialInteraction(const FString& SocialNetwork, const FString& SocialAction, const FString& SocialTarget);

	/** Records an user timing (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void RecordGoogleUserTiming(const FString& TimingCategory, const int32 TimingValue, const FString& TimingName);

	/** Set new Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void SetTrackingId(const FString& TrackingId);

	/** Get current Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static FString GetTrackingId();
};
