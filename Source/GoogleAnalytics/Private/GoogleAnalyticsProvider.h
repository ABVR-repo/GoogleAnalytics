// Google Analytics Provider
// Created by Patryk Stepniewski
// Copyright (c) 2014-2017 gameDNA. All Rights Reserved.

#pragma once

#if !PLATFORM_IOS && !PLATFORM_ANDROID
#include "Http.h" 
#include "Json.h"
#endif

class FAnalyticsProviderGoogleAnalytics :
	public IAnalyticsProvider
{
	FString ApiTrackingId;
	bool bHasSessionStarted;
	bool bSessionStartedSent;
	FString UserId;
	FString UniversalCid;
	FString Location;
	int32 Interval;
	FString OpenUrlIOS;
	FString OpenUrlHostIOS;

	static TSharedPtr<IAnalyticsProvider> Provider;
	FAnalyticsProviderGoogleAnalytics(const FString TrackingId, const int32 SendInterval);

public:
	static TSharedPtr<IAnalyticsProvider> Create(const FString TrackingId, const int32 SendInterval)
	{
		if (!Provider.IsValid())
		{
			Provider = TSharedPtr<IAnalyticsProvider>(new FAnalyticsProviderGoogleAnalytics(TrackingId, SendInterval));
		}
		return Provider;
	}

	static void Destroy()
	{
		Provider.Reset();
	}

	static TSharedPtr<FAnalyticsProviderGoogleAnalytics> GetProvider() {
		return StaticCastSharedPtr<FAnalyticsProviderGoogleAnalytics>(Provider);
	}

	virtual ~FAnalyticsProviderGoogleAnalytics();

	virtual bool StartSession(const TArray<FAnalyticsEventAttribute>& Attributes) override;
	virtual void EndSession() override;
	virtual void FlushEvents() override;

	virtual void SetUserID(const FString& InUserID) override;
	virtual FString GetUserID() const override;

	virtual FString GetSessionID() const override;
	virtual bool SetSessionID(const FString& InSessionID) override;

	virtual void SetGender(const FString& InGender) override;
	virtual void SetLocation(const FString& InLocation) override;
	virtual void SetAge(const int32 InAge) override;

	virtual void RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes) override;
	void RecordScreen(const FString& ScreenName);
	void RecordSocialInteraction(const FString& Network, const FString& Action, const FString& Target);
	void RecordUserTiming(const FString& Category, const int32 Value, const FString& Name);
	virtual void RecordItemPurchase(const FString& ItemId, int ItemQuantity, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
	virtual void RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
	virtual void RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
	virtual void RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
	virtual void RecordProgress(const FString& ProgressType, const TArray<FString>& ProgressHierarchy, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;

	void SetTrackingId(const FString& TrackingId);
	FString GetTrackingId();

	FString GetSystemInfo();
	
	void SetOpenUrlIOS(const FString& OpenUrl);
	FString GetOpenUrlIOS();

	void SetOpenUrlHostIOS(const FString& OpenUrlHost);
	FString GetOpenUrlHostIOS();
};
