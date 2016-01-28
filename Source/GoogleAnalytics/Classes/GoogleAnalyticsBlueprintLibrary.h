// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2016 gameDNA studio. All Rights Reserved.

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

	/** Set new Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static void SetTrackingId(const FString& TrackingId);

	/** Get current Tracking Id (only for Google Analytics) */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	static FString GetTrackingId();
};
